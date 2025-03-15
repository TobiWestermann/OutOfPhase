#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class DiscreteSlider : public juce::Slider
{
public:
    double snapValue(double attemptedValue, juce::Slider::DragMode) override
    {
        constexpr std::array<double, 6> values = { 256, 512, 1024, 2048, 4096, 8192}; // Custom values

        auto closest = *std::min_element(values.begin(), values.end(),
            [attemptedValue](double a, double b)
            {
                return std::abs(a - attemptedValue) < std::abs(b - attemptedValue);
            });

        return closest;
    }
};