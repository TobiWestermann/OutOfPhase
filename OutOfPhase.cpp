#include <math.h>
#include "OutOfPhase.h"

#include "PluginProcessor.h"

#include "resources/images/paint_bin.h"
#include "resources/images/paper_bin.h"

class ImageLoadThread : public juce::Thread
{
public:
    ImageLoadThread(OutOfPhaseGUI* gui) 
        : juce::Thread("ImageLoadThread"), owner(gui) {}

    void run() override;

private:
    OutOfPhaseGUI* owner;
};

void ImageLoadThread::run()
{
    auto paintImage = juce::ImageFileFormat::loadFrom(paint_bin, paint_bin_len);
    auto paperImage = juce::ImageFileFormat::loadFrom(paper_bin, paper_bin_len);

    if (threadShouldExit())
        return;

    juce::MessageManager::callAsync([this, paintImg = std::move(paintImage), paperImg = std::move(paperImage)]()
    {
        if (owner != nullptr && !threadShouldExit()) {
            owner->setBackgroundImages(paintImg, paperImg);
        }
    });
}

OutOfPhaseAudio::OutOfPhaseAudio(OutOfPhaseAudioProcessor* processor)
:WOLA(), m_processor(processor)
{
}

void OutOfPhaseAudio::prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels)
{
    juce::ScopedLock lock(dataMutex);
    juce::ignoreUnused(max_samplesPerBlock);
    juce::ignoreUnused(sampleRate);

    float desired_blocksize = *m_processor->m_parameterVTS->getRawParameterValue(g_paramBlocksize.ID);
    int synchronblocksize = static_cast<int>(round(desired_blocksize));
    
    if (g_forcePowerOf2)
        synchronblocksize = juce::nextPowerOfTwo(synchronblocksize);

    // only resize if necessary 
    if (synchronblocksize != m_synchronblocksize) {
        m_synchronblocksize = synchronblocksize;
        
        prepareWOLAprocessing(max_channels, synchronblocksize, WOLA::WOLAType::SqrtHann_over50);
        m_fftprocess.setFFTSize(synchronblocksize);
        
        
        m_PrePhaseData = std::vector<float>(synchronblocksize/2+1, 0.0f);
        m_PostPhaseData = std::vector<float>(synchronblocksize/2+1, 0.0f);
        initFrostPhaseData();
        
        m_realdata.setSize(max_channels, synchronblocksize/2+1);
        m_imagdata.setSize(max_channels, synchronblocksize/2+1);
        m_realdata.clear();
        m_imagdata.clear();
    }
    
    m_Latency = synchronblocksize;
}

void OutOfPhaseAudio::addParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &paramVector)
{
    paramVector.push_back(std::make_unique<juce::AudioParameterChoice>(g_paramMode.ID,
        g_paramMode.name,
        juce::StringArray {g_paramMode.mode1, g_paramMode.mode2, g_paramMode.mode3 , g_paramMode.mode4}, g_paramMode.defaultValue
    ));
    
    paramVector.push_back(std::make_unique<juce::AudioParameterInt>(g_paramBlocksize.ID,
        g_paramBlocksize.name, g_paramBlocksize.minValue, g_paramBlocksize.maxValue, g_paramBlocksize.defaultValue
    ));

    paramVector.push_back(std::make_unique<juce::AudioParameterFloat>(g_paramDryWet.ID,
        g_paramDryWet.name, g_paramDryWet.minValue, g_paramDryWet.maxValue, g_paramDryWet.defaultValue
    ));

    paramVector.push_back(std::make_unique<juce::AudioParameterChoice>(g_paramDistributionMode.ID,
        g_paramDistributionMode.name,
        juce::StringArray {g_paramDistributionMode.mode1, g_paramDistributionMode.mode2}, g_paramDistributionMode.defaultValue
    ));

    paramVector.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::String(g_paramBandMode.ID),
        juce::String(g_paramBandMode.name), 
        g_paramBandMode.defaultValue
    ));

    paramVector.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::String(g_paramLowFreq.ID),
        juce::String(g_paramLowFreq.name), 
        g_paramLowFreq.minValue, 
        g_paramLowFreq.maxValue, 
        g_paramLowFreq.defaultValue
    ));

    paramVector.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::String(g_paramHighFreq.ID),
        juce::String(g_paramHighFreq.name), 
        g_paramHighFreq.minValue, 
        g_paramHighFreq.maxValue, 
        g_paramHighFreq.defaultValue
    ));

}

void OutOfPhaseAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    juce::ignoreUnused(vts);
}

int OutOfPhaseAudio::processWOLA(juce::AudioBuffer<float> &data, juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);

    if (data.getNumSamples() == 0 || m_synchronblocksize == 0)
        return 0;

    if (m_tempPrePhaseData.size() != m_synchronblocksize/2+1) {
        m_tempPrePhaseData.resize(m_synchronblocksize/2+1);
        m_tempPostPhaseData.resize(m_synchronblocksize/2+1);
    }

    float operatingMode = *m_processor->m_parameterVTS->getRawParameterValue(g_paramMode.ID);
    float dryWetMix = *m_processor->m_parameterVTS->getRawParameterValue(g_paramDryWet.ID);
    dryWetMix = juce::jlimit(0.0f, 1.0f, dryWetMix);

    bool bandModeActive = *m_processor->m_parameterVTS->getRawParameterValue(g_paramBandMode.ID) > 0.5f;
    float lowFreq = *m_processor->m_parameterVTS->getRawParameterValue(g_paramLowFreq.ID);
    float highFreq = *m_processor->m_parameterVTS->getRawParameterValue(g_paramHighFreq.ID);
    
    int lowBin = 0;
    int highBin = m_synchronblocksize / 2;

    if (bandModeActive) {
        double sampleRate = m_processor->getSampleRate();
        lowBin = static_cast<int>(std::floor(lowFreq * m_synchronblocksize / sampleRate));
        highBin = static_cast<int>(std::ceil(highFreq * m_synchronblocksize / sampleRate));
        
        lowBin = juce::jlimit(0, m_synchronblocksize / 2, lowBin);
        highBin = juce::jlimit(0, m_synchronblocksize / 2, highBin);
    }

    int numchns = data.getNumChannels();
    int numSamples = data.getNumSamples();

    if (m_realdata.getNumChannels() < numchns || m_imagdata.getNumChannels() < numchns) {
        return 0;
    }

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.setSize(numchns, numSamples, false, true, true);

    for (int cc = 0; cc < numchns; cc++)
        dryBuffer.copyFrom(cc, 0, data, cc, 0, numSamples);

    for (int cc = 0 ; cc < numchns; cc++)
    {

    // FFT 
        auto dataPtr = data.getWritePointer(cc);
        auto realPtr = m_realdata.getWritePointer(cc);
        auto imagPtr = m_imagdata.getWritePointer(cc);
        m_fftprocess.fft(dataPtr,realPtr,imagPtr);

        //m_FrostPhaseData.resize(m_synchronblocksize/2+1);
        //DBG("FrostPhaseData size: " << m_FrostPhaseData.size());

        for (int nn = 0; nn< m_synchronblocksize/2+1; nn++)
        {
            float absval = sqrtf(realPtr[nn]*realPtr[nn] + imagPtr[nn]*imagPtr[nn]);
            float PrePhase = atan2f(imagPtr[nn],realPtr[nn]);
            float PostPhase = atan2f(imagPtr[nn],realPtr[nn]);

            if (!bandModeActive || (nn >= lowBin && nn <= highBin)) {
                if (operatingMode == 0) // zero
                {
                    PostPhase = 0;
                }
                else if (operatingMode == 1) // frost
                {
                    if (nn < m_FrostPhaseData.size())
                    {
                        PostPhase = m_FrostPhaseData[nn];
                    }
                    else
                    {
                        PostPhase = 0.0f; // Default value if out of bounds
                    }
                }
                else if (operatingMode == 2) // random
                {
                    float DistributionModeID = *m_processor->m_parameterVTS->getRawParameterValue(g_paramDistributionMode.ID);
                    if (DistributionModeID == 0) // Uniform
                    {
                        PostPhase = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * juce::MathConstants<float>::pi;
                    }
                    else if (DistributionModeID == 1) // Gaussian
                    {
                        static float nextGaussian = 0.0f;
                        static bool hasNextGaussian = false;
                        
                        if (hasNextGaussian)
                        {
                            PostPhase = nextGaussian * juce::MathConstants<float>::pi; // Scale to full -π to π range
                            hasNextGaussian = false;
                        }
                        else
                        {
                            float u1, u2, s;
                            do
                            {
                                u1 = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                                u2 = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                                s = u1 * u1 + u2 * u2;
                            } while (s >= 1.0f || s == 0.0f);
                            
                            s = sqrtf(-2.0f * logf(s) / s);
                            nextGaussian = u2 * s;
                            float concentration = 0.5f; // adjust to control concentration of the Gaussian distribution
                            PostPhase = u1 * s * juce::MathConstants<float>::pi * concentration; // Scale to full -π to π range
                            hasNextGaussian = true;
                        }

                        PostPhase = juce::jlimit(-juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, PostPhase);
                    }
                }
                else if (operatingMode == 3) // flip
                {
                    //PostPhase = fmod(PostPhase + juce::MathConstants<float>::pi, juce::MathConstants<float>::pi);
                    PostPhase = -PostPhase;
                }
            }
            // rück
            realPtr[nn] = absval*cosf(PostPhase);
            imagPtr[nn] = absval*sinf(PostPhase);

            m_tempPrePhaseData[nn] = PrePhase;
            m_tempPostPhaseData[nn] = PostPhase;
        }

        // IFFT
        m_fftprocess.ifft(realPtr,imagPtr, dataPtr);

        // Apply dry/wet mix
        auto dryPtr = dryBuffer.getReadPointer(cc);
        
        float wetRatio = dryWetMix;
        wetRatio = juce::jlimit(0.0f, 1.0f, wetRatio);
        float dryRatio = 1.0f - wetRatio;
        
        for (int i = 0; i < numSamples; ++i)
            dataPtr[i] = dryPtr[i] * dryRatio + dataPtr[i] * wetRatio;
    }

    {
        juce::ScopedLock lock(dataMutex);
        std::swap(m_PrePhaseData, m_tempPrePhaseData);
        std::swap(m_PostPhaseData, m_tempPostPhaseData);
    }


    return 0;
}

OutOfPhaseGUI::~OutOfPhaseGUI()
{
    if (m_imageLoadThread != nullptr)
    {
        m_imageLoadThread->stopThread(1000);
        m_imageLoadThread = nullptr;
    }
}

OutOfPhaseGUI::OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
:m_processor(p) ,m_apvts(apvts)
{
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this);
    tooltipWindow->setMillisecondsBeforeTipAppears(500);
    
    startTimerHz(60);

    m_BlocksizeSlider.setDoubleClickReturnValue(true, 512);
    addAndMakeVisible(m_BlocksizeSlider);
    BlocksizeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramBlocksize.ID, m_BlocksizeSlider);
    m_BlocksizeSlider.onValueChange = [this]
    {
        juce::ScopedLock lock(m_processor.getCallbackLock());
        m_processor.suspendProcessing(true);

        auto sampleRate = m_processor.getSampleRate();
        auto blockSize = m_BlocksizeSlider.getValue();
        auto channels = m_processor.getTotalNumInputChannels();

        m_processor.m_algo.prepareToPlay(sampleRate, static_cast<int>(blockSize), channels);

        m_processor.suspendProcessing(false);
    };

    m_DryWetSlider.setNumDecimalPlacesToDisplay(2);
    m_DryWetSlider.setDoubleClickReturnValue(true, 0.5f);
    DryWetSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramDryWet.ID, m_DryWetSlider);
    addAndMakeVisible(m_DryWetSlider);

    addAndMakeVisible(m_PrePhasePlot);
    addAndMakeVisible(m_PostPhasePlot);

    m_ZeroModeTextButton.setButtonText("Zero");
    addAndMakeVisible(m_ZeroModeTextButton);
    m_ZeroModeTextButton.onClick = [this]
    {
        m_processor.m_parameterVTS->getParameterAsValue(g_paramMode.ID) = 0;
    };
    m_ZeroModeTextButton.setRadioGroupId(1);
    m_ZeroModeTextButton.setClickingTogglesState(true);
    
    m_FrostModeTextButton.setButtonText("Frost");
    addAndMakeVisible(m_FrostModeTextButton);
    m_FrostModeTextButton.onClick = [this]
    {
        m_processor.m_parameterVTS->getParameterAsValue(g_paramMode.ID) = 1;
    };
    m_FrostModeTextButton.setRadioGroupId(1);
    m_FrostModeTextButton.setClickingTogglesState(true);
    
    m_RandomModeTextButton.setButtonText("Random");
    addAndMakeVisible(m_RandomModeTextButton);
    m_RandomModeTextButton.onClick = [this]
    {
        m_processor.m_parameterVTS->getParameterAsValue(g_paramMode.ID) = 2;
    };
    m_RandomModeTextButton.setRadioGroupId(1);
    m_RandomModeTextButton.setClickingTogglesState(true);
    
    m_FlipModeTextButton.setButtonText("Flip");
    addAndMakeVisible(m_FlipModeTextButton);
    m_FlipModeTextButton.onClick = [this]
    {
        m_processor.m_parameterVTS->getParameterAsValue(g_paramMode.ID) = 3;
    };
    m_FlipModeTextButton.setRadioGroupId(1);
    m_FlipModeTextButton.setClickingTogglesState(true);

    m_BandModeButton.setButtonText("Band Mode");
    addAndMakeVisible(m_BandModeButton);
    m_BandModeButton.setClickingTogglesState(true);
    m_BandModeButton.setTooltip("Apply effect only to a specific frequency band when enabled");
    BandModeButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        *m_processor.m_parameterVTS, g_paramBandMode.ID, m_BandModeButton);
    m_BandModeButton.onClick = [this] {
        updateModeButtonStates();
    };

    // low freq slider
    m_LowFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    m_LowFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    m_LowFreqSlider.setSkewFactorFromMidPoint(500.0f);
    m_LowFreqSlider.setTooltip("Lower frequency bound of the band");
    addAndMakeVisible(m_LowFreqSlider);
    LowFreqSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramLowFreq.ID, m_LowFreqSlider);
    m_LowFreqSlider.onValueChange = [this] {
        if (m_HighFreqSlider.getValue() < m_LowFreqSlider.getValue())
            m_HighFreqSlider.setValue(m_LowFreqSlider.getValue() * 1.2f);
    };
    m_LowFreqSlider.setVisible(false);

    // high freq slider
    m_HighFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    m_HighFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    m_HighFreqSlider.setSkewFactorFromMidPoint(2000.0f);
    m_HighFreqSlider.setTooltip("Upper frequency bound of the band");
    addAndMakeVisible(m_HighFreqSlider);
    HighFreqSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramHighFreq.ID, m_HighFreqSlider);
    m_HighFreqSlider.onValueChange = [this] {
        if (m_HighFreqSlider.getValue() < m_LowFreqSlider.getValue())
            m_LowFreqSlider.setValue(m_HighFreqSlider.getValue() * 0.8f);
    };
    m_HighFreqSlider.setVisible(false);

    addAndMakeVisible(m_DistributionSwitch);
    m_DistributionSwitch.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    m_DistributionSwitch.onStateChange = [this]() {
        m_processor.m_parameterVTS->getParameterAsValue(g_paramDistributionMode.ID) = 
            m_DistributionSwitch.getToggleState() ? 1 : 0;
    };

    m_FreezeCaptureButton.onClick = [this]()
    {
        m_processor.m_algo.updateFrostPhaseData();
        m_FreezeCaptureButton.triggerFlash();
    };
    addAndMakeVisible(m_FreezeCaptureButton);
    m_FreezeCaptureButton.setVisible(false);

    float initialMode = m_processor.m_parameterVTS->getRawParameterValue(g_paramMode.ID)->load();
    if (initialMode == 2.0f) {
        m_DistributionSwitch.setVisible(true);
        bool isGaussian = m_processor.m_parameterVTS->getRawParameterValue(g_paramDistributionMode.ID)->load() == 1;
        m_DistributionSwitch.setToggleState(isGaussian, juce::dontSendNotification);
    } else {
        m_DistributionSwitch.setVisible(false);
    }

    m_imageLoadThread = std::make_unique<ImageLoadThread>(this);
    m_imageLoadThread->startThread();

    updateModeButtonStates();
    resized();
}

void OutOfPhaseGUI::setBackgroundImages(const juce::Image& paintImg, const juce::Image& paperImg)
{
    m_paintImage = paintImg;
    m_paperImage = paperImg;
    m_imagesLoaded = true;
    repaint();
}

void OutOfPhaseGUI::paint(juce::Graphics &g)
{
    g.fillAll (juce::Colours::bisque);

    if (m_imagesLoaded) {
        g.drawImageWithin(m_paperImage, 0, 0, getWidth(), getHeight(),
                          juce::RectanglePlacement::fillDestination);

        g.drawImageWithin(m_paintImage, static_cast<int>(-getWidth() / 4), 
                          static_cast<int>(getHeight() * 0.84), getWidth(), 
                          static_cast<int>(getHeight() / 4),
                          juce::RectanglePlacement::fillDestination);

        g.drawImageWithin(m_paintImage, static_cast<int>(getWidth() - getWidth() * 0.7), 
                          static_cast<int>(-getHeight() / 3), getWidth(), getHeight(),
                          juce::RectanglePlacement::fillDestination);
    }

    g.setColour (juce::Colours::darkslategrey);
    g.setFont (12.0f * m_processor.getScaleFactor());

    int labelWidth = static_cast<int>(m_BlocksizeSlider.getWidth() * 1.5f);   

    g.drawText("Block Size", 
        m_BlocksizeSlider.getX() - (labelWidth - m_BlocksizeSlider.getWidth()) / 2,
        static_cast<int>(m_BlocksizeSlider.getY() - 15 * m_processor.getScaleFactor()),
        labelWidth, 
        static_cast<int>(20 * m_processor.getScaleFactor()),
        juce::Justification::centred);

    g.drawText("Dry/Wet", 
        m_DryWetSlider.getX() - (labelWidth - m_DryWetSlider.getWidth()) / 2,
        static_cast<int>(m_DryWetSlider.getY() - 15 * m_processor.getScaleFactor()),
        labelWidth, 
        static_cast<int>(20 * m_processor.getScaleFactor()),
        juce::Justification::centred);

    g.setFont(14.0f * m_processor.getScaleFactor());
    g.setColour(juce::Colours::darkslategrey);
    
    float leftEdge = std::min(static_cast<float>(m_RandomModeTextButton.getX()), static_cast<float>(m_FrostModeTextButton.getX()));
    float rightEdge = static_cast<float>(std::max(m_ZeroModeTextButton.getRight(), m_FlipModeTextButton.getRight()));
    

    juce::Rectangle<int> buttonGroupBounds(
        static_cast<int>(leftEdge), 
        m_ZeroModeTextButton.getY(),
        static_cast<int>(rightEdge - leftEdge),
        m_FlipModeTextButton.getBottom() - m_ZeroModeTextButton.getY()
    );
    
    g.drawText("Phase Mode", 
        buttonGroupBounds.getX(),
        static_cast<int>(buttonGroupBounds.getY() - 25 * m_processor.getScaleFactor()),
        buttonGroupBounds.getWidth(), 
        static_cast<int>(20 * m_processor.getScaleFactor()),
        juce::Justification::centred);

    g.setFont(10.0f * m_processor.getScaleFactor());

    g.drawText("Zero-Phase", 
        m_ZeroModeTextButton.getX(),
        static_cast<int>(m_ZeroModeTextButton.getBottom() + 5 * m_processor.getScaleFactor()),
        m_ZeroModeTextButton.getWidth(), 
        static_cast<int>(12 * m_processor.getScaleFactor()),
        juce::Justification::centred);
    
    g.drawText("Phase-Frost", 
        m_FrostModeTextButton.getX(),
        m_FrostModeTextButton.getBottom() + static_cast<int>(5 * m_processor.getScaleFactor()),
        m_FrostModeTextButton.getWidth(), 
        static_cast<int>(12 * m_processor.getScaleFactor()),
        juce::Justification::centred);
    
    g.drawText("Random-Phase", 
        m_RandomModeTextButton.getX(),
        static_cast<int>(m_RandomModeTextButton.getBottom() + 5 * m_processor.getScaleFactor()),
        m_RandomModeTextButton.getWidth(), 
        static_cast<int>(12 * m_processor.getScaleFactor()),
        juce::Justification::centred);
    
    g.drawText("Phase-Flip", 
        m_FlipModeTextButton.getX(),
        m_FlipModeTextButton.getBottom() + static_cast<int>(5 * m_processor.getScaleFactor()),
        m_FlipModeTextButton.getWidth(), 
        static_cast<int>(12 * m_processor.getScaleFactor()),
        juce::Justification::centred);
    
    if (m_DistributionSwitch.isVisible() && 
        m_processor.m_parameterVTS->getRawParameterValue(g_paramMode.ID)->load() == 2.0f) {
        g.drawText("Distribution", 
            m_DistributionSwitch.getX(),
            m_DistributionSwitch.getBottom() + static_cast<int>(5 * m_processor.getScaleFactor()),
            m_DistributionSwitch.getWidth(), 
            static_cast<int>(12 * m_processor.getScaleFactor()),
            juce::Justification::centred);
    }

    if (m_LowFreqSlider.isVisible()) {
        g.setFont(10.0f * m_processor.getScaleFactor());
        g.drawText("Low Freq",
            m_LowFreqSlider.getX(),
            static_cast<int>(m_LowFreqSlider.getY() - 15 * m_processor.getScaleFactor()),
            m_LowFreqSlider.getWidth(),
            static_cast<int>(15 * m_processor.getScaleFactor()),
            juce::Justification::centred);
        
        g.drawText("High Freq",
            m_HighFreqSlider.getX(),
            static_cast<int>(m_HighFreqSlider.getY() - 15 * m_processor.getScaleFactor()),
            m_HighFreqSlider.getWidth(),
            static_cast<int>(15 * m_processor.getScaleFactor()),
            juce::Justification::centred);
    }


    juce::String text2display = "OutOfPhase V " + juce::String(PLUGIN_VERSION_MAJOR) + "." + juce::String(PLUGIN_VERSION_MINOR) + "." + juce::String(PLUGIN_VERSION_PATCH);
    g.drawFittedText (text2display, getLocalBounds(), juce::Justification::bottomLeft, 1);
    
    // create shadow path
    juce::Path shadowPath;
    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.3f), 5, { 0, 2 });
    
    // add rounded rectangles for buttons
    shadowPath.addRoundedRectangle(m_FrostModeTextButton.getBounds().toFloat().expanded(2.0f), 10.0f);
    shadowPath.addRoundedRectangle(m_RandomModeTextButton.getBounds().toFloat().expanded(2.0f), 10.0f);
    shadowPath.addRoundedRectangle(m_ZeroModeTextButton.getBounds().toFloat().expanded(2.0f), 10.0f);
    shadowPath.addRoundedRectangle(m_FlipModeTextButton.getBounds().toFloat().expanded(2.0f), 10.0f);
    
    // add rectangles for sliders and plots
    shadowPath.addRectangle(m_PrePhasePlot.getBounds().toFloat().expanded(2.0f));
    shadowPath.addRectangle(m_PostPhasePlot.getBounds().toFloat().expanded(2.0f));
    
    if (m_DistributionSwitch.isVisible()) {
        shadowPath.addRoundedRectangle(m_DistributionSwitch.getBounds().toFloat().expanded(2.0f), 5.0f);
    }

    if (m_FreezeCaptureButton.isVisible())
    {
        shadowPath.addRoundedRectangle(m_FreezeCaptureButton.getBounds().toFloat().expanded(2.0f), 5.0f);
    }

    shadow.drawForPath(g, shadowPath);
}

void OutOfPhaseGUI::resized() {
    auto r = getLocalBounds();
    float scaleFactor = m_processor.getScaleFactor();

    // Dimensions
    float knobWidth = 100 * scaleFactor, knobHeight = 100 * scaleFactor;
    float distance = 40 * scaleFactor, margin = 10 * scaleFactor;

    int buttonWidth = static_cast<int>(knobWidth * 0.9);
    int buttonHeight = static_cast<int>(knobHeight * 0.4);
    int buttonSpacing = static_cast<int>(distance * 0.6);

    int displayHeight = getHeight() / 2;

    r.removeFromTop(static_cast<int>(displayHeight * 0.02)); 
    m_PrePhasePlot.setBounds(r.removeFromTop(static_cast<int>(displayHeight * 0.7))
                                 .reduced(static_cast<int>(margin * 0.7))
                                 .withTrimmedLeft(static_cast<int>(10 * scaleFactor))
                                 .withTrimmedRight(static_cast<int>(10 * scaleFactor)));

    r.removeFromTop(static_cast<int>(displayHeight * 0.05));
    m_PostPhasePlot.setBounds(r.removeFromTop(static_cast<int>(displayHeight * 0.23))
                                  .withTrimmedLeft(static_cast<int>(80 * scaleFactor))
                                  .withTrimmedRight(static_cast<int>(80 * scaleFactor)));

    // Sliders
    float sliderHeight = static_cast<float>(knobHeight * 2.5);
    float sliderWidth = static_cast<float>(knobWidth * 0.4);
    m_BlocksizeSlider.setBounds(static_cast<int>(distance * 0.6), static_cast<int>(distance * 4.2), static_cast<int>(sliderWidth), static_cast<int>(sliderHeight));
    m_DryWetSlider.setBounds(static_cast<int>(getWidth() - sliderWidth - distance * 0.6), static_cast<int>(distance * 4.2), static_cast<int>(sliderWidth), static_cast<int>(sliderHeight));

    float buttonsStartX = static_cast<float>(getWidth() / 2 - buttonWidth - buttonSpacing / 2);
    float buttonsStartY = static_cast<float>(getHeight() / 2 + distance / 2);

    m_FlipModeTextButton.setBounds(static_cast<int>(buttonsStartX), static_cast<int>(buttonsStartY), buttonWidth, buttonHeight);
    m_RandomModeTextButton.setBounds(static_cast<int>(buttonsStartX), static_cast<int>(buttonsStartY + buttonHeight + buttonSpacing), buttonWidth, buttonHeight);
    m_ZeroModeTextButton.setBounds(static_cast<int>(buttonsStartX + buttonWidth + buttonSpacing), static_cast<int>(buttonsStartY), buttonWidth, buttonHeight);
    m_FrostModeTextButton.setBounds(static_cast<int>(buttonsStartX + buttonWidth + buttonSpacing), static_cast<int>(buttonsStartY + buttonHeight + buttonSpacing), buttonWidth, buttonHeight);

    int freezeWidth = static_cast<int>(buttonWidth * 0.7);
    int freezeHeight = static_cast<int>(buttonHeight * 0.4);
    int freezeX = m_FrostModeTextButton.getX() + (m_FrostModeTextButton.getWidth() - freezeWidth) / 2;
    int freezeY = m_FrostModeTextButton.getBottom() + static_cast<int>(25 * scaleFactor);
    m_FreezeCaptureButton.setBounds(freezeX, freezeY, freezeWidth, freezeHeight);

    int switchWidth = static_cast<int>(buttonWidth * 0.7);
    int switchHeight = static_cast<int>(buttonHeight * 0.4);
    int switchX = m_RandomModeTextButton.getX() + (m_RandomModeTextButton.getWidth() - switchWidth) / 2;
    int switchY = m_RandomModeTextButton.getBottom() + static_cast<int>(25 * scaleFactor);
    m_DistributionSwitch.setBounds(switchX, switchY, switchWidth, switchHeight);

    float currentMode = m_processor.m_parameterVTS->getRawParameterValue(g_paramMode.ID)->load();
    bool randomMode = (currentMode == 2.0f);
    bool frostMode = (currentMode == 1.0f);
    
    if (m_DistributionSwitch.isVisible() != randomMode) {
        m_DistributionSwitch.setVisible(randomMode);
        if (randomMode) {
            bool isGaussian = m_processor.m_parameterVTS->getRawParameterValue(g_paramDistributionMode.ID)->load() == 1;
            m_DistributionSwitch.setToggleState(isGaussian, juce::dontSendNotification);
        }
    }
    
    if (m_FreezeCaptureButton.isVisible() != frostMode) {
        m_FreezeCaptureButton.setVisible(frostMode);
    }

    int bandButtonWidth = static_cast<int>(buttonWidth * 1.2f);
    int bandButtonHeight = static_cast<int>(buttonHeight * 0.7f);
    int bandY = getHeight() - bandButtonHeight - static_cast<int>(distance * 0.5f);
    int bandX = (getWidth() - bandButtonWidth) / 2;
    m_BandModeButton.setBounds(bandX, bandY, bandButtonWidth, bandButtonHeight);

    if (m_LowFreqSlider.isVisible()) {
        int freqControlWidth = static_cast<int>(knobWidth * 0.8f);
        int freqControlHeight = static_cast<int>(knobHeight * 0.8f);
        int freqY = bandY - freqControlHeight - static_cast<int>(buttonSpacing * 0.8f);
        int margin = static_cast<int>(buttonWidth * 0.2f);
        
        m_LowFreqSlider.setBounds(
            bandX - freqControlWidth/2 + margin,
            freqY,
            freqControlWidth,
            freqControlHeight
        );
        
        m_HighFreqSlider.setBounds(
            bandX + bandButtonWidth - freqControlWidth/2 - margin,
            freqY,
            freqControlWidth,
            freqControlHeight
        );
    }
};

void OutOfPhaseGUI::updateModeButtonStates()
{
    float currentMode = m_processor.m_parameterVTS->getRawParameterValue(g_paramMode.ID)->load();
    
    m_ZeroModeTextButton.setToggleState(currentMode == 0.0f, juce::dontSendNotification);
    m_FrostModeTextButton.setToggleState(currentMode == 1.0f, juce::dontSendNotification);
    m_RandomModeTextButton.setToggleState(currentMode == 2.0f, juce::dontSendNotification);
    m_FlipModeTextButton.setToggleState(currentMode == 3.0f, juce::dontSendNotification);
    
    bool isRandomMode = (currentMode == 2.0f);
    if (m_DistributionSwitch.isVisible() != isRandomMode) {
        m_DistributionSwitch.setVisible(isRandomMode);
        
        if (isRandomMode) {
            bool isGaussian = m_processor.m_parameterVTS->getRawParameterValue(g_paramDistributionMode.ID)->load() == 1;
            m_DistributionSwitch.setToggleState(isGaussian, juce::dontSendNotification);
        }
    }

    bool isFrostMode = (currentMode == 1.0f);
    if (m_FreezeCaptureButton.isVisible() != isFrostMode) {
        m_FreezeCaptureButton.setVisible(isFrostMode);
    }

    bool bandModeActive = m_BandModeButton.getToggleState();
    if (m_LowFreqSlider.isVisible() != bandModeActive) {
        m_LowFreqSlider.setVisible(bandModeActive);
        m_HighFreqSlider.setVisible(bandModeActive);
        resized();
    }
}

void OutOfPhaseGUI::timerCallback()
{
    std::vector<float> prePhaseDataCopy;
    std::vector<float> postPhaseDataCopy;
    
    prePhaseDataCopy = m_processor.m_algo.getPrePhaseData();
    postPhaseDataCopy = m_processor.m_algo.getPostPhaseData();
    
    m_PrePhasePlot.setPrePhaseData(std::move(prePhaseDataCopy));
    m_PostPhasePlot.setPostPhaseData(std::move(postPhaseDataCopy));
    
    updateModeButtonStates();
    
    repaint();
}

void OutOfPhaseGUI::parentHierarchyChanged()
{
    updateModeButtonStates();
    resized();
}