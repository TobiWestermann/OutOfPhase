#include <math.h>
#include "OutOfPhase.h"

#include "PluginProcessor.h"

OutOfPhaseAudio::OutOfPhaseAudio(OutOfPhaseAudioProcessor* processor)
:WOLA(), m_processor(processor)
{
}

void OutOfPhaseAudio::prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels)
{
    juce::ignoreUnused(max_samplesPerBlock,max_channels);
    int synchronblocksize;
    synchronblocksize = static_cast<int>(round(g_desired_blocksize_ms * sampleRate * 0.001)); // 0.001 to transform ms to seconds;
    if (g_forcePowerOf2)
    {
        int nextpowerof2 = int(log2(synchronblocksize))+1;
        synchronblocksize = int(pow(2,nextpowerof2));
    }
    m_synchronblocksize = synchronblocksize;
    //prepareSynchronProcessing(max_channels,synchronblocksize);
    prepareWOLAprocessing(max_channels,synchronblocksize,WOLA::WOLAType::SqrtHann_over50);
    m_Latency += synchronblocksize;
    // here your code
    m_fftprocess.setFFTSize(synchronblocksize);
    m_realdata.setSize(max_channels,synchronblocksize/2+1);
    m_imagdata.setSize(max_channels,synchronblocksize/2+1);
}

/* int OutOfPhaseAudio::processSynchronBlock(juce::AudioBuffer<float> & buffer, juce::MidiBuffer &midiMessages, int NrOfBlocksSinceLastProcessBlock)
{
    processWOLA(buffer, midiMessages);
    juce::ignoreUnused( NrOfBlocksSinceLastProcessBlock);
    return 0;
} */

void OutOfPhaseAudio::addParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &paramVector)
{
    // this is just a placeholder (necessary for compiling/testing the template)
    paramVector.push_back(std::make_unique<AudioParameterChoice>(g_paramMode.ID,
        g_paramMode.name,
        StringArray {g_paramMode.mode1, g_paramMode.mode2, g_paramMode.mode3 , g_paramMode.mode4}, g_paramMode.defaultValue
                        ));

}

void OutOfPhaseAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    juce::ignoreUnused(vts);
}

int OutOfPhaseAudio::processWOLA(juce::AudioBuffer<float> &data, juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);

    int numchns = data.getNumChannels();

    for (int cc = 0 ; cc < numchns; cc++)
    {
    // FFT 
        auto dataPtr = data.getWritePointer(cc);
        auto realPtr = m_realdata.getWritePointer(cc);
        auto imagPtr = m_imagdata.getWritePointer(cc);
        m_fftprocess.fft(dataPtr,realPtr,imagPtr);

        for (int nn = 0; nn< m_synchronblocksize/2+1; nn++)
        {
            float absval = sqrtf(realPtr[nn]*realPtr[nn] + imagPtr[nn]*imagPtr[nn]);
            float phase = atan2f(imagPtr[nn],realPtr[nn]);

            if (nn>30)
                absval = 0.f;

            // rück
            realPtr[nn] = absval*cosf(phase);
            imagPtr[nn] = absval*sinf(phase);
        }

    // IFFT
        m_fftprocess.ifft(realPtr,imagPtr, dataPtr);

    }

    return 0;
}

OutOfPhaseGUI::OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
:m_processor(p) ,m_apvts(apvts)
{   
    addAndMakeVisible(m_ComboBoxWithArrows);
    m_ComboBoxWithArrows.setOnSelectionChanged([this](int newId)
    {
        bool shouldShowDistribution = (newId == 2);
        m_ComboBoxDistribution.setVisible(shouldShowDistribution);
        if (newId == 2)
    {
        // Beispielhafte Phasendaten (Sinusförmige Phase für Visualisierung)
        std::vector<float> phaseData;
        for (int i = 0; i < 100; ++i)
        {
            float phase = std::sin(i * juce::MathConstants<float>::twoPi / 100);
            phaseData.push_back(phase);
        }
        m_PhasePlot.setPhaseData(phaseData);
    }


        resized();

        m_apvts.getParameterAsValue(g_paramMode.ID) = newId;
    });

    m_BlocksizeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_BlocksizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(m_BlocksizeSlider);

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
}