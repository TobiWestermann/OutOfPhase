#include "PhasePlot.h"
#include "resources/images/glass_texture2_bin.h"

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

    g.setOpacity(0.6f);
    g.drawImageWithin(m_glassImage, getX(), 0, getWidth(), getHeight(),
                      juce::RectanglePlacement::fillDestination);
    g.setOpacity(1.0f);

    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 3);

    if (mouseOver)
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font("Arial", 15.0f, juce::Font::plain));
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

        float logMin = static_cast<float>(std::log10(1));
        float logMax = static_cast<float>(std::log10(N));

        float margin = mouseOver ? 23.0f : 0.0f;
        float plotWidth = width - 2 * margin;

        float xStart = clipBounds.getX() + margin + (static_cast<float>(std::log10(1)) - logMin) / (logMax - logMin) * plotWidth;
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

        g.strokePath(PrePhasePath, juce::PathStrokeType(2.0f));
    }

    g.setColour(juce::Colours::white);
    g.setOpacity(0.5f);

    if (!PostPhaseData.empty())
    {
        size_t N = PostPhaseData.size();
        juce::Path PostPhasePath;

        float logMin = static_cast<float>(std::log10(1));
        float logMax = static_cast<float>(std::log10(N));

        float margin = mouseOver ? 23.0f : 0.0f;
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

        g.strokePath(PostPhasePath, juce::PathStrokeType(2.0f));
    }

    if (mouseOver)
    {
        g.setColour(juce::Colours::white);
        g.setFont(15.0f);
        g.drawText(juce::CharPointer_UTF8("0"), 5, getHeight() / 2 - 10, 20, 20, juce::Justification::centred);
        g.drawText(juce::CharPointer_UTF8("fs/2"), getWidth() - 25, getHeight() / 2 - 10, 20, 20, juce::Justification::centred);
    }
}