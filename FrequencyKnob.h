#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class FrequencyKnob : public juce::Slider
{
public:
    enum KnobType
    {
        LowFreq,
        HighFreq
    };

    FrequencyKnob(KnobType type) : m_type(type)
    {
        setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        
        // Create logarithmic range with skew factor
        // A skew factor of 0.2 gives roughly logarithmic behavior
        juce::NormalisableRange<double> freqRange(20.0, 20000.0, 1.0);
        freqRange.setSkewForCentre(m_type == LowFreq ? 500.0 : 2000.0);
        
        // Use a stronger skew for better low frequency control
        const double skewFactor = 0.2;
        freqRange.skew = skewFactor;
        
        setNormalisableRange(freqRange);
        
        // Set appropriate color based on knob type
        if (type == LowFreq) {
            m_knobColor = juce::Colour(60, 140, 180); // Cooler blue for low freq
            // Default to a low frequency value
            setValue(100.0, juce::dontSendNotification);
        } else {
            m_knobColor = juce::Colour(180, 60, 90);  // Warmer red for high freq
            // Default to a mid-high frequency value
            setValue(5000.0, juce::dontSendNotification);
        }
        
        setDoubleClickReturnValue(true, type == LowFreq ? 100.0 : 5000.0);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.38f;
        
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillEllipse(center.x - radius + 2.0f, center.y - radius + 2.0f, radius * 2.0f, radius * 2.0f);
        
        auto knobBounds = juce::Rectangle<float>(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::darkgrey.brighter(0.1f),
            center.x, knobBounds.getY(),
            juce::Colours::darkgrey.darker(0.2f),
            center.x, knobBounds.getBottom(),
            false
        ));
        g.fillEllipse(knobBounds);
        
        g.setColour(juce::Colours::grey.darker(0.6f));
        g.drawEllipse(knobBounds, 1.2f);
        
        float angleStart = juce::MathConstants<float>::pi * 1.2f;
        float angleRange = juce::MathConstants<float>::pi * 1.6f;
        
        float valueNormalized = static_cast<float>(getNormalisableRange().convertTo0to1(getValue()));
        float angleValue = angleStart + valueNormalized * angleRange;
        
        float arcThickness = radius * 0.15f;
        auto arcBounds = knobBounds.reduced(arcThickness / 2.0f);
        
        g.setColour(m_knobColor.darker(0.2f));
        g.drawEllipse(arcBounds, arcThickness);
        
        g.setColour(m_knobColor);
        juce::Path arcPath;
        arcPath.addArc(arcBounds.getX(), arcBounds.getY(), 
                       arcBounds.getWidth(), arcBounds.getHeight(),
                       angleStart, angleValue, arcThickness);
        g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        juce::Path pointerPath;
        auto pointerLength = radius * 0.7f;
        auto pointerThickness = radius * 0.1f;
        
        pointerPath.addRectangle(-pointerThickness * 0.5f, -radius + arcThickness, pointerThickness, pointerLength);
        
        g.setColour(juce::Colours::black);
        g.fillPath(pointerPath, juce::AffineTransform::rotation(angleValue).translated(center.x, center.y));
        
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.fillEllipse(center.x - radius * 0.15f, center.y - radius * 0.15f, radius * 0.3f, radius * 0.3f);

        g.setColour(juce::Colours::black.withAlpha(0.1f));
        g.fillEllipse(knobBounds.reduced(radius * 0.6f));
        
        juce::String valueText = formatFrequency(getValue());
        g.setFont(juce::Font(radius * 0.42f).boldened());
        g.setColour(juce::Colours::white);
        g.drawText(valueText, bounds, juce::Justification::centred);
        
        g.setFont(juce::Font(radius * 0.4f));
        g.setColour(juce::Colours::black);
        g.drawText("Hz", bounds, juce::Justification::centredBottom);
        
        juce::String typeLabel = m_type == LowFreq ? "LOW" : "HIGH"; 
        g.setFont(juce::Font(radius * 0.4f));
        g.setColour(juce::Colours::black);
        g.drawText(typeLabel, bounds, juce::Justification::centredTop);
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        auto dragX = e.getDistanceFromDragStartX();
        auto dragY = e.getDistanceFromDragStartY();
        
        if (e.mouseWasDraggedSinceMouseDown() && !m_dragging)
        {
            m_dragging = true;
            m_dragStartValue = getValue();
            m_lastDragPosition = e.position;
            return;
        }
        
        if (!m_dragging)
            return;
        
        float dragDelta = m_lastDragPosition.y - e.position.y;
        m_lastDragPosition = e.position;
        
        if (std::abs(dragDelta) < 0.1f)
            return;
            
        auto currentValue = getValue();
        double scaleFactor = currentValue / 500.0;
        
        scaleFactor = juce::jmax(0.05, scaleFactor);
        
        auto newValue = currentValue + dragDelta * scaleFactor * 5.0;
        
        newValue = juce::jlimit(getMinimum(), getMaximum(), newValue);
        
        setValue(newValue);
    }
    
    void mouseUp(const juce::MouseEvent& e) override
    {
        m_dragging = false;
        juce::Slider::mouseUp(e);
    }

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        auto currentValue = getValue();
        double scaleFactor = currentValue / 1000.0;
        
        scaleFactor = juce::jmax(0.05, scaleFactor);
        
        auto newValue = currentValue + wheel.deltaY * 200.0 * scaleFactor;
        
        newValue = juce::jlimit(getMinimum(), getMaximum(), newValue);
        
        setValue(newValue);
    }
    
private:
    KnobType m_type;
    juce::Colour m_knobColor;
    bool m_dragging = false;
    double m_dragStartValue = 0.0;
    juce::Point<float> m_lastDragPosition;
    
    juce::String formatFrequency(double freq) const
    {
        if (freq >= 1000.0)
            return juce::String(freq / 1000.0, 1) + "k";
        else
            return juce::String(static_cast<int>(freq));
    }
};