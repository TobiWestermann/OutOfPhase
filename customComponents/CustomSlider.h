#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class CustomSlider : public juce::Slider
{
public:
    CustomSlider() : juce::Slider()
    {
        setScrollWheelEnabled(true);
        
        setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        
        // Configure colors for slider components
        setColour(juce::Slider::trackColourId, juce::Colour(30, 160, 255));
        setColour(juce::Slider::backgroundColourId, juce::Colour(120, 120, 125));
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(100, 100, 110));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(70, 90, 120));
        
        setLookAndFeel(&customLookAndFeel);
    }

    ~CustomSlider() override
    {
        setLookAndFeel(nullptr);
    }
    
private:
    // Custom appearance for slider elements
    class CustomSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        CustomSliderLookAndFeel() {}
        
        // Handles drawing the slider track and thumb
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                             float sliderPos, float minSliderPos, float maxSliderPos,
                             const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            const bool isVertical = style == juce::Slider::LinearVertical;
            
            juce::Rectangle<int> bounds(x, y, width, height);
            
            // Calculate track position and size
            juce::Rectangle<float> trackBounds;
            
            if (isVertical)
            {
                const float trackWidth = width * 0.4f;
                const float trackX = x + (width - trackWidth) * 0.5f;
                trackBounds = juce::Rectangle<float>(trackX, y + 10.0f, trackWidth, height - 40.0f);
            }
            else
            {
                const float trackHeight = height * 0.4f;
                const float trackY = y + (height - trackHeight) * 0.5f;
                trackBounds = juce::Rectangle<float>(x + 10.0f, trackY, width - 20.0f, trackHeight);
            }
            
            // Draw track background
            juce::Colour backgroundColour = slider.findColour(juce::Slider::backgroundColourId);
            g.setColour(backgroundColour);
            g.fillRoundedRectangle(trackBounds, 4.0f);
            
            // Draw track border
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.drawRoundedRectangle(trackBounds.translated(1.0f, 1.0f), 4.0f, 1.0f);
            g.setColour(juce::Colours::black);
            g.drawRoundedRectangle(trackBounds, 4.0f, 1.5f);
            
            // Calculate and draw the filled part of the track
            juce::Rectangle<float> activeTrackBounds;
            
            if (isVertical)
            {
                float topPos = juce::jlimit(trackBounds.getY(), trackBounds.getBottom(), sliderPos);
                activeTrackBounds = trackBounds.withTop(topPos);
            }
            else
            {
                float rightPos = juce::jlimit(trackBounds.getX(), trackBounds.getRight(), sliderPos);
                activeTrackBounds = trackBounds.withRight(rightPos);
            }
            
            juce::Colour trackColour = slider.findColour(juce::Slider::trackColourId);
            g.setColour(trackColour);
            
            if (activeTrackBounds.getWidth() > 0 && activeTrackBounds.getHeight() > 0)
                g.fillRoundedRectangle(activeTrackBounds, 4.0f);
            
            // Draw the thumb
            juce::Rectangle<float> thumbBounds;
            float thumbSize = isVertical ? width * 0.6f : height * 0.6f;
            
            if (isVertical)
            {
                float thumbY = juce::jlimit(
                    trackBounds.getY() - thumbSize/2,
                    trackBounds.getBottom() - thumbSize/2,
                    sliderPos - thumbSize/2
                );
                
                thumbBounds = juce::Rectangle<float>(
                    trackBounds.getCentreX() - thumbSize/2,
                    thumbY,
                    thumbSize,
                    thumbSize
                );
            }
            else
            {
                float thumbX = juce::jlimit(
                    trackBounds.getX() - thumbSize/2,
                    trackBounds.getRight() - thumbSize/2,
                    sliderPos - thumbSize/2
                );
                
                thumbBounds = juce::Rectangle<float>(
                    thumbX,
                    trackBounds.getCentreY() - thumbSize/2,
                    thumbSize,
                    thumbSize
                );
            }
            
            // Draw thumb with highlight effect when active
            juce::Colour thumbColour = slider.findColour(juce::Slider::thumbColourId);
            
            if (slider.isMouseOverOrDragging())
                thumbColour = thumbColour.brighter(0.2f);
                
            g.setColour(thumbColour);
            g.fillRoundedRectangle(thumbBounds, 4.0f);
            
            g.setColour(thumbColour.brighter());
            g.fillRoundedRectangle(thumbBounds.withSizeKeepingCentre(thumbSize * 0.8f, thumbSize * 0.8f), 3.0f);
            
            const float borderThickness = 1.0f;
            g.setColour(juce::Colours::black);
            g.drawRoundedRectangle(thumbBounds, 4.0f, borderThickness);
            
            // Draw grip lines on thumb
            g.setColour(juce::Colours::darkgrey);
            float gripWidth = thumbSize * 0.5f;
            float gripHeight = thumbSize * 0.1f;
            float gripY = thumbBounds.getCentreY() - gripHeight/2;
            float gripX = thumbBounds.getCentreX() - gripWidth/2;
            
            for (int i = -1; i <= 1; i++)
            {
                float offset = i * gripHeight * 1.5f;
                
                if (isVertical)
                {
                    g.fillRoundedRectangle(gripX, gripY + offset, gripWidth, gripHeight, 1.0f);
                }
                else
                {
                    g.fillRoundedRectangle(gripY + offset, gripX, gripHeight, gripWidth, 1.0f);
                }
            }
        }

        // Create custom text box for slider value display
        juce::Label* createSliderTextBox(juce::Slider& slider) override
        {
            auto* l = new juce::Label();
            
            l->setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
            l->setJustificationType(juce::Justification::centred);
            
            // Configure label colors
            l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
            l->setColour(juce::Label::backgroundColourId, slider.findColour(juce::Slider::textBoxBackgroundColourId));
            l->setColour(juce::Label::outlineColourId, slider.findColour(juce::Slider::textBoxOutlineColourId));
            
            // Configure text editor colors
            l->setColour(juce::TextEditor::textColourId, juce::Colours::white);
            l->setColour(juce::TextEditor::backgroundColourId, juce::Colour(80, 110, 150));
            l->setColour(juce::TextEditor::highlightColourId, juce::Colour(50, 150, 255));
            l->setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::white);
            l->setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
            l->setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(50, 180, 255));
            
            l->setEditable(true, true, false);
            l->setBufferedToImage(true);
            
            return l;
        }

        // Custom drawing for text labels
        void drawLabel(juce::Graphics& g, juce::Label& label) override
        {
            juce::Rectangle<int> bounds = label.getLocalBounds();
            
            // Gradient background
            juce::ColourGradient gradient(
                juce::Colour(70, 100, 140),
                static_cast<float>(bounds.getX()), static_cast<float>(bounds.getY()),
                juce::Colour(50, 80, 120),
                static_cast<float>(bounds.getX()), static_cast<float>(bounds.getBottom()),
                false);

            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
            
            // Draw border
            g.setColour(juce::Colour(100, 120, 145));
            g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), 6.0f, 1.0f);
            
            // Highlight effect to top half
            juce::Path highlight;
            auto highlightBounds = bounds.toFloat().reduced(1.0f);
            highlight.addRoundedRectangle(
                highlightBounds.getX(), highlightBounds.getY(),
                highlightBounds.getWidth(), highlightBounds.getHeight() * 0.5f,
                5.0f, 5.0f, true, true, false, false);
            
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillPath(highlight);
            
            // Draw text if not being edited
            if (!label.isBeingEdited())
            {
                g.setColour(label.findColour(juce::Label::textColourId));
                g.setFont(label.getFont());
                
                g.drawText(label.getText(), bounds.reduced(4, 0),
                          label.getJustificationType(), true);
            }
        }
    };

    CustomSliderLookAndFeel customLookAndFeel;
};