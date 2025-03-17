#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

class PhasePlot : public juce::Component
{
public:
    PhasePlot();

    void setPrePhaseData(const std::vector<float>& newPrePhaseData);
    void setPostPhaseData(const std::vector<float>& newPostPhaseData);
    void paint(juce::Graphics& g) override;

private:
    std::vector<float> PrePhaseData;
    std::vector<float> PostPhaseData;
    juce::Image m_glassImage;
};