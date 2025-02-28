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
    
    // if you have to place several components, use scaleFactor
    //int width = r.getWidth();
	//float scaleFactor = float(width)/g_minGuiSize_x;

    // use the given canvas in r
    juce::ignoreUnused(r);
}
