#include <math.h>
#include "YourPluginName.h"


YourPluginNameAudio::YourPluginNameAudio()
:SynchronBlockProcessor()
{
}

void YourPluginNameAudio::prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels)
{
    int synchronblocksize;
    synchronblocksize = (g_desired_blocksize_ms*sampleRate*0.001);
    if (g_forcePowerOf2)
    {
        int nextpowerof2 = int(log2(synchronblocksize))+1;
        synchronblocksize = int(pow(2,nextpowerof2));
    }
    prepareSynchronProcessing(max_channels,synchronblocksize);
    m_Latency += synchronblocksize;
    // here your code

}

int YourPluginNameAudio::processSynchronBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &midiMessages)
{
    return 0;
}

void YourPluginNameAudio::addParameter(std::vector<std::unique_ptr<juce::RangedAudioParameter>> &paramVector)
{
    // this is just a placeholder (necessary for compiling/testing the template)
    paramVector.push_back(std::make_unique<AudioParameterFloat>(g_paramExample.ID,
        g_paramExample.name,
        NormalisableRange<float>(g_paramExample.minValue, g_paramExample.maxValue),
        g_paramExample.defaultValue,
        g_paramExample.unitName,
        AudioProcessorParameter::genericParameter));
// these are to additional lines wit lambdas to convert data
        // [](float value, int MaxLen) { value = int(exp(value) * 10) * 0.1;  return (String(value, MaxLen) + " Hz"); },
        // [](const String& text) {return text.getFloatValue(); }));




}

void YourPluginNameAudio::prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState> &vts)
{
}

YourPluginNameGUI::YourPluginNameGUI()
{
}
