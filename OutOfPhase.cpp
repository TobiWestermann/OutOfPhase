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
        int nextpowerof2 = int(log2(synchronblocksize))+1;
        synchronblocksize = int(pow(2,nextpowerof2));
    }
    m_synchronblocksize = synchronblocksize;

    prepareWOLAprocessing(max_channels,synchronblocksize,WOLA::WOLAType::SqrtHann_over50);
    m_Latency += synchronblocksize;
    // here your code
    m_fftprocess.setFFTSize(synchronblocksize);
    m_realdata.setSize(max_channels,synchronblocksize/2+1);
    m_imagdata.setSize(max_channels,synchronblocksize/2+1);

    initFrostPhaseData();
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

    float operatingMode = *m_processor->m_parameterVTS->getRawParameterValue(g_paramMode.ID);

    int numchns = data.getNumChannels();

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
                PostPhase = m_FrostPhaseData[nn];
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

        m_PrePhaseData = newPrePhaseData;
        m_PostPhaseData = newPostPhaseData;
    }

    return 0;
}

OutOfPhaseGUI::OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
:m_processor(p) ,m_apvts(apvts)
{   
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
    addAndMakeVisible(m_BlocksizeSlider);
    BlocksizeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramBlocksize.ID, m_BlocksizeSlider);
    m_BlocksizeSlider.onValueChange = [this]
    {
        m_processor.m_algo.prepareToPlay(44100, 512, 2);
    };

    m_DryWetSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    m_DryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
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
    

    g.setColour (juce::Colours::grey);
    g.setFont (12.0f);
    
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
