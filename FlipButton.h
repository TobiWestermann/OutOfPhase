#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FlipButton : public juce::Button
{
public:
    FlipButton() : juce::Button("FlipButton")
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
            Path upperPath;
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
            Path lowerPath;
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
            // Draw the yellow upper half with rounded top corners
            Path upperPath;
            upperPath.startNewSubPath(bounds.getX(), bounds.getBottomLeft().getY() - halfHeight);
            upperPath.lineTo(bounds.getX(), bounds.getY() + cornerSize);
            upperPath.quadraticTo(bounds.getX(), bounds.getY(), bounds.getX() + cornerSize, bounds.getY());
            upperPath.lineTo(bounds.getRight() - cornerSize, bounds.getY());
            upperPath.quadraticTo(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + cornerSize);
            upperPath.lineTo(bounds.getRight(), bounds.getBottomLeft().getY() - halfHeight);
            upperPath.closeSubPath();
            g.setColour(Color2);
            g.fillPath(upperPath);

            // Draw the red bottom half with rounded bottom corners
            Path lowerPath;
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

        juce::Font textFont(18.0f);
        juce::String buttonText("F L I P");

        juce::Colour textColor = juce::Colours::black;;

        if (!isActive) {
            g.setFont(textFont);
            g.setColour(textColor);
            g.drawText(buttonText, bounds, juce::Justification::centred, false);
        } else {
            g.setFont(textFont);
            g.setColour(textColor);
            
            float centerX = bounds.getCentreX();
            float centerY = bounds.getCentreY();
            
            juce::Graphics::ScopedSaveState savedState(g);
            g.addTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi, centerX, centerY));
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
};