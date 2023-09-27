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
5. Rename YourPluginName.cpp and YourPluginName.h into YourNewProjectName.cpp and YourNewProjectName.h (e.g. Linux: 
```console    
    rename 's/YourPluginName/YourNewProjectName/' *.*     
```    
6. Add your new subdiretory to the main CMakeLists.txt (in main directory AudioDev) file
7. add or remove add_compile_definitions to your intention (Do you need a preset manager (default is yes), 
                                                            Do you need a midi-keyboard display (default is no)) 
8. Start coding your plugin


## Options:
* In the CMakeLists.txt you can add some defines (FACTORY_PRESETS and WITH_MIDIKEYBOARD). The second is recommended for synth.

## Example to use (tbd)

### Gain plugin (of course)

Apply Usage
for the 4th step: sed -i 's/YourPluginName/GainPlugin/g' *.*
for the 5th step: rename 's/YourPluginName/GainPlugin/' *.*     

for the 7th step switch off PresetHandlerGUI (for a simple gain not necessary) 














