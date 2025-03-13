#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

#include "tools/SynchronBlockProcessor.h"
#include "PluginSettings.h"
#include "FFT.h"

#include "ComboBoxWithArrows.h"
#include "PhasePlot.h"

class OutOfPhaseAudioProcessor;

// This is how we define our parameter as globals to use it in the audio processor as well as in the editor
const struct
{
	const std::string ID = "ModeID";
	const std::string name = "Mode";
	const std::string mode1 = "Zero";
	const std::string mode2 = "Frost";
	const std::string mode3 = "Random";
	const std::string mode4 = "Flip";
	const int defaultValue = 1;
}g_paramMode;

const struct
{
	const std::string ID = "BlocksizeID";
	const std::string name = "Blocksize";
	const int defaultValue = 1024;
	const int minValue = 256;
	const int maxValue = 8192;
}g_paramBlocksize;

class OutOfPhaseAudio : public WOLA
{
public:
    OutOfPhaseAudio(OutOfPhaseAudioProcessor* processor);
    void prepareToPlay(double sampleRate, int max_samplesPerBlock, int max_channels);
	
	virtual int processWOLA(juce::AudioBuffer<float>& inBlock, juce::MidiBuffer& midiMessages);

    // parameter handling
  	void addParameter(std::vector < std::unique_ptr<juce::RangedAudioParameter>>& paramVector);
    void prepareParameter(std::unique_ptr<juce::AudioProcessorValueTreeState>&  vts);
    
    // some necessary info for the host
    int getLatency(){return m_Latency;};

	std::vector<float> getPhaseData() {
        juce::ScopedLock lock(dataMutex);
        return m_phaseData;
	}


private:
	OutOfPhaseAudioProcessor* m_processor;
    int m_Latency = 0;

	int m_synchronblocksize = 0;
	spectrum m_fftprocess;
	juce::AudioBuffer<float> m_realdata;
	juce::AudioBuffer<float> m_imagdata;
	std::vector<float> m_phaseData;
	juce::CriticalSection dataMutex;
};

class OutOfPhaseGUI : public juce::Component, public juce::Timer
{
public:
	OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts);

	void paint(juce::Graphics& g) override;
	void resized() override;

	void timerCallback() override;

private:
	OutOfPhaseAudioProcessor& m_processor;
    juce::AudioProcessorValueTreeState& m_apvts; 

	juce::Slider m_BlocksizeSlider;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> BlocksizeSliderAttachment;
	juce::Slider m_DryWetSlider;
	ComboBoxWithArrows m_ComboBoxWithArrows;

	juce::ComboBox m_ComboBoxDistribution;
	PhasePlot m_PhasePlot;
	std::vector<float> phaseDataPlot;
	juce::TextButton m_frostButton;
};
