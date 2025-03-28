#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "resources/images/snowflake_bin.h"

class FrostButton : public juce::Button, private juce::Timer
{
public:
    FrostButton() : juce::Button("FrostButton"), rotationAngle(0.0f)
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
        
        float scaleFactor = isActive ? 0.85f : 0.7f;
        auto imageBounds = m_snowflakeImage.getBounds().toFloat();
        imageBounds = imageBounds.withSizeKeepingCentre(
            juce::jmin(bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor),
            juce::jmin(bounds.getWidth() * scaleFactor, bounds.getHeight() * scaleFactor)
        );
        
        juce::Point<float> center = bounds.getCentre();
        imageBounds = imageBounds.withCentre(center);
        
        if (isActive) {
            juce::Graphics::ScopedSaveState saveState(g);
            
            g.addTransform(juce::AffineTransform::rotation(
                rotationAngle, 
                center.getX(), 
                center.getY()
            ));
            
            g.drawImage(m_snowflakeImage, imageBounds);
        } else {
            g.drawImage(m_snowflakeImage, imageBounds);
        }

        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.f));
        g.fillRoundedRectangle(bounds, 10.0f);

        if (isActive) {
            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(outlineBounds, 10.0f, 1.5f);
        }
    }
    
void clicked() override
{
    Button::clicked();
    
    if (getToggleState()) {
        startTimerHz(60);
    } else {

        stopTimer();
        rotationAngle = 0.0f;
        repaint();
    }
}
    
private:
    juce::Image m_snowflakeImage = juce::ImageFileFormat::loadFrom(snowflake_bin, snowflake_bin_len);
    float rotationAngle;
    
    // Timer callback to update rotation
    void timerCallback() override
    {
        rotationAngle += 0.05f; // radians per frame -> animation speed
        
        if (rotationAngle > juce::MathConstants<float>::twoPi) {
            rotationAngle -= juce::MathConstants<float>::twoPi;
        }

        repaint();
    }
};