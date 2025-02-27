#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class ComboBoxWithArrows : public juce::Component
{
public:
    ComboBoxWithArrows()
    {
        // Füge Einträge zur ComboBox hinzu
        comboBox.addItem("Mode 1", 1);
        comboBox.addItem("Mode 2", 2);
        comboBox.addItem("Mode 3", 3);
        comboBox.addItem("Mode 4", 4);
        
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

private:
    juce::ComboBox comboBox;
    juce::TextButton leftButton, rightButton;

    void selectPreviousItem()
    {
        int currentId = comboBox.getSelectedId();
        if (currentId > 1)
            comboBox.setSelectedId(currentId - 1);
        if (currentId == 1)
            comboBox.setSelectedId(comboBox.getNumItems());
    }

    void selectNextItem()
    {
        int currentId = comboBox.getSelectedId();
        if (currentId < comboBox.getNumItems())
            comboBox.setSelectedId(currentId + 1);
        if (currentId == comboBox.getNumItems())
            comboBox.setSelectedId(1);
    }

    int getSelectedItemId() const
    {
        return comboBox.getSelectedId();
    }

    void addItems(const juce::StringArray& items)
    {
        comboBox.addItemList(items, 1);
    }

};
