#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class PhasePlot : public juce::Component
{

public:

    PhasePlot() {}

    void setPhaseData(const std::vector<float>& newPhaseData)
    {
        phaseData = newPhaseData;
        repaint(); // Neuzeichnen des Plots
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black); // Hintergrund schwarz setzen
        g.setColour(juce::Colours::white); // Linienfarbe weiß

        // if (phaseData.empty()) return;

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        juce::Path phasePath;
        float xStep = width / static_cast<float>(phaseData.size());

        phasePath.startNewSubPath(0, height / 2);

        for (size_t i = 0; i < phaseData.size(); ++i)
        {
            float x = i * xStep;
            float y = height / 2 - (phaseData[i] * (height / 2)); // Skalierung
            phasePath.lineTo(x, y);
        }

        g.strokePath(phasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen
    }

private:
    std::vector<float> phaseData;
};