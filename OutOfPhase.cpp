#include <math.h>
#include "OutOfPhase.h"

#include "PluginProcessor.h"

OutOfPhaseAudio::OutOfPhaseAudio(OutOfPhaseAudioProcessor* processor)
:SynchronBlockProcessor(), m_processor(processor)
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
    prepareSynchronProcessing(max_channels,synchronblocksize);
    m_Latency += synchronblocksize;
    // here your code

}

int OutOfPhaseAudio::processSynchronBlock(juce::AudioBuffer<float> & buffer, juce::MidiBuffer &midiMessages, int NrOfBlocksSinceLastProcessBlock)
{
    juce::ignoreUnused(buffer, midiMessages, NrOfBlocksSinceLastProcessBlock);
    return 0;
}

void OutOfPhaseAudio::addParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &paramVector)
{
    // this is just a placeholder (necessary for compiling/testing the template)
    paramVector.push_back(std::make_unique<AudioParameterFloat>(g_paramExample.ID,
        g_paramExample.name,
        NormalisableRange<float>(g_paramExample.minValue, g_paramExample.maxValue),
        g_paramExample.defaultValue,
        AudioParameterFloatAttributes().withLabel (g_paramExample.unitName)
                                        .withCategory (juce::AudioProcessorParameter::genericParameter)
                                        // or two additional lines with lambdas to convert data for display
                                        // .withStringFromValueFunction (std::move ([](float value, int MaxLen) { value = int(exp(value) * 10) * 0.1f;  return (String(value, MaxLen) + " Hz"); }))
                                        // .withValueFromStringFunction (std::move ([](const String& text) {return text.getFloatValue(); }))
                        ));

}

void OutOfPhaseAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
    juce::ignoreUnused(vts);
}


OutOfPhaseGUI::OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts)
:m_processor(p) ,m_apvts(apvts)
{
    addAndMakeVisible(m_ComboBoxWithArrows);

    m_BlocksizeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_BlocksizeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    addAndMakeVisible(m_BlocksizeSlider);

    m_DryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_DryWetSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    addAndMakeVisible(m_DryWetSlider);

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
    int width = getWidth();

    float scaleFactor = m_processor.getScaleFactor();

    // general button size
    int knobWidth = 80 * scaleFactor;
    int knobHeight = 80 * scaleFactor;
    int distance = 20 * scaleFactor;

    int comboxWidth = 150 * scaleFactor;
    int comboxHeight = 20 * scaleFactor;

    // Display for phase in upper half of the GUI
    int displayHeight = height / 2;
    
    // remove from top so buttons are in lower half
    r.removeFromTop(displayHeight);
    
    // Combox in the middle
    int comboxY = r.getY() + r.getHeight() / 2 - comboxHeight / 2;
    int comboxX = r.getX() + r.getWidth() / 2 - comboxWidth / 2;
    m_ComboBoxWithArrows.setBounds(comboxX, comboxY, comboxWidth, comboxHeight);

}
