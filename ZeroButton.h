#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ZeroButton : public juce::Button
{
public:
    ZeroButton() : juce::Button("ZeroButton")
    {
        setClickingTogglesState(true);
        setTooltip("Sets all phase components to zero.");
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();

        juce::ColourGradient gradient(
            isActive ? juce::Colours::lightgrey : juce::Colours::white,
            center.x, center.y,
            juce::Colours::darkturquoise,
            bounds.getRight(), bounds.getBottom(),
            true
        );

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 10.0f);

        g.setFont(juce::Font(20.0f));
        g.setColour(juce::Colours::grey);
        g.drawText("0", bounds, juce::Justification::centred);

        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.f));
        g.fillRoundedRectangle(bounds, 10.0f);

        if (isActive)
        {
            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(outlineBounds, 10.0f, 1.5f);
        }
    }
};
