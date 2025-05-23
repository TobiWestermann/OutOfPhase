#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

#include "tools/SynchronBlockProcessor.h"
#include "PluginSettings.h"
#include "libs/FFT.h"

#include "customComponents/PhasePlot.h"
#include "customComponents/DiscreteSlider.h"
#include "customComponents/FrostButton.h"
#include "customComponents/RandomButton.h"
#include "customComponents/ZeroButton.h"
#include "customComponents/FlipButton.h"
#include "customComponents/DistributionSwitch.h"
#include "customComponents/FreezeCaptureButton.h"
#include "customComponents/BandModeButton.h"
#include "customComponents/FrequencyKnob.h"

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
	const std::string ID = "DistributionModeID";
	const std::string name = "Distribution Mode";
	const std::string mode1 = "Uniform";
	const std::string mode2 = "Gaussian";
	const int defaultValue = 1;
} g_paramDistributionMode;

const struct
{
	const std::string ID = "BlocksizeID";
	const std::string name = "Blocksize";
	const int defaultValue = 1024;
	const int minValue = 256;
	const int maxValue = 8192;
}g_paramBlocksize;

const struct
{
	const std::string ID = "DryWetID";
	const std::string name = "DryWet";
	const float defaultValue = 1;
	const float minValue = 0;
	const float maxValue = 1;
}g_paramDryWet;

const struct
{
    const std::string ID = "bandmode";
    const std::string name = "Band Mode";
    const int defaultValue = 0;
} g_paramBandMode;

const struct
{
    const std::string ID = "lowfreq";
    const std::string name = "Low Frequency";
    const float minValue = 20.0f;
    // maxValue will be set dynamically based on sample rate
    const float defaultValue = 100.0f;
} g_paramLowFreq;

const struct
{
    const std::string ID = "highfreq";
    const std::string name = "High Frequency";
    const float minValue = 20.0f;
    // maxValue will be set dynamically based on sample rate
    const float defaultValue = 5000.0f;
} g_paramHighFreq;

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
    int getLatency(){return m_Latency;}

	void updateFrequencyRange(double sampleRate);

	std::vector<float> getPrePhaseData() {
        juce::ScopedLock lock(dataMutex);
        return m_PrePhaseData;
	}

	std::vector<float> getPostPhaseData() {
        juce::ScopedLock lock(dataMutex);
        return m_PostPhaseData;
	}

	void acquireLock()
    {
        dataMutex.enter();
    }
    
    void releaseLock()
    {
        dataMutex.exit();
    }
	
	void updateFrostPhaseData() {
		juce::ScopedLock lock(dataMutex);
		m_FrostPhaseData = m_PrePhaseData;
	}

	void initFrostPhaseData() {
		juce::ScopedLock lock(dataMutex);
		m_FrostPhaseData.resize(m_synchronblocksize/2+1);
		std::fill(m_FrostPhaseData.begin(), m_FrostPhaseData.end(), 0.0f);
		//DBG("FrostPhaseData size: " << m_FrostPhaseData.size());
	}

private:
	OutOfPhaseAudioProcessor* m_processor;
    int m_Latency = 0;

	int m_synchronblocksize = 0;
	spectrum m_fftprocess;
	juce::AudioBuffer<float> m_realdata;
	juce::AudioBuffer<float> m_imagdata;

	std::vector<float> m_PrePhaseData;
	std::vector<float> m_PostPhaseData;

	std::vector<float> m_tempPrePhaseData;
	std::vector<float> m_tempPostPhaseData;

	std::vector<float> m_FrostPhaseData;
	juce::CriticalSection dataMutex;
};

class OutOfPhaseGUI : public juce::Component, public juce::Timer
{
public:
	OutOfPhaseGUI(OutOfPhaseAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts);
	~OutOfPhaseGUI();

	void setBackgroundImages(const juce::Image& paintImg, const juce::Image& paperImg);
	void paint(juce::Graphics& g) override;
	void resized() override;
	void parentHierarchyChanged() override;
	void updateModeButtonStates();

	void updateFrequencyKnobRanges(double sampleRate)
	{
		m_LowFreqKnob.updateRange(sampleRate);
		m_HighFreqKnob.updateRange(sampleRate);
	}

	void timerCallback() override;

private:
	OutOfPhaseAudioProcessor& m_processor;
    juce::AudioProcessorValueTreeState& m_apvts;

	juce::Image m_paintImage;
	juce::Image m_paperImage;
	bool m_imagesLoaded = false;
	
	DiscreteSlider m_BlocksizeSlider;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> BlocksizeSliderAttachment;
	CustomSlider m_DryWetSlider;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> DryWetSliderAttachment;
	PhasePlot m_PrePhasePlot;
	PhasePlot m_PostPhasePlot;
	//std::vector<float> phaseDataPlot;

	std::unique_ptr<juce::TooltipWindow> tooltipWindow;
	
	ZeroButton m_ZeroModeTextButton;
	FrostButton m_FrostModeTextButton;
	RandomButton m_RandomModeTextButton;
	FlipButton m_FlipModeTextButton;
	DistributionSwitch m_DistributionSwitch;
	FreezeCaptureButton m_FreezeCaptureButton;

	BandModeButton m_BandModeButton;
	FrequencyKnob m_LowFreqKnob{FrequencyKnob::LowFreq};
	FrequencyKnob m_HighFreqKnob{FrequencyKnob::HighFreq};
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> BandModeButtonAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> LowFreqKnobAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> HighFreqKnobAttachment;
};
