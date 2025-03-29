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
        
        juce::NormalisableRange<double> freqRange(20.0, 20000.0, 1.0);
        freqRange.setSkewForCentre(m_type == LowFreq ? 500.0 : 2000.0);
        freqRange.skew = 0.2;
        setNormalisableRange(freqRange);
        
        if (type == LowFreq) {
            m_knobColor = juce::Colour(50, 130, 190);
            setValue(100.0, juce::dontSendNotification);
        } else {
            m_knobColor = juce::Colour(190, 70, 90);
            setValue(5000.0, juce::dontSendNotification);
        }
        
        setDoubleClickReturnValue(true, type == LowFreq ? 100.0 : 5000.0);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.43f;
        
        auto knobBounds = juce::Rectangle<float>(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
        
        g.setGradientFill(juce::ColourGradient(
            juce::Colours::grey.brighter(0.2f),
            center.x, knobBounds.getY(),
            juce::Colours::darkgrey,
            center.x, knobBounds.getBottom(),
            false
        ));
        g.fillEllipse(knobBounds);
        
        g.setColour(juce::Colours::lightgrey.withAlpha(0.4f));
        g.drawEllipse(knobBounds, 1.0f);
        
        float angleStart = juce::MathConstants<float>::pi * 1.2f;
        float angleRange = juce::MathConstants<float>::pi * 1.6f;
        
        float valueNormalized = static_cast<float>(getNormalisableRange().convertTo0to1(getValue()));
        float angleValue = angleStart + valueNormalized * angleRange;
        
        float arcThickness = radius * 0.18f;
        auto arcBounds = knobBounds.reduced(radius * 0.25f);
        
        g.setColour(juce::Colours::darkgrey);
        juce::Path arcBackgroundPath;
        arcBackgroundPath.addArc(arcBounds.getX(), arcBounds.getY(), 
                       arcBounds.getWidth(), arcBounds.getHeight(),
                       angleStart, angleStart + angleRange, arcThickness);
        g.strokePath(arcBackgroundPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        g.setColour(m_knobColor);
        juce::Path arcPath;
        arcPath.addArc(arcBounds.getX(), arcBounds.getY(), 
                       arcBounds.getWidth(), arcBounds.getHeight(),
                       angleStart, angleValue, arcThickness);
        g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        juce::Path pointerPath;
        auto pointerLength = radius * 0.4f;
        auto pointerThickness = radius * 0.12f;
        
        pointerPath.addRectangle(-pointerThickness * 0.5f, -radius * 0.9f, pointerThickness, pointerLength);
        
        pointerPath.addEllipse(-pointerThickness * 0.5f, -radius * 0.9f - pointerThickness * 0.5f,
                              pointerThickness, pointerThickness);
        
        g.setColour(juce::Colours::white);
        g.fillPath(pointerPath, juce::AffineTransform::rotation(angleValue).translated(center.x, center.y));
        
        juce::String valueText = formatFrequency(getValue());
        g.setFont(juce::Font(radius * 0.45f).boldened());
        g.setColour(juce::Colours::white);
        g.drawText(valueText, bounds, juce::Justification::centred);
        
        g.setFont(juce::Font(radius * 0.35f));
        g.setColour(juce::Colours::lightgrey);
        g.drawText("Hz", bounds.translated(0.0f, radius * 0.4f), juce::Justification::centred);
        
        juce::String typeLabel = m_type == LowFreq ? "LOW" : "HIGH"; 
        g.setFont(juce::Font(radius * 0.35f).boldened());
        g.setColour(m_knobColor.brighter(0.5f));
        g.drawText(typeLabel, bounds.translated(0.0f, -radius * 0.4f), juce::Justification::centred);
    }
    
    void mouseDrag(const juce::MouseEvent& e) override
    {
        const auto dragVertical = e.getDistanceFromDragStartY() * -1.0f;
        
        if (!m_dragging) {
            m_dragging = true;
            m_dragStartValue = getValue();
            m_previousDragDistance = 0.0f;
        }
        
        const float dragDelta = dragVertical - m_previousDragDistance;
        m_previousDragDistance = dragVertical;
        
        auto currentValue = getValue();
        const double sensitivity = 0.5;
        
        double scaleFactor = currentValue / 500.0;
        scaleFactor = juce::jmax(0.05, scaleFactor);
        
        double newValue = currentValue + dragDelta * sensitivity * scaleFactor * 10.0;
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
    float m_previousDragDistance = 0.0f;
    
    juce::String formatFrequency(double freq) const
    {
        if (freq >= 1000.0)
            return juce::String(freq / 1000.0, 1) + "k";
        else
            return juce::String(static_cast<int>(freq));
    }
};