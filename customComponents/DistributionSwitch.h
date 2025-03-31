#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Custom toggle switch between Uniform and Gaussian distributions
class DistributionSwitch : public juce::Button
{
public:
    DistributionSwitch() : juce::Button("DistributionSwitch")
    {
        setClickingTogglesState(true);
        setTooltip("Toggle between Uniform (U) and Gaussian (G) distributions for random phase");
    }
    
    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        float cornerRadius = bounds.getHeight() / 2.0f;
        
        // Define colors for each state
        juce::Colour uniformColor = juce::Colour(58, 134, 255);
        juce::Colour gaussianColor = juce::Colour(245, 171, 53);
        
        juce::Colour gradientStartColor, gradientEndColor;
        
        // Set gradient colors based on toggle state
        if (getToggleState()) {
            gradientStartColor = gaussianColor.brighter(0.2f);
            gradientEndColor = gaussianColor.darker(0.3f);
        } else {
            gradientStartColor = uniformColor.brighter(0.2f);
            gradientEndColor = uniformColor.darker(0.3f);
        }
        
        // Draw switch background
        juce::ColourGradient gradient(
            gradientStartColor,
            bounds.getX(), bounds.getCentreY(),
            gradientEndColor,
            bounds.getRight(), bounds.getCentreY(),
            false
        );
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, cornerRadius);
        
        // Draw thumb (sliding circle)
        float thumbWidth = bounds.getHeight() * 0.9f;
        float thumbX = getToggleState() ? bounds.getRight() - thumbWidth - 2.0f : bounds.getX() + 2.0f;
        
        juce::ColourGradient thumbGradient(
            juce::Colours::white,
            thumbX, bounds.getY(),
            juce::Colours::lightgrey,
            thumbX, bounds.getBottom(),
            false
        );
        g.setGradientFill(thumbGradient);
        g.fillEllipse(thumbX, bounds.getY() + 2.0f, thumbWidth, bounds.getHeight() - 4.0f);
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.drawEllipse(thumbX, bounds.getY() + 2.0f, thumbWidth, bounds.getHeight() - 4.0f, 1.0f);
        
        // Draw U and G labels
        float leftLetterX = bounds.getX() + bounds.getHeight() / 2.0f;
        float letterY = bounds.getCentreY();
        float rightLetterX = bounds.getRight() - bounds.getHeight() / 2.0f;
        
        g.setFont(juce::Font(juce::FontOptions((bounds.getHeight() * 0.5f), juce::Font::bold)));
        
        // Draw U (Uniform) label
        g.setColour(getToggleState() ? 
                  uniformColor.darker(0.5f).withAlpha(0.5f) : 
                  juce::Colours::black.withAlpha(0.8f));
                  
        g.drawText("U", 
                  static_cast<int>(leftLetterX - bounds.getHeight() * 0.25f),
                  static_cast<int>(letterY - bounds.getHeight() * 0.25f),
                  static_cast<int>(bounds.getHeight() * 0.5f), 
                  static_cast<int>(bounds.getHeight() * 0.5f), 
                  juce::Justification::centred);
        
        // Draw G (Gaussian) label
        g.setColour(getToggleState() ? 
                  juce::Colours::black.withAlpha(0.8f) : 
                  gaussianColor.darker(0.5f).withAlpha(0.5f));
                  
        g.drawText("G", 
                  static_cast<int>(rightLetterX - bounds.getHeight() * 0.25f),
                  static_cast<int>(letterY - bounds.getHeight() * 0.25f),
                  static_cast<int>(bounds.getHeight() * 0.5f), 
                  static_cast<int>(bounds.getHeight() * 0.5f), 
                  juce::Justification::centred);
        
        // Highlight on hover
        if (isMouseOverButton) {
            g.setColour(juce::Colours::white.withAlpha(0.15f));
            g.fillRoundedRectangle(bounds, cornerRadius);
        }
    }
};