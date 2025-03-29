#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "resources/images/snowflake_bin.h"
#include <random>

class FrostButton : public juce::Button, private juce::Timer
{
public:
    FrostButton() : juce::Button("FrostButton"), rotationAngle(0.0f)
    {
        setClickingTogglesState(true);
        setTooltip("Captures and maintains the current phase spectrum.");
        
        // Initialize snowflakes with random properties
        const int numFlakes = 20;
        snowflakes.resize(numFlakes);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> xDist(0.0f, 1.0f);
        std::uniform_real_distribution<float> yDist(-1.0f, 0.0f);
        std::uniform_real_distribution<float> sizeDist(0.5f, 2.5f);
        std::uniform_real_distribution<float> speedDist(0.2f, 0.8f);
        
        for (auto& flake : snowflakes)
        {
            flake.x = xDist(gen);
            flake.y = yDist(gen);
            flake.size = sizeDist(gen);
            flake.speed = speedDist(gen);
            flake.alpha = juce::jmap(flake.size, 0.5f, 2.5f, 0.3f, 0.8f);
        }
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isButtonDown);
        
        bool isActive = getToggleState();
        
        auto bounds = getLocalBounds().toFloat();

        // Draw button background
        if (isActive) {
            juce::ColourGradient gradient(
                juce::Colour(0xFF2C3E50), // Dark blue
                bounds.getX(), bounds.getY(),
                juce::Colour(0xFF34495E), // lighter blue
                bounds.getRight(), bounds.getBottom(),
                false);
            g.setGradientFill(gradient);
        } else {
            g.setColour(juce::Colours::white);
        }
        
        g.fillRoundedRectangle(bounds, 10.0f);
        
        // Draw snowflakes when active
        if (isActive) {
            for (const auto& flake : snowflakes) {
                float flakeSize = flake.size * juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.03f;
                float x = flake.x * bounds.getWidth();
                float y = flake.y * bounds.getHeight();
                
                g.setColour(juce::Colours::white.withAlpha(flake.alpha));
                g.fillEllipse(x - flakeSize/2, y - flakeSize/2, flakeSize, flakeSize);
            }
        }
        
        // Draw main snowflake image
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
            
            g.setColour(juce::Colours::white);
            g.drawImage(m_snowflakeImage, imageBounds, juce::RectanglePlacement::centred);
        } else {
            g.drawImage(m_snowflakeImage, imageBounds);
        }

        // Hover effect
        g.setColour(juce::Colours::white.withAlpha(isMouseOverButton ? 0.2f : 0.0f));
        g.fillRoundedRectangle(bounds, 10.0f);

        // Active state border
        if (isActive) {
            auto outlineBounds = bounds.reduced(0.5f);
            g.setColour(juce::Colour(0xFF7F9BAF));
            g.drawRoundedRectangle(outlineBounds, 10.0f, 1.5f);
        }
    }
    
    void clicked() override
    {
        Button::clicked();
        
        if (getToggleState()) {
            // Start animation when button is activated
            startTimerHz(60);
            
            // Reset snowflake positions
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> xDist(0.0f, 1.0f);
            std::uniform_real_distribution<float> yDist(-1.0f, 0.0f);
            
            for (auto& flake : snowflakes) {
                flake.x = xDist(gen);
                flake.y = yDist(gen);
            }
        } else {
            stopTimer();
            rotationAngle = 0.0f;
            repaint();
        }
    }
    
private:
    // Snowflake data structure
    struct Snowflake {
        float x;     // Position x (0-1)
        float y;     // Position y (0-1)
        float size;  // Size factor
        float speed; // Fall speed
        float alpha; // Transparency
    };
    
    std::vector<Snowflake> snowflakes;
    juce::Image m_snowflakeImage = juce::ImageFileFormat::loadFrom(snowflake_bin, snowflake_bin_len);
    float rotationAngle;
    
    void timerCallback() override
    {
        // Rotate main snowflake image
        rotationAngle += 0.01f;
        
        if (rotationAngle > juce::MathConstants<float>::twoPi) {
            rotationAngle -= juce::MathConstants<float>::twoPi;
        }

        // Animate snowflakes
        for (auto& flake : snowflakes) {
            flake.y += flake.speed * 0.01f;
            flake.x += 0.002f * std::sin(flake.y * 5.0f);
            
            // Reset snowflakes that fall off the bottom
            if (flake.y > 1.0f) {
                flake.y = -0.05f;
                
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> xDist(0.0f, 1.0f);
                flake.x = xDist(gen);
            }
            
            // Wrap snowflakes horizontally
            if (flake.x > 1.0f) flake.x -= 1.0f;
            if (flake.x < 0.0f) flake.x += 1.0f;
        }

        repaint();
    }
};