#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PhasePlot : public juce::Component
{

public:

    PhasePlot() {}

    void setPrePhaseData(const std::vector<float>& newPrePhaseData)
    {
        PrePhaseData = newPrePhaseData;
        repaint(); // Neuzeichnen des Plots
    }

    void setPostPhaseData(const std::vector<float>& newPostPhaseData)
    {
        PostPhaseData = newPostPhaseData;
        repaint(); // Neuzeichnen des Plots
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::white); // Hintergrund schwarz setzen

        // PrePhase

        g.setColour(juce::Colours::blue); // Linienfarbe weiß
        g.setOpacity(0.5f);

        // if (phaseData.empty()) return;

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        juce::Path PrePhasePath;
        float xStep = width / static_cast<float>(PrePhaseData.size());

        PrePhasePath.startNewSubPath(0, height / 2);

        for (size_t i = 0; i < PrePhaseData.size(); ++i)
        {
            float x = i * xStep;
            float y = height / 2 - (PrePhaseData[i] * (height / 2)); // Skalierung
            PrePhasePath.lineTo(x, y);
        }

        g.strokePath(PrePhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen

        // PostPhase

        juce::Path PostPhasePath;
        g.setColour(juce::Colours::pink); // Linienfarbe weiß
        g.setOpacity(0.5f);
        PostPhasePath.startNewSubPath(0, height / 2);

        for (size_t i = 0; i < PostPhaseData.size(); ++i)
        {
            float x = i * xStep;
            float y = height / 2 - (PostPhaseData[i] * (height / 2)); // Skalierung
            PostPhasePath.lineTo(x, y);
        }

        g.strokePath(PostPhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen
    }

private:
    std::vector<float> PrePhaseData;
    std::vector<float> PostPhaseData;
};