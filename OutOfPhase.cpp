#include <math.h>
#include "OutOfPhase.h"

#include "PluginProcessor.h"

#include "resources/images/paint_bin.h"
#include "resources/images/paper_bin.h"

OutOfPhaseAudio::OutOfPhaseAudio(OutOfPhaseAudioProcessor* processor)
:WOLA(), m_processor(processor)
{
}

void OutOfPhaseAudio::prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels)
{
    juce::ScopedLock lock(dataMutex);
    juce::ignoreUnused(sampleRate, max_samplesPerBlock,max_channels);

    float desired_blocksize = *m_processor->m_parameterVTS->getRawParameterValue(g_paramBlocksize.ID);

    int synchronblocksize;
    synchronblocksize = static_cast<int>(round(desired_blocksize));
    if (g_forcePowerOf2)
    {
        synchronblocksize = juce::nextPowerOfTwo(synchronblocksize);
    }
    m_synchronblocksize = synchronblocksize;

    m_realdata.setSize(0, 0, false, false, false);
    m_imagdata.setSize(0, 0, false, false, false);

    prepareWOLAprocessing(max_channels,synchronblocksize,WOLA::WOLAType::SqrtHann_over50);
    m_Latency += synchronblocksize;

    m_fftprocess.setFFTSize(synchronblocksize);
    m_realdata.setSize(max_channels,synchronblocksize/2+1);
    m_imagdata.setSize(max_channels,synchronblocksize/2+1);

    m_realdata.clear();
    m_imagdata.clear();

    initFrostPhaseData();

    m_PrePhaseData.resize(synchronblocksize/2+1);
    m_PostPhaseData.resize(synchronblocksize/2+1);
    
    std::fill(m_PrePhaseData.begin(), m_PrePhaseData.end(), 0.0f);
    std::fill(m_PostPhaseData.begin(), m_PostPhaseData.end(), 0.0f);
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

}

void OutOfPhaseAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    juce::ignoreUnused(vts);
}

int OutOfPhaseAudio::processWOLA(juce::AudioBuffer<float> &data, juce::MidiBuffer &midiMessages)
{
    juce::ScopedLock lock(dataMutex);
    juce::ignoreUnused(midiMessages);

    if (data.getNumSamples() == 0 || m_synchronblocksize == 0)
        return 0;

    float operatingMode = *m_processor->m_parameterVTS->getRawParameterValue(g_paramMode.ID);
    float dryWetMix = *m_processor->m_parameterVTS->getRawParameterValue(g_paramDryWet.ID);
    dryWetMix = juce::jlimit(0.0f, 1.0f, dryWetMix);

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

        std::vector<float> newPrePhaseData;
        std::vector<float> newPostPhaseData;
        newPrePhaseData.resize(m_synchronblocksize/2+1);
        newPostPhaseData.resize(m_synchronblocksize/2+1);
        //m_FrostPhaseData.resize(m_synchronblocksize/2+1);
        //DBG("FrostPhaseData size: " << m_FrostPhaseData.size());

        for (int nn = 0; nn< m_synchronblocksize/2+1; nn++)
        {
            float absval = sqrtf(realPtr[nn]*realPtr[nn] + imagPtr[nn]*imagPtr[nn]);
            float PrePhase = atan2f(imagPtr[nn],realPtr[nn]);
            float PostPhase = atan2f(imagPtr[nn],realPtr[nn]);

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
                        PostPhase = u1 * s * juce::MathConstants<float>::pi; // Scale to full -π to π range
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

            // rück
            realPtr[nn] = absval*cosf(PostPhase);
            imagPtr[nn] = absval*sinf(PostPhase);

            newPrePhaseData[nn] = PrePhase;
            newPostPhaseData[nn] = PostPhase;
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
        
        m_PrePhaseData = newPrePhaseData;
        m_PostPhaseData = newPostPhaseData;
    }

    return 0;
}

OutOfPhaseGUI::OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
:m_processor(p) ,m_apvts(apvts)
{
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this);
    tooltipWindow->setMillisecondsBeforeTipAppears(500);
    
    startTimerHz(30); // 30 Hz update rate

    //addAndMakeVisible(m_ComboBoxWithArrows);
    m_ComboBoxWithArrows.setOnSelectionChanged([this](int newId)
    {
        m_apvts.getParameterAsValue(g_paramMode.ID) = newId;
        if (newId == 1)
        {
            m_ComboBoxDistribution.setVisible(false);
            m_frostButton.setVisible(true);
            resized();
        }
        else if (newId == 2)
        {
            m_frostButton.setVisible(false);
            m_ComboBoxDistribution.setVisible(true);
            resized();
        }
        else 
        {
            m_frostButton.setVisible(false);
            m_ComboBoxDistribution.setVisible(false);
        }
    });

    addAndMakeVisible(m_frostButton);
    m_frostButton.setButtonText("Frost");
    m_frostButton.setVisible(false);
    m_frostButton.onClick = [this]
    {
        m_processor.m_algo.updateFrostPhaseData();
    };

    m_BlocksizeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    m_BlocksizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    m_BlocksizeSlider.setDoubleClickReturnValue(true, 512);
    m_BlocksizeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    m_BlocksizeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lightgrey);
    m_BlocksizeSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::lightgrey);
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

    m_DryWetSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    m_DryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    m_DryWetSlider.setNumDecimalPlacesToDisplay(2);
    m_DryWetSlider.setDoubleClickReturnValue(true, 0.5f);
    m_DryWetSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    m_DryWetSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::lightgrey);
    m_DryWetSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::lightgrey);
    DryWetSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramDryWet.ID, m_DryWetSlider);
    addAndMakeVisible(m_DryWetSlider);

    // example options, standard should be uniform distribution
    m_ComboBoxDistribution.addItem("Uniform", 1);
    m_ComboBoxDistribution.addItem("Gaussian", 2);
    addAndMakeVisible(m_ComboBoxDistribution);
    m_ComboBoxDistribution.setVisible(false);
    m_ComboBoxDistribution.setSelectedId(1);

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

    m_paintImage = juce::ImageFileFormat::loadFrom(paint_bin, paint_bin_len);
    m_paperImage = juce::ImageFileFormat::loadFrom(paper_bin, paper_bin_len);
}

void OutOfPhaseGUI::paint(juce::Graphics &g)
{
    g.fillAll (juce::Colours::bisque);

    g.drawImageWithin(m_paperImage, 0, 0, getWidth(), getHeight(),
                       juce::RectanglePlacement::fillDestination);

    g.drawImageWithin(m_paintImage, -getWidth()/4, getHeight()*0.84, getWidth(), getHeight()/4,
                       juce::RectanglePlacement::fillDestination);
    

    
    g.drawImageWithin(m_paintImage, getWidth() - getWidth() * 0.7, -getHeight()/3, getWidth(), getHeight(),
    juce::RectanglePlacement::fillDestination);
    

    g.setColour (juce::Colours::darkslategrey);
    g.setFont (12.0f * m_processor.getScaleFactor());

    int labelWidth = static_cast<int>(m_BlocksizeSlider.getWidth() * 1.5f);   

    g.drawText("Block Size", 
        m_BlocksizeSlider.getX() - (labelWidth - m_BlocksizeSlider.getWidth()) / 2,
        m_BlocksizeSlider.getY() - 15 * m_processor.getScaleFactor(),
        labelWidth, 
        20 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    g.drawText("Dry/Wet", 
        m_DryWetSlider.getX() - (labelWidth - m_DryWetSlider.getWidth()) / 2,
        m_DryWetSlider.getY() - 15 * m_processor.getScaleFactor(),
        labelWidth, 
        20 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    g.setFont(14.0f * m_processor.getScaleFactor());
    g.setColour(juce::Colours::darkslategrey);
    
    float leftEdge = std::min(m_RandomModeTextButton.getX(), m_FrostModeTextButton.getX());
    float rightEdge = std::max(m_ZeroModeTextButton.getRight(), m_FlipModeTextButton.getRight());
    

    juce::Rectangle<int> buttonGroupBounds(
        leftEdge, 
        m_ZeroModeTextButton.getY(),
        rightEdge - leftEdge,
        m_FlipModeTextButton.getBottom() - m_ZeroModeTextButton.getY()
    );
    
    g.drawText("Phase Mode", 
        buttonGroupBounds.getX(),
        buttonGroupBounds.getY() - 25 * m_processor.getScaleFactor(),
        buttonGroupBounds.getWidth(), 
        20 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    g.setFont(10.0f * m_processor.getScaleFactor());

    g.drawText("Zero-Phase", 
        m_ZeroModeTextButton.getX(),
        m_ZeroModeTextButton.getBottom() + 5 * m_processor.getScaleFactor(),
        m_ZeroModeTextButton.getWidth(), 
        12 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    g.drawText("Random-Phase", 
        m_RandomModeTextButton.getX(),
        m_RandomModeTextButton.getBottom() + 5 * m_processor.getScaleFactor(),
        m_RandomModeTextButton.getWidth(), 
        12 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    g.drawText("Phase-Frost", 
        m_FrostModeTextButton.getX(),
        m_FrostModeTextButton.getBottom() + 5 * m_processor.getScaleFactor(),
        m_FrostModeTextButton.getWidth(), 
        12 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    g.drawText("Phase-Flip", 
        m_FlipModeTextButton.getX(),
        m_FlipModeTextButton.getBottom() + 5 * m_processor.getScaleFactor(),
        m_FlipModeTextButton.getWidth(), 
        12 * m_processor.getScaleFactor(),
        juce::Justification::centred);

    juce::String text2display = "OutOfPhase V " + juce::String(PLUGIN_VERSION_MAJOR) + "." + juce::String(PLUGIN_VERSION_MINOR) + "." + juce::String(PLUGIN_VERSION_PATCH);
    g.drawFittedText (text2display, getLocalBounds(), juce::Justification::bottomLeft, 1);

    auto shadowBounds = m_FrostModeTextButton.getBounds().toFloat().expanded(2.0f); // Expand for shadow
    juce::DropShadow shadow(juce::Colours::black.withAlpha(0.3f), 5, { 0, 2 });
    juce::Path roundedRect;
    roundedRect.addRoundedRectangle(shadowBounds, 10.0f); // Add rounded rectangle with corner radius 10.0f
    
    shadowBounds = m_RandomModeTextButton.getBounds().toFloat().expanded(2.0f); // Expand for shadow
    roundedRect.addRoundedRectangle(shadowBounds, 10.0f); // Add rounded rectangle with corner radius 10.0f
    
    shadowBounds = m_ZeroModeTextButton.getBounds().toFloat().expanded(2.0f); // Expand for shadow
    roundedRect.addRoundedRectangle(shadowBounds, 10.0f); // Add rounded rectangle with corner radius 10.0f

    shadowBounds = m_FlipModeTextButton.getBounds().toFloat().expanded(2.0f); // Expand for shadow
    roundedRect.addRoundedRectangle(shadowBounds, 10.0f); // Add rounded rectangle with corner radius 5.0f
    
    shadowBounds = m_PrePhasePlot.getBounds().toFloat().expanded(2.0f); // Expand for shadow
    roundedRect.addRectangle(shadowBounds); // Add rounded rectangle with corner radius 5.0f

    shadowBounds = m_PostPhasePlot.getBounds().toFloat().expanded(2.0f); // Expand for shadow
    roundedRect.addRectangle(shadowBounds); // Add rounded rectangle with corner radius 5.0f

    shadow.drawForPath(g, roundedRect);

}

void OutOfPhaseGUI::resized() {
    auto r = getLocalBounds();
    float scaleFactor = m_processor.getScaleFactor();

    // Dimensions
    float knobWidth = 100 * scaleFactor, knobHeight = 100 * scaleFactor;
    float distance = 40 * scaleFactor, margin = 10 * scaleFactor;
    float comboxWidth = 150 * scaleFactor, comboxHeight = 20 * scaleFactor;
    int displayHeight = getHeight() / 2;

    r.removeFromTop(displayHeight * 0.02); // Add more space between the two plots
    // Plots
    m_PrePhasePlot.setBounds(r.removeFromTop(displayHeight * 0.7)
                                 .reduced(margin * 0.7)
                                 .withTrimmedLeft(10 * scaleFactor)
                                 .withTrimmedRight(10 * scaleFactor));

    r.removeFromTop(displayHeight * 0.05); // Add more space between the two plots

    m_PostPhasePlot.setBounds(r.removeFromTop(displayHeight * 0.23)
                                  .withTrimmedLeft(80 * scaleFactor)
                                  .withTrimmedRight(80 * scaleFactor));

    // ComboBox Centered
    m_ComboBoxWithArrows.setBounds(
        (getWidth() - comboxWidth) / 2, 
        (getHeight() - comboxHeight) / 2, 
        comboxWidth, 
        comboxHeight
    );

    // Sliders
    float sliderHeight = knobHeight * 2.5, sliderWidth = knobWidth * 0.4;
    m_BlocksizeSlider.setBounds(distance * 0.6, distance * 4.2, sliderWidth, sliderHeight);
    m_DryWetSlider.setBounds(getWidth() - sliderWidth - distance * 0.6, distance * 4.2, sliderWidth, sliderHeight);

    // Optional ComboBoxes and Buttons
    if (m_ComboBoxDistribution.isVisible()) {
        m_ComboBoxDistribution.setBounds((getWidth() - comboxWidth) / 2, (getHeight() - comboxHeight) / 2 + distance, comboxWidth, comboxHeight);
    }
    if (m_frostButton.isVisible()) {
        m_frostButton.setBounds((getWidth() - comboxWidth) / 2, (getHeight() - comboxHeight) / 2 + distance, comboxWidth, comboxHeight);
    }

    // Radio Buttons
    int buttonWidth = static_cast<int>(knobWidth * 0.9);
    int buttonHeight = static_cast<int>(knobHeight * 0.4);
    int buttonSpacing = static_cast<int>(distance * 0.6);

    float buttonsStartX = getWidth() / 2 - buttonWidth - buttonSpacing / 2;
    float buttonsStartY = getHeight() / 2 + distance / 2;

    m_RandomModeTextButton.setBounds(buttonsStartX, buttonsStartY, buttonWidth, buttonHeight);
    m_FrostModeTextButton.setBounds(buttonsStartX, buttonsStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
    m_ZeroModeTextButton.setBounds(buttonsStartX + buttonWidth + buttonSpacing, buttonsStartY, buttonWidth, buttonHeight);
    m_FlipModeTextButton.setBounds(buttonsStartX + buttonWidth + buttonSpacing, buttonsStartY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight);
}

void OutOfPhaseGUI::timerCallback()
{
    std::vector<float> PrePhaseDataPlot = m_processor.m_algo.getPrePhaseData(); // Retrieve data from processor
    std::vector<float> PostPhaseDataPlot = m_processor.m_algo.getPostPhaseData(); // Retrieve data from processor
    m_PrePhasePlot.setPrePhaseData(PrePhaseDataPlot); // Update plot
    m_PostPhasePlot.setPostPhaseData(PostPhaseDataPlot); // Update plot
}
