#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

// Custom button that displays animated waves when active
class ZeroButton : public juce::Button, private juce::Timer
{
public:
    ZeroButton() : juce::Button("ZeroButton")
    {
        setClickingTogglesState(true);
        setTooltip("Sets all phase components to zero.");
        
        // Initialize wave positions
        for (std::size_t i = 0; i < numWaves; ++i) {
            wavePositions[i] = i * (1.0f / numWaves);
        }
        
        startTimerHz(60);
    }

    ~ZeroButton() override
    {
        stopTimer();
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        bool isActive = getToggleState();
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        
        // Set base colors based on button state
        juce::Colour baseColor = isActive ? juce::Colours::dodgerblue.darker(0.2f) 
                                         : juce::Colours::lightgrey;
        
        // Draw button background with gradient
        if (isActive) {
            g.setGradientFill(juce::ColourGradient(
                baseColor.brighter(0.1f),
                bounds.getX(), bounds.getY(),
                baseColor.darker(0.2f),
                bounds.getRight(), bounds.getBottom(),
                false
            ));
        } else {
            g.setGradientFill(juce::ColourGradient(
                juce::Colours::white.brighter(0.1f),
                bounds.getX(), bounds.getY(),
                juce::Colours::lightgrey.darker(0.1f),
                bounds.getRight(), bounds.getBottom(),
                false
            ));
        }
        
        g.fillRoundedRectangle(bounds, 10.0f);
        
        // Add top shadow
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::black.withAlpha(0.2f),
            bounds.getX(), bounds.getY(),
            juce::Colours::transparentBlack,
            bounds.getX(), bounds.getY() + bounds.getHeight() * 0.15f,
            false
        ));
        g.fillRoundedRectangle(bounds, 10.0f);
        
        // Add bottom highlight
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::transparentWhite,
            bounds.getX(), bounds.getBottom() - bounds.getHeight() * 0.15f,
            juce::Colours::white.withAlpha(0.1f),
            bounds.getX(), bounds.getBottom(),
            false
        ));
        g.fillRoundedRectangle(bounds, 10.0f);
        
        if (isActive) {
            // Draw center glow
            float centerGlowSize = bounds.getWidth() * 0.25f;
            g.setGradientFill(juce::ColourGradient(
                juce::Colours::white.withAlpha(0.7f),
                center.x, center.y,
                juce::Colours::white.withAlpha(0.0f),
                center.x + centerGlowSize, center.y,
                true
            ));
            g.fillEllipse(center.x - centerGlowSize, center.y - centerGlowSize, 
                          centerGlowSize * 2.0f, centerGlowSize * 2.0f);
            
            // Draw animated expanding waves
            const float maxRadius = bounds.getWidth() * 0.7f;
            for (std::size_t i = 0; i < numWaves; ++i) {
                float position = wavePositions[i];
                float radius = position * maxRadius;
                
                float alpha = 0.7f * (1.0f - position);
                juce::Colour waveColor = juce::Colours::dodgerblue.withAlpha(alpha);
                
                g.setColour(waveColor);
                float thickness = 3.0f + 2.0f * (1.0f - position);
                
                g.drawEllipse(
                    center.x - radius, 
                    center.y - radius, 
                    radius * 2.0f, 
                    radius * 2.0f,
                    thickness
                );
            }
        } else {
            // Draw inactive state with pulsing circles
            float baseInnerRadius = bounds.getWidth() * 0.15f;
            float baseOuterRadius = bounds.getWidth() * 0.25f;
            
            float pulse = 0.5f + 0.5f * std::sin(inactivePhase * 0.5f);
            
            float innerRadius = baseInnerRadius * (1.0f + 0.1f * pulse);
            float outerRadius = baseOuterRadius * (1.0f + 0.1f * pulse);
            
            float innerAlpha = 0.2f + 0.5f * pulse;
            float outerAlpha = 0.1f + 0.4f * pulse;
            
            juce::Colour innerColor = juce::Colours::darkslategrey.withAlpha(innerAlpha);
            juce::Colour outerColor = juce::Colours::darkslategrey.withAlpha(outerAlpha);
            
            g.setColour(outerColor);
            g.drawEllipse(center.x - outerRadius, center.y - outerRadius,
                        outerRadius * 2.0f, outerRadius * 2.0f, 2.0f); // Thicker line
            
            g.setColour(innerColor);
            g.drawEllipse(center.x - innerRadius, center.y - innerRadius,
                        innerRadius * 2.0f, innerRadius * 2.0f, 2.0f);
            
            // Draw spokes
            for (int i = 0; i < 6; i++) {
                float angle = i * juce::MathConstants<float>::pi / 3.0f + rotationAngle;
                float x1 = center.x + std::cos(angle) * innerRadius;
                float y1 = center.y + std::sin(angle) * innerRadius;
                float x2 = center.x + std::cos(angle) * outerRadius;
                float y2 = center.y + std::sin(angle) * outerRadius;
                
                float spokeAlpha = 0.15f + 0.15f * pulse;
                g.setColour(juce::Colours::darkslategrey.withAlpha(spokeAlpha));
                g.drawLine(x1, y1, x2, y2, 1.5f);
            }
        }
        
        // Draw the "0" text
        float fontSize = isActive ? 32.0f : 24.0f;
        g.setFont(juce::Font(juce::FontOptions(fontSize, juce::Font::bold)));
        
        if (isActive) {
            g.setColour(juce::Colours::black.withAlpha(0.4f));
            g.drawText("0", bounds.translated(1, 1), juce::Justification::centred);
        }
        
        g.setColour(isActive ? juce::Colours::white : juce::Colours::darkslategrey);
        g.drawText("0", bounds, juce::Justification::centred);
        
        // Handle hover and press states
        if (isMouseOverButton) {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, 10.0f);
        }
        
        if (isButtonDown) {
            g.setColour(juce::Colours::black.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, 10.0f);
        }
        
        // Draw button border
        g.setColour(isActive 
            ? juce::Colours::white.withAlpha(0.5f) 
            : juce::Colours::darkslategrey.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 10.0f, 1.5f);
    }
    
private:
    static constexpr int numWaves = 5;
    std::array<float, numWaves> wavePositions;
    float inactivePhase = 0.0f;
    float rotationAngle = 0.0f;
    
    // Handle animations
    void timerCallback() override
    {
        bool needsRepaint = false;
        
        if (getToggleState()) {
            // Update wave positions when active
            for (size_t i = 0; i < numWaves; ++i) {
                wavePositions[i] += 0.008f;
                
                if (wavePositions[i] >= 1.0f) {
                    wavePositions[i] = 0.0f;
                }
            }
            needsRepaint = true;
        } else {
            // Update inactive state animations
            inactivePhase += 0.03f;
            if (inactivePhase > juce::MathConstants<float>::twoPi) {
                inactivePhase -= juce::MathConstants<float>::twoPi;
            }
            
            rotationAngle += 0.005f;
            
            // Repaint less frequently when inactive
            static int frameCounter = 0;
            frameCounter = (frameCounter + 1) % 4;
            needsRepaint = (frameCounter == 0);
        }
        
        if (needsRepaint) {
            repaint();
        }
    }
};