# AdvancedAudioTemplate
A template for quick development of audio plugins on a semi-professional level.

Dependencies:

CMake Basic Files from AudioDev.

## Purpose
This template provides some basic features for effects and synth, like:
* synchron block processing for arbitrary block sizes (done, not tested)
* preset handler (done)
* resizable GUI (done)
* saving/loading of plugin state (with GUI size) (done)
* keyboard  + pitch-wheel and modulation-wheel (midi insert and display) (done) 
* a template plugin cpp and h for easy start. (done)

## Usage

1. Create a new directory (better create a new repository in GitHub)
2. (if Github): Checkout your new project
3. copy template files (https://github.com/JoergBitzer/AdvancedAudioTemplate)
4. rename all instances of "YourPluginName" in the Files with something appropriate 
    (use a renaming-tool like   
```console    
    sed -i 's/YourPluginName/YourNewProjectName/g' *.*
```    
for MacOS (https://stackoverflow.com/questions/4247068/sed-command-with-i-option-failing-on-mac-but-works-on-linux)
for Windows: (https://stackoverflow.com/questions/17144355/how-can-i-replace-every-occurrence-of-a-string-in-a-file-with-powershell)  

```console    
 C:\AudioDev\AAT_Test> Get-ChildItem '*.*' -Recurse | ForEach {
      (Get-Content $_ | ForEach  { $_ -replace 'YourPluginName', 'YourNewProjectName' }) |
      Set-Content $_ }
```    
or start the windows subsystem for linux

or use the tools given by visual studio code.

5. Rename YourPluginName.cpp and YourPluginName.h into YourNewProjectName.cpp and YourNewProjectName.h (e.g. Linux: 
```console    
    rename 's/YourPluginName/YourNewProjectName/' *.*     
```    
and Windows (cmd, not PS)
```console    
ren YourPluginName.* YourNewProjectName.*
```    

6. Add your new subdiretory to the main CMakeLists.txt (in main directory AudioDev) file
7. add or remove add_compile_definitions to your intention (Do you need a preset manager (default is yes), 
                                                            Do you need a midi-keyboard display (default is no)) 
8. Start coding your plugin


## Options:
* In the CMakeLists.txt you can add some defines (FACTORY_PRESETS and WITH_MIDIKEYBOARD). The second is recommended for synth.

## Example to use (tbd)

### Gain plugin (of course) 
[source code at](https://github.com/JoergBitzer/AAT_GainExample). I would use it 

1. Think about a name: Here, GainPlugin

2. Apply Usage
for the 4th step: sed -i 's/YourPluginName/GainPlugin/g' *.*
for MacOS, see (https://stackoverflow.com/questions/4247068/sed-command-with-i-option-failing-on-mac-but-works-on-linux)
for Windows: (https://stackoverflow.com/questions/17144355/how-can-i-replace-every-occurrence-of-a-string-in-a-file-with-powershell)  (for multiple files the solution is further down) or start the windows subsystem for linux

for the 5th step: rename 's/YourPluginName/GainPlugin/' *.* or by hand (just 2 files)

for the 7th step switch off PresetHandlerGUI (for a simple gain not necessary) 

3. Change the size in PluginSettings.h to something useful for a gain plugin 
```cpp
const int g_minGuiSize_x(200);
const int g_maxGuiSize_x(500);
const int g_minGuiSize_y(400);
```

4. 
In GainPLugin.h add the parameter definition (delete the example)
```cpp
const struct
{
	const std::string ID = "gain";
	const std::string name = "Gain";
	const std::string unitName = "dB";
	const float minValue = -80.f;
	const float maxValue = 20.f;
	const float defaultValue = 0.f;
}g_paramGain;

```
5. Add the support for a smoothed parameter
```cpp
private:
    float m_gain = 1.f;
    std::atomic<float>* m_gainParam = nullptr; 
    float m_gainParamOld = std::numeric_limits<float>::min(); //smallest possible number, will change in the first block
    juce::SmoothedValue<float,juce::ValueSmoothingTypes::Multiplicative> m_smoothedGain;

```

6. change addParameter (delete the example code)
```cpp
    paramVector.push_back(std::make_unique<juce::AudioParameterFloat>(g_paramGain.ID,            // parameterID
                                                        g_paramGain.name,            // parameter name
                                                        g_paramGain.minValue,              // minimum value
                                                        g_paramGain.maxValue,              // maximum value
                                                        g_paramGain.defaultValue));
```
7. change prepareParameter (delete the ignore method)
```cpp
    m_gainParam = vts->getRawParameterValue(g_paramGain.ID);
```


7. change the smoothing time for the SmoothedValues in prepareToPlay
```cpp
    // here your code
    m_smoothedGain.reset(sampleRate,0.0015); // 50ms is enough for a smooth gain, 
```

8. Change the processSynchronBlock method (delete ignore method)
```cpp
    // check parameter update
    if (*m_gainParam != m_gainParamOld)
    {
        m_gainParamOld = *m_gainParam;
        m_gain = powf(10.f,m_gainParamOld/20.f);
    }

    int NrOfSamples = buffer.getNumSamples();
    int chns = buffer.getNumChannels();

    //m_smoothedGain.setTargetValue(m_gain);
    m_smoothedGain.setTargetValue(m_gain);
    float curGain;
    for (int channel = 0; channel < chns; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        // ..do something to the data...
        for (int kk = 0; kk < NrOfSamples; ++kk)
        {
            if (channel == 0)
                curGain = m_smoothedGain.getNextValue();
            channelData[kk] *= curGain;
        }
    }
```

9. Component adjustment (add one slide)

In the header add the necessary variables it should like that
```cpp
private:
    AudioProcessorValueTreeState& m_apvts; 
    juce::Slider m_GainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_GainAttachment;
```

In the cpp file change the constructor
```cpp
    m_GainSlider.setRange (g_paramGain.minValue, g_paramGain.maxValue);         
    m_GainSlider.setTextValueSuffix (g_paramGain.unitName);    
    m_GainSlider.setSliderStyle(juce::Slider::LinearVertical);
    m_GainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove, true, 60, 20);
    m_GainSlider.setValue(g_paramGain.defaultValue);
	m_GainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, g_paramGain.ID, m_GainSlider);
	addAndMakeVisible(m_GainSlider);
```
and the setbounds method
```cpp
	auto r = getLocalBounds();
	m_GainSlider.setBounds(r);
```

10. compile and you have your gain plugin

Some remarks
For most parameter it is better to use small synchron blocks (e.g. 2ms) and smooth the update. If you need smoothing on a sample base (like for gains) use applyRamp from the buffer (it is linear, but faster compared to a sample-based smoothing). 






