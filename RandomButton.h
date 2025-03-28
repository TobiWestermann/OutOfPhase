#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "resources/images/dice_bin.h"

class RandomButton : public juce::Button, private juce::Timer
{
public:
    RandomButton() : juce::Button("RandomButton"), 
                    colorPhase(0.0f),
                    shakeOffset(0.0f, 0.0f),
                    shakePhase(0.0f)
    {
        setClickingTogglesState(true);
        setTooltip("Randomizes all phase components using either uniform or Gaussian distribution.");
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        auto bounds = getLocalBounds().toFloat();

        if (isActive) {
            // Animated gradient with cycling colors
            float hue1 = fmodf(colorPhase, 1.0f);
            float hue2 = fmodf(colorPhase + 0.33f, 1.0f);
            float hue3 = fmodf(colorPhase + 0.66f, 1.0f);
            
            juce::Colour color1 = juce::Colour::fromHSV(hue1, 0.8f, 0.9f, 1.0f);
            juce::Colour color2 = juce::Colour::fromHSV(hue2, 0.8f, 0.9f, 1.0f);
            juce::Colour color3 = juce::Colour::fromHSV(hue3, 0.8f, 0.9f, 1.0f);
            
            juce::ColourGradient gradient(
                color1,
                bounds.getTopLeft(),
                color2,
                bounds.getBottomRight(),
                false
            );
            gradient.addColour(0.5, color3);
            
            g.setGradientFill(gradient);
        } else {
            // Static gradient for inactive state
            juce::ColourGradient gradient(
                juce::Colours::blue,
                bounds.getTopLeft(),
                juce::Colours::green,
                bounds.getBottomRight(),
                false
            );
            gradient.addColour(0.5, juce::Colours::orange);
            
            g.setGradientFill(gradient);
        }
        
        g.fillRoundedRectangle(bounds, 10.0f);
        
        float scaleFactor = isActive ? 0.85f : 0.7f;
        
        auto imageBounds = m_diceImage.getBounds().toFloat();
        imageBounds = imageBounds.withSizeKeepingCentre(
            juce::jmin(bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor),
            juce::jmin(bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor)
        );
        
        juce::Point<float> center = bounds.getCentre();
        if (isActive) {
            center.x += shakeOffset.x;
            center.y += shakeOffset.y;
        }
        
        imageBounds = imageBounds.withCentre(center);
        g.drawImage(m_diceImage, imageBounds);

        // Mouse over highlight effect
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
    
    void clicked() override
    {
        Button::clicked();
        
        if (getToggleState()) {
            startTimerHz(30);
            shakePhase = 0.0f;
        } else {
            stopTimer();
            colorPhase = 0.0f;
            shakeOffset.x = 0.0f;
            shakeOffset.y = 0.0f;
            repaint();
        }
    }
    
private:
    juce::Image m_diceImage = juce::ImageFileFormat::loadFrom(dice_bin, dice_bin_len);
    float colorPhase;
    juce::Point<float> shakeOffset;
    float shakePhase;
    
    void timerCallback() override
    {
        if (getToggleState()) {
            colorPhase += 0.01f;
            
            if (colorPhase > 1.0f) {
                colorPhase -= 1.0f;
            }
            
            shakePhase += 0.2f;
            
            float amplitude = 2.0f;
            
            shakeOffset.x = amplitude * (sinf(shakePhase * 2.3f) + cosf(shakePhase * 1.7f) * 0.5f);
            shakeOffset.y = amplitude * (sinf(shakePhase * 1.9f) + cosf(shakePhase * 2.1f) * 0.5f);
            
            repaint();
        }
    }
};