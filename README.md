# AdvancedAudioTemplate
A template for quick development of audio plugins on a semi-professional level.

Dependencies:
TGMLib (to be created) as static lib and header files

CMake Basic Files from AudioDev. (to generate the lib)

## Purpose
This template provides some basic features for effects and synth, like:
* synchron block processing for arbitrary block sizes
* preset handler
* resizable GUI
* saving/loading of plugin state (with GUI size)
* keyboard  + pitch-wheel and modulation-wheel (midi insert and display) 
* a basic audioProcessing class for the core dsp code 
  

## Usage

1. Create a new directory (better create a new repository in GitHub)
2. (if Github): Checkout your new project
3. copy template files
4. rename all instances of "YourPluginName" in the Files with something apropriate 
    (use a renaming-tool like   "sed -i 's/YourPluginName/YourNewProjectName/g' *.*")   
5. Add your new subdiretory to the main CMakeLists.txt (in main directory AudioDev) file
6. Start coding your plugin


## Options:
* In the CMakeLists.txt you can add some defines (FACTORY_PRESETS and WITH_MIDIKEYBOARD). The second is recommended for synth.














