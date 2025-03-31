#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FlipButton : public juce::Button, private juce::Timer
{
public:
    // Initialize button state and animations
    FlipButton() : juce::Button("FlipButton"), 
                   rotationAngle(0.0f), 
                   targetAngle(0.0f), 
                   wavePosition(-1.0f),
                   waveMovingLeftToRight(true),
                   colorTransition(1.0f)
    {
        setClickingTogglesState(true);
        setTooltip("Inverts the phase of all components (multiplies by -1).");
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        auto bounds = getLocalBounds().toFloat();
        auto halfHeight = bounds.getHeight() / 2.0f;
        float cornerSize = 13.0f;

        juce::Colour yellowColor = juce::Colours::yellow.withMultipliedBrightness(1.05f);
        juce::Colour redColor = juce::Colours::red.darker(0.1f);
        
        juce::Colour topColor, bottomColor;
        
        // Set colors based on button state
        if (isActive) {
            topColor = redColor.interpolatedWith(yellowColor, colorTransition);
            bottomColor = yellowColor.interpolatedWith(redColor, colorTransition);
        } else {
            topColor = yellowColor.interpolatedWith(redColor, colorTransition);
            bottomColor = redColor.interpolatedWith(yellowColor, colorTransition);
        }

        // Upper half with rounded corners
        juce::Path upperPath;
        upperPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
        upperPath.lineTo(bounds.getX(), bounds.getY() + cornerSize);
        upperPath.quadraticTo(bounds.getX(), bounds.getY(), bounds.getX() + cornerSize, bounds.getY());
        upperPath.lineTo(bounds.getRight() - cornerSize, bounds.getY());
        upperPath.quadraticTo(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + cornerSize);
        upperPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
        upperPath.closeSubPath();

        g.setGradientFill(juce::ColourGradient(
            topColor.brighter(0.05f),
            bounds.getX(), bounds.getY(),
            topColor.darker(0.1f),
            bounds.getX(), bounds.getCentreY(), 
            false
        ));
        g.fillPath(upperPath);

        // Lower half with rounded corners
        juce::Path lowerPath;
        lowerPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
        lowerPath.lineTo(bounds.getX(), bounds.getBottom() - cornerSize);
        lowerPath.quadraticTo(bounds.getX(), bounds.getBottom(), bounds.getX() + cornerSize, bounds.getBottom());
        lowerPath.lineTo(bounds.getRight() - cornerSize, bounds.getBottom());
        lowerPath.quadraticTo(bounds.getRight(), bounds.getBottom(), bounds.getRight(), bounds.getBottom() - cornerSize);
        lowerPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
        lowerPath.closeSubPath();

        g.setGradientFill(juce::ColourGradient(
            bottomColor,
            bounds.getX(), bounds.getCentreY(),
            bottomColor.darker(0.15f),
            bounds.getX(), bounds.getBottom(),
            false
        ));
        g.fillPath(lowerPath);

        // Draw dividing line between halves
        float lineY = bounds.getBottomLeft().getY() - halfHeight;
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.drawLine(bounds.getX(), lineY, bounds.getRight(), lineY, 1.0f);

        // Draw animated wave effect when toggling
        if (wavePosition >= 0.0f && wavePosition <= 1.0f) {
            float effectivePosition = waveMovingLeftToRight ? wavePosition : (1.0f - wavePosition);
            float waveX = bounds.getX() + bounds.getWidth() * effectivePosition;
            float waveWidth = bounds.getWidth() * 0.2f;
            
            float gradientStart = waveX;
            float gradientEnd = waveMovingLeftToRight ? (waveX - waveWidth/2) : (waveX + waveWidth/2);
            
            juce::ColourGradient waveGradient(
                juce::Colours::white.withAlpha(0.6f), gradientStart, bounds.getY(),
                juce::Colours::white.withAlpha(0.0f), gradientEnd, bounds.getY(),
                false
            );
            waveGradient.addColour(0.8, juce::Colours::white.withAlpha(0.0f));
            
            g.setGradientFill(waveGradient);
            g.fillRect(waveX - waveWidth/2, bounds.getY(), waveWidth, bounds.getHeight());
        }

        // Draw button text with animation
        juce::Font textFont = juce::FontOptions(20.0f);
        textFont.setBold(true);
        juce::String buttonText("F L I P");
        juce::Colour textColor = juce::Colours::black;

        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();
        
        {
            juce::Graphics::ScopedSaveState savedState(g);
            
            g.addTransform(juce::AffineTransform::rotation(rotationAngle, centerX, centerY));
            
            g.setFont(textFont);
            g.setColour(textColor);
            g.drawText(buttonText, bounds, juce::Justification::centred, false);
        }

        // Highlight on hover
        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.0f));
        g.fillRoundedRectangle(bounds, cornerSize);
    
        // Outline when active
        if (isActive) {
            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(outlineBounds, cornerSize, 1.5f);
        }
    }

    // Start animations when clicked
    void clicked() override
    {
        bool newState = !getToggleState();
        Button::clicked();
        
        targetAngle = getToggleState() ? juce::MathConstants<float>::pi : 0.0f;
        wavePosition = 0.0f;
        waveMovingLeftToRight = !newState;
        colorTransition = 0.0f;
        startTimerHz(60);
    }
    
private:
    float rotationAngle;
    float targetAngle;
    float wavePosition;
    bool waveMovingLeftToRight;
    float colorTransition;
    
    // Update animations
    void timerCallback() override
    {
        const float rotationSpeed = 0.15f;
        const float waveSpeed = 0.05f;
        const float colorSpeed = 0.08f;
        
        float angleDiff = targetAngle - rotationAngle;
        rotationAngle += angleDiff * rotationSpeed;
        
        wavePosition += waveSpeed;
        
        colorTransition += colorSpeed;
        if (colorTransition > 1.0f) {
            colorTransition = 1.0f;
        }
        
        // Stop timer when all animations complete
        if (std::abs(angleDiff) < 0.01f && wavePosition > 1.0f && colorTransition >= 1.0f) {
            rotationAngle = targetAngle;
            wavePosition = -1.0f;
            stopTimer();
        }
        
        repaint();
    }
};