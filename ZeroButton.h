#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

class ZeroButton : public juce::Button, private juce::Timer
{
public:
    ZeroButton() : juce::Button("ZeroButton")
    {
        setClickingTogglesState(true);
        setTooltip("Sets all phase components to zero.");
        
        for (int i = 0; i < 5; ++i) {
            wavePositions[i] = i * 0.2f;
        }
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        
        g.setColour(isActive ? juce::Colours::white.darker(0.05f) : juce::Colours::white);
        g.fillRoundedRectangle(bounds, 10.0f);
        
        if (isActive) {
            const float maxRadius = bounds.getWidth() * 0.7f;
            
            for (int i = 0; i < 5; ++i) {
                float position = wavePositions[i];
                float radius = position * maxRadius;
                
                float alpha = 0.6f * (1.0f - position);
                
                juce::Colour circleColor = juce::Colours::dodgerblue.withAlpha(alpha);
                g.setColour(circleColor);
                
                float ringThickness = 6.0f;
                g.drawEllipse(
                    center.x - radius, 
                    center.y - radius, 
                    radius * 2.0f, 
                    radius * 2.0f,
                    ringThickness
                );
            }
        } else {
            juce::ColourGradient gradient(
                juce::Colours::white,
                center.x, center.y,
                juce::Colours::lightblue.withAlpha(0.7f),
                bounds.getRight(), bounds.getBottom(),
                true
            );
            
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, 10.0f);
        }

        float fontSize = isActive ? 26.0f : 20.0f;
        g.setFont(juce::Font(fontSize).boldened());
        g.setColour(juce::Colours::darkslategrey);
        g.drawText("0", bounds, juce::Justification::centred);

        // Highlight effect for mouse over
        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.0f));
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
            repaint();
        }
    }
    
private:
    std::array<float, 5> wavePositions;
    
    void timerCallback() override
    {
        if (getToggleState()) {
            for (int i = 0; i < 5; ++i) {
                wavePositions[i] += 0.005f;
                
                if (wavePositions[i] >= 1.0f) {
                    wavePositions[i] = 0.0f;
                }
            }
            
            // Trigger repaint to show animation
            repaint();
        }
    }
};