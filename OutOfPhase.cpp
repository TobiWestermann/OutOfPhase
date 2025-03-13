#include <math.h>
#include "OutOfPhase.h"

#include "PluginProcessor.h"

OutOfPhaseAudio::OutOfPhaseAudio(OutOfPhaseAudioProcessor* processor)
:WOLA(), m_processor(processor)
{
}

void OutOfPhaseAudio::prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels)
{
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

}

void OutOfPhaseAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    juce::ignoreUnused(vts);
}

int OutOfPhaseAudio::processWOLA(juce::AudioBuffer<float> &data, juce::MidiBuffer &midiMessages)
{
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

        std::vector<float> newPhaseData;
        newPhaseData.resize(m_synchronblocksize/2+1);

        for (int nn = 0; nn< m_synchronblocksize/2+1; nn++)
        {
            float absval = sqrtf(realPtr[nn]*realPtr[nn] + imagPtr[nn]*imagPtr[nn]);
            float phase = atan2f(imagPtr[nn],realPtr[nn]);

            if (operatingMode == 1) // zero
            {
                phase = 0;
            }
            else if (operatingMode == 2) // frost
            {
                phase = 0.5f * juce::MathConstants<float>::pi;
            }
            else if (operatingMode == 3) // random
            {
                phase = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
            }
            else if (operatingMode == 4) // flip
            {
                phase = phase + juce::MathConstants<float>::pi;
            }

            // rück
            realPtr[nn] = absval*cosf(phase);
            imagPtr[nn] = absval*sinf(phase);

            newPhaseData[nn] = phase;
        }

        // IFFT
        m_fftprocess.ifft(realPtr,imagPtr, dataPtr);

        m_phaseData = newPhaseData;
    }

    return 0;
}

OutOfPhaseGUI::OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
:m_processor(p) ,m_apvts(apvts)
{   
    startTimerHz(30); // 30 Hz update rate

    addAndMakeVisible(m_ComboBoxWithArrows);
    m_ComboBoxWithArrows.setOnSelectionChanged([this](int newId)
    {
        m_apvts.getParameterAsValue(g_paramMode.ID) = newId;
        if (newId == 2)
        {
            m_frostButton.setVisible(true);
        }
        else 
        {
            m_frostButton.setVisible(false);
        }
    });

    addAndMakeVisible(m_frostButton);
    m_frostButton.setButtonText("Frost");
    m_frostButton.setVisible(false);

    m_BlocksizeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_BlocksizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(m_BlocksizeSlider);
    BlocksizeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        *m_processor.m_parameterVTS, g_paramBlocksize.ID, m_BlocksizeSlider);

    m_DryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_DryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(m_DryWetSlider);

    // example options, standard should be uniform distribution
    m_ComboBoxDistribution.addItem("Uniform", 1);
    m_ComboBoxDistribution.addItem("Gaussian", 2);
    addAndMakeVisible(m_ComboBoxDistribution);

    addAndMakeVisible(m_PhasePlot);

}

void OutOfPhaseGUI::paint(juce::Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).brighter(0.3f));

    g.setColour (juce::Colours::white);
    g.setFont (12.0f);
    
    juce::String text2display = "OutOfPhase V " + juce::String(PLUGIN_VERSION_MAJOR) + "." + juce::String(PLUGIN_VERSION_MINOR) + "." + juce::String(PLUGIN_VERSION_PATCH);
    g.drawFittedText (text2display, getLocalBounds(), juce::Justification::bottomLeft, 1);

}

void OutOfPhaseGUI::resized()
{
	auto r = getLocalBounds();
    
    int height = getHeight();
    // int width = getWidth();

    float scaleFactor = m_processor.getScaleFactor();

    // general button size
    float knobWidth = 100 * scaleFactor;
    float knobHeight = 100 * scaleFactor;
    float distance = 40 * scaleFactor;

    float comboxWidth = 150 * scaleFactor;
    float comboxHeight = 20 * scaleFactor;

    // Display for phase in upper half of the GUI
    int displayHeight = height / 2;
    // int plotHeight = displayHeight / 3;
    m_PhasePlot.setBounds(r.removeFromTop(displayHeight));
    
    
    // Combox in the middle
    float comboxY = r.getY() + r.getHeight() / 2 - comboxHeight / 2;
    float comboxX = r.getX() + r.getWidth() / 2 - comboxWidth / 2;
    m_ComboBoxWithArrows.setBounds(static_cast<int>(comboxX), static_cast<int>(comboxY), static_cast<int>(comboxWidth), static_cast<int>(comboxHeight));

    // button blocksize left from combobox
    float blocksizeX = comboxX - knobWidth - distance;
    float blocksizeY = comboxY - comboxHeight;
    m_BlocksizeSlider.setBounds(static_cast<int>(blocksizeX), static_cast<int>(blocksizeY), static_cast<int>(knobWidth), static_cast<int>(knobHeight));

    // button drywet right from combobox
    float drywetX = comboxX + comboxWidth + distance;
    float drywetY = comboxY - comboxHeight;
    m_DryWetSlider.setBounds(static_cast<int>(drywetX), static_cast<int>(drywetY), static_cast<int>(knobWidth), static_cast<int>(knobHeight));

    if (m_ComboBoxDistribution.isVisible())
    {
        float distributionX = comboxX;
        float distributionY = comboxY + distance;
        m_ComboBoxDistribution.setBounds(static_cast<int>(distributionX), static_cast<int>(distributionY), static_cast<int>(comboxWidth), static_cast<int>(comboxHeight));
    }

    if (m_frostButton.isVisible())
    {
        float frostX = comboxX;
        float frostY = comboxY + 2 * distance;
        m_frostButton.setBounds(static_cast<int>(frostX), static_cast<int>(frostY), static_cast<int>(comboxWidth), static_cast<int>(comboxHeight));
    }
}

void OutOfPhaseGUI::timerCallback()
{
    phaseDataPlot = m_processor.m_algo.getPhaseData(); // Retrieve data from processor
    m_PhasePlot.setPhaseData(phaseDataPlot); // Update plot
}
