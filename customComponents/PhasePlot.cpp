#include "PhasePlot.h"
#include "../resources/images/glass_texture2_bin.h"

PhasePlot::PhasePlot() 
{
    m_glassImage = juce::ImageFileFormat::loadFrom(glass_texture2_bin, glass_texture2_bin_len);
}

void PhasePlot::setPrePhaseData(const std::vector<float>& newPrePhaseData)
{
    PrePhaseData = newPrePhaseData;
    repaint();
}

void PhasePlot::setPostPhaseData(const std::vector<float>& newPostPhaseData)
{
    PostPhaseData = newPostPhaseData;
    repaint();
}

void PhasePlot::paint(juce::Graphics& g)
{
    bool mouseOver = isMouseOver();

    g.fillAll(juce::Colours::whitesmoke.darker(0.2f));

    g.setOpacity(1.0f);

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 3);

    if (mouseOver)
    {
        juce::Rectangle<int> plotBounds = getLocalBounds().reduced(55, 0);
        float centerY = plotBounds.getY() + plotBounds.getHeight() / 2.0f;
        g.setColour(juce::Colours::grey);
        g.drawLine(static_cast<float>(plotBounds.getX()), centerY, 
                static_cast<float>(plotBounds.getRight()), centerY, 
                1.0f);
    } else {
        juce::Rectangle<int> plotBounds = getLocalBounds();
        float centerY = plotBounds.getY() + plotBounds.getHeight() / 2.0f;
        g.setColour(juce::Colours::grey);
        g.drawLine(static_cast<float>(plotBounds.getX()), centerY, 
                static_cast<float>(plotBounds.getRight()), centerY, 
                1.0f);
    }

    if (mouseOver)
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions("Arial", 15.0f, juce::Font::plain)));
        g.drawText(juce::CharPointer_UTF8("\xCF\x80"), 5, 5, 20, 20, juce::Justification::centred);
        g.drawText(juce::CharPointer_UTF8("-\xCF\x80"), 5, getHeight() - 25, 20, 20, juce::Justification::centred);
    }

    juce::Rectangle<int> clipBounds = getLocalBounds().reduced(5);
    g.reduceClipRegion(clipBounds);

    g.setColour(juce::Colours::white);
    g.setOpacity(0.5f);

    if (!PrePhaseData.empty())
    {
        auto width = static_cast<float>(clipBounds.getWidth());
        auto height = static_cast<float>(clipBounds.getHeight());
        size_t N = PrePhaseData.size();

        juce::Path PrePhasePath;

        float logMin = static_cast<float>(std::log10(1.0f));
        float logMax = static_cast<float>(std::log10(static_cast<float>(N)));

        float margin;
        if (mouseOver) {
            juce::Font labelFont(juce::FontOptions(15.0f));
            juce::GlyphArrangement glyphArrangement;
            glyphArrangement.addLineOfText(labelFont, "0", 0.0f, 0.0f);
            float zeroWidth = glyphArrangement.getBoundingBox(0, 1, true).getWidth();
            glyphArrangement.clear();
            
            juce::String nyquistLabel;
            if (m_sampleRate >= 2000) {
                nyquistLabel = juce::String(m_sampleRate / 2000.0f, 1) + " kHz";
            } else {
                nyquistLabel = juce::String(m_sampleRate / 2) + " Hz";
            }
            
            glyphArrangement.addLineOfText(labelFont, nyquistLabel, 0.0f, 0.0f);
            float fsWidth = glyphArrangement.getBoundingBox(0, nyquistLabel.length(), true).getWidth() - 10.0f;
            margin = std::max(zeroWidth, fsWidth) + 5.0f;
        }
        else {
            margin = 0.0f;
        }
        float plotWidth = width - 2 * margin;

        float xStart = clipBounds.getX() + margin + (static_cast<float>(std::log10(1.0f)) - logMin) / (logMax - logMin) * plotWidth;
        float yStart = clipBounds.getY() + height / 2 - (PrePhaseData[0] * (height / 2));
        PrePhasePath.startNewSubPath(xStart, yStart);

        for (size_t i = 1; i < N; ++i)
        {
            float logIndex = static_cast<float>(std::log10(i + 1));
            float normX = (logIndex - logMin) / (logMax - logMin);
            float x = clipBounds.getX() + margin + normX * plotWidth;
            float y = clipBounds.getY() + height / 2 - (PrePhaseData[i] * (height / 2));

            PrePhasePath.lineTo(x, y);
        }

        g.strokePath(PrePhasePath, juce::PathStrokeType(1.0f));
    }

    g.setColour(juce::Colours::white);
    g.setOpacity(0.5f);

    if (!PostPhaseData.empty())
    {
        size_t N = PostPhaseData.size();
        juce::Path PostPhasePath;

        float logMin = static_cast<float>(std::log10(1.0f));
        float logMax = static_cast<float>(std::log10(static_cast<float>(N)));

        float margin;
        if (mouseOver) {
            juce::Font labelFont(juce::FontOptions(15.0f));
            juce::GlyphArrangement glyphArrangement;
            glyphArrangement.addLineOfText(labelFont, "0", 0.0f, 0.0f);
            float zeroWidth = glyphArrangement.getBoundingBox(0, 1, true).getWidth();
            glyphArrangement.clear();
            
            juce::String nyquistLabel;
            if (m_sampleRate >= 2000) {
                nyquistLabel = juce::String(m_sampleRate / 2000.0f, 1) + " kHz";
            } else {
                nyquistLabel = juce::String(m_sampleRate / 2) + " Hz";
            }
            
            glyphArrangement.addLineOfText(labelFont, nyquistLabel, 0.0f, 0.0f);
            float fsWidth = glyphArrangement.getBoundingBox(0, nyquistLabel.length(), true).getWidth() - 10.0f;
            margin = std::max(zeroWidth, fsWidth) + 5.0f;
        }
        else {
            margin = 0.0f;
        }
        float plotWidth = clipBounds.getWidth() - 2 * margin;

        float xStart = clipBounds.getX() + margin + (static_cast<float>(std::log10(1)) - logMin) / (logMax - logMin) * plotWidth;
        float yStart = clipBounds.getY() + clipBounds.getHeight() / 2 - (PostPhaseData[0] * (clipBounds.getHeight() / 2));
        PostPhasePath.startNewSubPath(xStart, yStart);

        for (size_t i = 1; i < N; ++i)
        {
            float logIndex = static_cast<float>(std::log10(i + 1));
            float normX = (logIndex - logMin) / (logMax - logMin);
            float x = clipBounds.getX() + margin + normX * plotWidth;
            float y = clipBounds.getY() + clipBounds.getHeight() / 2 - (PostPhaseData[i] * (clipBounds.getHeight() / 2));

            PostPhasePath.lineTo(x, y);
        }

        g.strokePath(PostPhasePath, juce::PathStrokeType(1.0f));
    }
    
    if (mouseOver)
    {
        g.setColour(juce::Colours::white);
        g.setFont(15.0f);

        g.drawText("0", 5, getHeight() / 2 - 10, 20, 20, juce::Justification::centred);

        g.setFont(12.0f);
        
        int nyquistFreq = static_cast<int>(m_sampleRate / 2);
        
        juce::String freqText;
        if (nyquistFreq >= 1000)
        {
            float nyquistKHz = nyquistFreq / 1000.0f;
            freqText = juce::String(nyquistKHz, 1) + " kHz";
        }
        else
        {
            freqText = juce::String(nyquistFreq) + " Hz";
        }
        
        juce::Font labelFont(juce::FontOptions(12.0f));
        juce::GlyphArrangement glyphArrangement;
        glyphArrangement.addLineOfText(labelFont, freqText, 0.0f, 0.0f);
        float textWidth = glyphArrangement.getBoundingBox(0, freqText.length(), true).getWidth();
        
        g.drawText(freqText, 
                  getWidth() - static_cast<int>(textWidth) - 10, 
                  getHeight() / 2 - 10, 
                  static_cast<int>(textWidth) + 5, 
                  20, 
                  juce::Justification::centred);
    }
}