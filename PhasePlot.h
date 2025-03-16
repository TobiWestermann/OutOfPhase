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
        bool mouseOver = isMouseOver();

        g.fillAll(juce::Colours::whitesmoke.darker(0.2)); // Hintergrund schwarz setzen

        g.setColour(juce::Colours::grey);
        g.drawRect(getLocalBounds(), 5); // Draw outline

        if (mouseOver)
        {
            g.setColour(juce::Colours::white);
            g.setFont(15.0f);
            g.drawText(juce::CharPointer_UTF8("\u03C0"), 5, 5, 20, 20, juce::Justification::centred);
            g.drawText(juce::CharPointer_UTF8("-\u03C0"), 5, getHeight() - 25, 20, 20, juce::Justification::centred);
        }

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

        // Define margins
        float margin = mouseOver ? 30.0f : 0.0f;
        float plotWidth = width - 2 * margin;

        // Startpunkt setzen
        float xStart = margin + (std::log10(1) - logMin) / (logMax - logMin) * plotWidth;
        float yStart = height / 2 - (PrePhaseData[0] * (height / 2));
        PrePhasePath.startNewSubPath(xStart, yStart);

        for (size_t i = 1; i < N; ++i)
        {
            float logIndex = std::log10(i + 1);
            float normX = (logIndex - logMin) / (logMax - logMin);
            float x = margin + normX * plotWidth; // Logarithmische Skalierung der x-Koordinate

            float y = height / 2 - (PrePhaseData[i] * (height / 2)); // Phase skalieren

            PrePhasePath.lineTo(x, y);
        }

        g.strokePath(PrePhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen

        // PostPhase

        g.setColour(juce::Colours::orange); // Linienfarbe orange
        g.setOpacity(0.5f);

        if (PostPhaseData.empty()) return;

        N = PostPhaseData.size();

        juce::Path PostPhasePath;

        // Logarithmische Skalierung basierend auf Indizes
        logMin = std::log10(1);
        logMax = std::log10(N);

        // Startpunkt setzen
        xStart = margin + (std::log10(1) - logMin) / (logMax - logMin) * plotWidth;
        yStart = height / 2 - (PostPhaseData[0] * (height / 2));
        PostPhasePath.startNewSubPath(xStart, yStart);

        for (size_t i = 1; i < N; ++i)
        {
            float logIndex = std::log10(i + 1);
            float normX = (logIndex - logMin) / (logMax - logMin);
            float x = margin + normX * plotWidth; // Logarithmische Skalierung der x-Koordinate

            float y = height / 2 - (PostPhaseData[i] * (height / 2)); // Phase skalieren

            PostPhasePath.lineTo(x, y);
        }

        g.strokePath(PostPhasePath, juce::PathStrokeType(2.0f)); // Linie zeichnen

        if (mouseOver)
        {
            g.setColour(juce::Colours::white);
            g.setFont(15.0f);
            g.drawText(juce::CharPointer_UTF8("0"), 5, getHeight()/2-10, 20, 20, juce::Justification::centred);
            g.drawText(juce::CharPointer_UTF8("fs"), getWidth()-25, getHeight()/2-10, 20, 20, juce::Justification::centred);
        }
        }
        
        private:
        std::vector<float> PrePhaseData;
    std::vector<float> PostPhaseData;
};