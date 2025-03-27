#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "resources/images/snowflake_bin.h"

class FrostButton : public juce::Button
{
public:
    FrostButton() : juce::Button("FrostButton")
    {
        setClickingTogglesState(true);
        setTooltip("Captures and maintains the current phase spectrum.");
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        juce::Colour bgColour = isActive ? juce::Colours::lightgrey.brighter(2) : juce::Colours::white;
        
        auto bounds = getLocalBounds().toFloat();

        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 10.0f);
        
        auto imageBounds = m_snowflakeImage.getBounds().toFloat();
        imageBounds = imageBounds.withSizeKeepingCentre(
            juce::jmin(bounds.getWidth() * 0.7f, bounds.getHeight() * 0.7f),
            juce::jmin(bounds.getWidth() * 0.7f, bounds.getHeight() * 0.7f)
        );
        imageBounds = imageBounds.withCentre(bounds.getCentre());
        g.drawImage(m_snowflakeImage, imageBounds);

        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.f));
        g.fillRoundedRectangle(bounds, 10.0f);

        if (isActive)
        {
            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(outlineBounds, 10.0f, 1.5f);
        }
    }
private:
    juce::Image m_snowflakeImage = juce::ImageFileFormat::loadFrom(snowflake_bin, snowflake_bin_len);
};
