#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class BandModeButton : public juce::ToggleButton,
                       private juce::Timer
{
public:
    BandModeButton()
    {
        setClickingTogglesState(true);
        setTooltip("Apply effect only to a specific frequency band when enabled");
        startTimerHz(60);
    }

    ~BandModeButton() override
    {
        stopTimer();
    }

    // Handle animations and updates
    void timerCallback() override
    {
        bool updated = false;
        
        const float targetAlpha = getToggleState() ? 1.0f : 0.0f;
        const float animationSpeed = 0.12f;
        
        // Animate transition between on/off states
        if (std::abs(transitionAlpha - targetAlpha) > 0.01f)
        {
            transitionAlpha = transitionAlpha + (targetAlpha - transitionAlpha) * animationSpeed;
            updated = true;
        }
        
        // Create pulsing effect when activated
        if (getToggleState())
        {
            pulseAlpha = 0.3f + 0.2f * std::sin(pulsePhase);
            pulsePhase += 0.05f;
            updated = true;
        }
        else if (pulseAlpha > 0.0f)
        {
            pulseAlpha = juce::jmax(0.0f, pulseAlpha - 0.05f);
            updated = true;
        }
        
        if (updated)
            repaint();
    }

    // Custom rendering of the button
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        float cornerSize = 10.0f;
        
        // Outer glow effect
        if (pulseAlpha > 0.0f)
        {
            g.setColour(juce::Colours::cadetblue.withAlpha(pulseAlpha * 0.7f));
            g.fillRoundedRectangle(bounds.expanded(3.0f), cornerSize + 3.0f);
        }
        
        // Main button background
        juce::Colour offColor = juce::Colours::darkgrey.brighter(0.3f);
        juce::Colour onColor = juce::Colours::cadetblue.brighter(0.1f);
        juce::Colour baseColor = offColor.interpolatedWith(onColor, transitionAlpha);
        
        g.setGradientFill(juce::ColourGradient(
            baseColor.brighter(0.1f),
            bounds.getX(), bounds.getY(),
            baseColor.darker(0.3f),
            bounds.getRight(), bounds.getBottom(),
            false));
        
        g.fillRoundedRectangle(bounds, cornerSize);
        
        // Top shadow effect
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::black.withAlpha(0.2f),
            bounds.getX(), bounds.getY(),
            juce::Colours::transparentBlack,
            bounds.getX(), bounds.getY() + bounds.getHeight() * 0.15f,
            false));
        g.fillRoundedRectangle(bounds, cornerSize);
        
        // Bottom highlight effect
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::transparentWhite,
            bounds.getX(), bounds.getBottom() - bounds.getHeight() * 0.15f,
            juce::Colours::white.withAlpha(0.1f),
            bounds.getX(), bounds.getBottom(),
            false));
        g.fillRoundedRectangle(bounds, cornerSize);
        
        // Border
        juce::Colour offBorderColor = juce::Colours::darkgrey.darker(0.1f);
        juce::Colour onBorderColor = juce::Colours::cadetblue.darker(0.5f);
        g.setColour(offBorderColor.interpolatedWith(onBorderColor, transitionAlpha));
        g.drawRoundedRectangle(bounds, cornerSize, 1.5f);
        
        // Draw animated frequency band visualization
        if (transitionAlpha > 0.1f)
        {
            auto bandArea = bounds.reduced(bounds.getWidth() * 0.3f, bounds.getHeight() * 0.3f);
            bandArea.setX(bounds.getX() + bounds.getWidth() * 0.15f); 
            float barWidth = bandArea.getWidth() / 5.0f;
            float spacing = barWidth * 0.3f;
            
            g.setColour(juce::Colours::white.withAlpha(0.2f + 0.15f * transitionAlpha));
            
            for (int i = 0; i < 5; ++i)
            {
                float height = bandArea.getHeight() * (0.1f + 0.4f * (1.0f + std::sin(pulsePhase + i * 0.7f)));
                float x = bandArea.getX() + i * (barWidth + spacing);
                float y = bandArea.getCentreY() + (bandArea.getHeight() - height) * 0.5f;
                
                g.fillRoundedRectangle(x, y, barWidth, height, 2.0f);
            }
        }
        
        // Draw button text
        auto textBounds = bounds.reduced(4.0f);
        g.setFont(juce::Font(juce::FontOptions((bounds.getHeight() * 0.4f), juce::Font::bold)));
        
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawText("BAND MODE", textBounds.translated(1, 1), juce::Justification::centred);
        
        juce::Colour textColor = juce::Colours::white.withAlpha(0.9f);
        g.setColour(textColor);
        g.drawText("BAND MODE", textBounds, juce::Justification::centred);
        
        // Draw status indicator light
        float lightSize = bounds.getHeight() * 0.25f;
        juce::Rectangle<float> light(
            bounds.getRight() - lightSize - 10.0f,
            bounds.getCentreY() - lightSize / 2.0f,
            lightSize,
            lightSize
        );
        
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(light.expanded(1.5f));

        float lightAlpha = getToggleState() ? (0.7f + 0.3f * std::sin(pulsePhase * 2.0f)) : 0.7f;
        juce::Colour offLightColor = juce::Colours::darkred.withAlpha(lightAlpha);
        juce::Colour onLightColor = juce::Colours::lime.withAlpha(lightAlpha);
        g.setColour(offLightColor.interpolatedWith(onLightColor, transitionAlpha));
        g.fillEllipse(light);
        
        // Add glass effect to light
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::white.withAlpha(0.7f),
            light.getX(), light.getY(),
            juce::Colours::transparentWhite,
            light.getCentreX(), light.getCentreY(),
            true));
        g.fillEllipse(light.reduced(lightSize * 0.3f));
        
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawEllipse(light, 1.0f);
        
        // Highlight reflection
        if (transitionAlpha > 0.1f)
        {
            g.setColour(juce::Colours::white.withAlpha(0.4f * transitionAlpha));
            g.fillEllipse(light.getX() + lightSize * 0.25f, 
                          light.getY() + lightSize * 0.25f,
                          lightSize * 0.3f, lightSize * 0.3f);
        }
    }
    
private:
    float pulsePhase = 0.0f;    // Controls animation timing
    float pulseAlpha = 0.0f;    // Controls glow intensity
    float transitionAlpha = 0.0f;  // Controls on/off state transition
};