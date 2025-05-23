#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Button Captures current phase
class FreezeCaptureButton : public juce::TextButton, public juce::Timer
{
public:
    FreezeCaptureButton() : flashAlpha(0.0f), isFlashing(false)
    {
        setButtonText("Capture");
        
        setClickingTogglesState(false);
        
        // Set up button colors
        setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
        
        setTooltip("Click to capture the current phase spectrum");
        
        startTimerHz(60);
    }
    
    ~FreezeCaptureButton() override 
    {
        stopTimer();
    }
    
    // Initiates the flash animation
    void triggerFlash()
    {
        flashAlpha = 1.0f;
        isFlashing = true;
    }
    
    // Custom button rendering
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        
        juce::ColourGradient gradient;
        
        // Different gradient for pressed state
        if (shouldDrawButtonAsDown)
        {
            gradient = juce::ColourGradient(
                juce::Colour(0xFF6BCBFF).withAlpha(0.9f),
                bounds.getX(), bounds.getY(),
                juce::Colour(0xFF1E88E5).withAlpha(0.9f),
                bounds.getRight(), bounds.getBottom(),
                false
            );
        }
        else
        {
            gradient = juce::ColourGradient(
                juce::Colour(0xFFADE1FF).withAlpha(0.7f),
                bounds.getX(), bounds.getY(),
                juce::Colour(0xFF4EADFF).withAlpha(0.7f),
                bounds.getRight(), bounds.getBottom(),
                false
            );
        }
        
        // Draw the button
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 5.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
        
        if (shouldDrawButtonAsHighlighted && !shouldDrawButtonAsDown)
        {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, 5.0f);
        }
        
        // Draw button text
        g.setColour(juce::Colours::white);
        g.setFont(getHeight() * 0.6f);
        g.drawText(getButtonText(), getLocalBounds(), juce::Justification::centred);
        
        // Draw flash overlay if active
        if (isFlashing)
        {
            g.setColour(juce::Colours::white.withAlpha(flashAlpha));
            g.fillRoundedRectangle(bounds, 5.0f);
        }
    }
    
    // Handles flash animation
    void timerCallback() override
    {
        if (isFlashing)
        {
            flashAlpha -= 0.1f;
            
            if (flashAlpha <= 0.0f)
            {
                flashAlpha = 0.0f;
                isFlashing = false;
            }
            
            repaint();
        }
    }
    
private:
    float flashAlpha;
    bool isFlashing;
};