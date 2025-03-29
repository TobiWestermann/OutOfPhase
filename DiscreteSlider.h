#pragma once

#include "CustomSlider.h"

class DiscreteSlider : public CustomSlider
{
public:
    DiscreteSlider() : CustomSlider()
    {
        setNumDecimalPlacesToDisplay(0);
        // setPopupDisplayEnabled(true, true, getParentComponent(), 300);
    }

    // Snaps slider to discrete values
    double snapValue(double attemptedValue, juce::Slider::DragMode) override
    {
        constexpr std::array<double, 6> values = { 256, 512, 1024, 2048, 4096, 8192};

        auto closest = *std::min_element(values.begin(), values.end(),
            [attemptedValue](double a, double b)
            {
                return std::abs(a - attemptedValue) < std::abs(b - attemptedValue);
            });

        return closest;
    }
    
    // Handles mouse wheel input to change between discrete values
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        double currentValue = getValue();
        
        constexpr std::array<double, 6> values = { 256, 512, 1024, 2048, 4096, 8192};
        int currentIndex = 0;
        
        // Find current value index
        for (int i = 0; i < values.size(); i++) {
            if (std::abs(currentValue - values[i]) < 0.001) {
                currentIndex = i;
                break;
            }
        }
        
        // Determine new index based on scroll direction
        int newIndex = currentIndex;
        if (wheel.deltaY > 0.0f) {
            newIndex = juce::jmin(currentIndex + 1, static_cast<int>(values.size() - 1));
        }
        else if (wheel.deltaY < 0.0f) {
            newIndex = juce::jmax(currentIndex - 1, 0);
        }
        
        if (newIndex != currentIndex) {
            setValue(values[newIndex], juce::sendNotificationAsync);
        }
    }
};