#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FlipButton : public juce::Button, private juce::Timer
{
public:
    FlipButton() : juce::Button("FlipButton"), 
                   rotationAngle(0.0f), 
                   targetAngle(0.0f), 
                   wavePosition(-1.0f),
                   waveMovingLeftToRight(true)
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

        juce::Colour Color1 = juce::Colours::yellow;
        juce::Colour Color2 = juce::Colours::red;

        // Draw background colors
        if (isActive)
        {
            // Draw the yellow upper half with rounded top corners
            juce::Path upperPath;
            upperPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
            upperPath.lineTo(bounds.getX(), bounds.getY() + cornerSize);
            upperPath.quadraticTo(bounds.getX(), bounds.getY(), bounds.getX() + cornerSize, bounds.getY());
            upperPath.lineTo(bounds.getRight() - cornerSize, bounds.getY());
            upperPath.quadraticTo(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + cornerSize);
            upperPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
            upperPath.closeSubPath();
            g.setColour(Color1);
            g.fillPath(upperPath);

            // Draw the red bottom half with rounded bottom corners
            juce::Path lowerPath;
            lowerPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
            lowerPath.lineTo(bounds.getX(), bounds.getBottom() - cornerSize);
            lowerPath.quadraticTo(bounds.getX(), bounds.getBottom(), bounds.getX() + cornerSize, bounds.getBottom());
            lowerPath.lineTo(bounds.getRight() - cornerSize, bounds.getBottom());
            lowerPath.quadraticTo(bounds.getRight(), bounds.getBottom(), bounds.getRight(), bounds.getBottom() - cornerSize);
            lowerPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
            lowerPath.closeSubPath();
            g.setColour(Color2);
            g.fillPath(lowerPath);
        }
        else
        {
            // Draw the red upper half with rounded top corners
            juce::Path upperPath;
            upperPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
            upperPath.lineTo(bounds.getX(), bounds.getY() + cornerSize);
            upperPath.quadraticTo(bounds.getX(), bounds.getY(), bounds.getX() + cornerSize, bounds.getY());
            upperPath.lineTo(bounds.getRight() - cornerSize, bounds.getY());
            upperPath.quadraticTo(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + cornerSize);
            upperPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
            upperPath.closeSubPath();
            g.setColour(Color2);
            g.fillPath(upperPath);

            // Draw the yellow bottom half with rounded bottom corners
            juce::Path lowerPath;
            lowerPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
            lowerPath.lineTo(bounds.getX(), bounds.getBottom() - cornerSize);
            lowerPath.quadraticTo(bounds.getX(), bounds.getBottom(), bounds.getX() + cornerSize, bounds.getBottom());
            lowerPath.lineTo(bounds.getRight() - cornerSize, bounds.getBottom());
            lowerPath.quadraticTo(bounds.getRight(), bounds.getBottom(), bounds.getRight(), bounds.getBottom() - cornerSize);
            lowerPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
            lowerPath.closeSubPath();
            g.setColour(Color1);
            g.fillPath(lowerPath);
        }

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

        juce::Font textFont(20.0f);
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

        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.f));
        g.fillRoundedRectangle(bounds, 10.0f);
    
        if (isActive) {

            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(outlineBounds, 10.0f, 1.5f);
            
            g.setColour(juce::Colours::grey);
            g.setOpacity(0.2f);
            g.fillRoundedRectangle(bounds, 10.0f);
            g.resetToDefaultState();
        }
    }

    void clicked() override
    {
        bool newState = !getToggleState();
        Button::clicked();
        
        targetAngle = getToggleState() ? juce::MathConstants<float>::pi : 0.0f;
        wavePosition = 0.0f;
        waveMovingLeftToRight = !newState;
        startTimerHz(60);
    }
    
private:
    float rotationAngle;
    float targetAngle;
    float wavePosition;
    bool waveMovingLeftToRight; // Controls wave direction
    
    void timerCallback() override
    {
        const float rotationSpeed = 0.15f;
        const float waveSpeed = 0.05f;
        
        float angleDiff = targetAngle - rotationAngle;
        rotationAngle += angleDiff * rotationSpeed;
        
        wavePosition += waveSpeed;
        
        if (std::abs(angleDiff) < 0.01f && wavePosition > 1.0f) {
            rotationAngle = targetAngle;
            wavePosition = -1.0f;
            stopTimer();
        }
        
        repaint();
    }
};