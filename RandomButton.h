#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "resources/images/dice_bin.h"

class RandomButton : public juce::Button
{
public:
    RandomButton() : juce::Button("RandomButton")
    {
        setClickingTogglesState(true);
        setTooltip("Randomizes all phase components using either uniform or Gaussian distribution.");
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient gradient(
            isActive ? juce::Colours::red : juce::Colours::blue,
            bounds.getTopLeft(),
            isActive ? juce::Colours::yellow : juce::Colours::green,
            bounds.getBottomRight(),
            false
        );
        gradient.addColour(0.5, isActive ? juce::Colours::purple : juce::Colours::orange);

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 10.0f);
        
        float scaleFactor = isActive ? 0.85f : 0.7f;
        
        auto imageBounds = m_diceImage.getBounds().toFloat();
        imageBounds = imageBounds.withSizeKeepingCentre(
            juce::jmin(bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor),
            juce::jmin(bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor)
        );
        imageBounds = imageBounds.withCentre(bounds.getCentre());
        g.drawImage(m_diceImage, imageBounds);

        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.f));
        g.fillRoundedRectangle(bounds, 10.0f);

        if (isActive)
        {
            g.setColour(juce::Colours::grey);
            g.setOpacity(0.2f);
            g.fillRoundedRectangle(bounds, 10.0f);
            g.resetToDefaultState();
        }
        
        if (isActive)
        {
            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(outlineBounds, 10.0f, 1.5f);
        }
    }
private:
    juce::Image m_diceImage = juce::ImageFileFormat::loadFrom(dice_bin, dice_bin_len);
};