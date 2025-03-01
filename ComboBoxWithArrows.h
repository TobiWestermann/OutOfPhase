#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class ComboBoxWithArrows : public juce::Component
{
public:
    ComboBoxWithArrows()
    {
        // Füge Einträge zur ComboBox hinzu
        comboBox.addItem("Zero", 1);
        comboBox.addItem("Frost", 2);
        comboBox.addItem("Random", 3);
        comboBox.addItem("Flip", 4);
        
        comboBox.setSelectedId(1);
        addAndMakeVisible(comboBox);

        // Links-Pfeil Button
        leftButton.setButtonText("<");
        leftButton.onClick = [this] { selectPreviousItem(); };
        addAndMakeVisible(leftButton);

        // Rechts-Pfeil Button
        rightButton.setButtonText(">");
        rightButton.onClick = [this] { selectNextItem(); };
        addAndMakeVisible(rightButton);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        leftButton.setBounds(area.removeFromLeft(30));
        rightButton.setBounds(area.removeFromRight(30));
        comboBox.setBounds(area);
    }

    int getSelectedItemId() const
    {
        return comboBox.getSelectedId();
    }

    void addItems(const juce::StringArray& items)
    {
        comboBox.addItemList(items, 1);
    }

    void setOnSelectionChanged(std::function<void(int)> callback)
    {
        comboBox.onChange = [callback, this] { callback(comboBox.getSelectedId() - 1); };
    }

private:
    juce::ComboBox comboBox;
    juce::TextButton leftButton, rightButton;

    void selectPreviousItem()
    {
        int currentId = comboBox.getSelectedId();
        if (currentId > 1)
            comboBox.setSelectedId(currentId - 1);
        else
            comboBox.setSelectedId(comboBox.getNumItems());
    }

    void selectNextItem()
    {
        int currentId = comboBox.getSelectedId();
        if (currentId < comboBox.getNumItems())
            comboBox.setSelectedId(currentId + 1);
        else
            comboBox.setSelectedId(1);
    }
};
