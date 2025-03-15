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
        g.fillAll(juce::Colours::black); // Hintergrund schwarz setzen

        g.setColour(juce::Colours::blue); // Linienfarbe blau
        g.setOpacity(0.5f);
    
        if (PrePhaseData.empty()) return;
    
        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());
        size_t N = PrePhaseData.size();
    
        juce::Path PrePhasePath;
    
        // Logarithmische Skalierung basierend auf Indizes
        float logMin = std::log10(1);
        float logMax = std::log10(N);
    
        // Startpunkt setzen
        float xStart = (std::log10(1) - logMin) / (logMax - logMin) * width;
        float yStart = height / 2 - (PrePhaseData[0] * (height / 2));
        PrePhasePath.startNewSubPath(xStart, yStart);
    
        for (size_t i = 1; i < N; ++i)
        {
            float logIndex = std::log10(i + 1);
            float normX = (logIndex - logMin) / (logMax - logMin);
            float x = normX * width; // Logarithmische Skalierung der x-Koordinate
    
            float y = height / 2 - (PrePhaseData[i] * (height / 2)); // Phase skalieren
    
            PrePhasePath.lineTo(x, y);
        }
    
        g.strokePath(PrePhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen

        // PostPhase

        // juce::Path PostPhasePath;
        // g.setColour(juce::Colours::pink); // Linienfarbe weiß
        // g.setOpacity(0.5f);
        // PostPhasePath.startNewSubPath(0, height / 2);

        // for (size_t i = 0; i < PostPhaseData.size(); ++i)
        // {
        //     float x = i * xStep;
        //     float y = height / 2 - (PostPhaseData[i] * (height / 2)); // Skalierung
        //     PostPhasePath.lineTo(x, y);
        // }

        // g.strokePath(PostPhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen

        g.setColour(juce::Colours::orange); // Linienfarbe blau
        g.setOpacity(0.5f);
    
        if (PostPhaseData.empty()) return;
    
        width = static_cast<float>(getWidth());
        height = static_cast<float>(getHeight());
        N = PostPhaseData.size();
    
        juce::Path PostPhasePath;
    
        // Logarithmische Skalierung basierend auf Indizes
        logMin = std::log10(1);
        logMax = std::log10(N);
    
        // Startpunkt setzen
        xStart = (std::log10(1) - logMin) / (logMax - logMin) * width;
        yStart = height / 2 - (PostPhaseData[0] * (height / 2));
        PostPhasePath.startNewSubPath(xStart, yStart);
    
        for (size_t i = 1; i < N; ++i)
        {
            float logIndex = std::log10(i + 1);
            float normX = (logIndex - logMin) / (logMax - logMin);
            float x = normX * width; // Logarithmische Skalierung der x-Koordinate
    
            float y = height / 2 - (PostPhaseData[i] * (height / 2)); // Phase skalieren
    
            PostPhasePath.lineTo(x, y);
        }
    
        g.strokePath(PostPhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen
    }

private:
    std::vector<float> PrePhaseData;
    std::vector<float> PostPhaseData;
};