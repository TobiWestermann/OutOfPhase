#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class BandModeButton : public juce::ToggleButton
{
public:
    BandModeButton()
    {
        setClickingTogglesState(true);
        setTooltip("Apply effect only to a specific frequency band when enabled");
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        float cornerSize = 10.0f;
        
        // Draw background
        juce::Colour baseColor = getToggleState() ? juce::Colours::cadetblue.brighter(0.1f) : juce::Colours::darkgrey.brighter(0.3f);
        g.setGradientFill(juce::ColourGradient(
            baseColor.brighter(0.1f), 
            bounds.getX(), bounds.getY(),
            baseColor.darker(0.2f), 
            bounds.getX(), bounds.getBottom(),
            false));
        
        g.fillRoundedRectangle(bounds, cornerSize);
        
        // Draw border
        g.setColour(getToggleState() ? juce::Colours::cadetblue.darker(0.5f) : juce::Colours::darkgrey.darker(0.1f));
        g.drawRoundedRectangle(bounds, cornerSize, 1.5f);
        
        // Draw text
        g.setFont(juce::Font(bounds.getHeight() * 0.5f).boldened());
        g.setColour(juce::Colours::white);
        
        juce::String text = "BAND MODE";
        g.drawText(text, bounds, juce::Justification::centred);

        // Draw indicator light
        float lightSize = bounds.getHeight() * 0.2f;
        juce::Rectangle<float> light(
            bounds.getRight() - lightSize - 10.0f,
            bounds.getCentreY() - lightSize / 2.0f,
            lightSize,
            lightSize
        );
        
        g.setColour(getToggleState() ? juce::Colours::lime.withAlpha(0.9f) : juce::Colours::darkred.withAlpha(0.7f));
        g.fillEllipse(light);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawEllipse(light, 1.0f);
    }
};