#pragma once
// ------------Audio -----------------
const int g_desired_blocksize_ms(2); // its in ms to be independent from the sampling rate
const bool g_forcePowerOf2(false); // should be true for FFT Processing

// -------------- GUI -----------------
// global GUI setting for YourPluginName
const int g_minGuiSize_x(400);
const int g_maxGuiSize_x(1200);
const int g_minGuiSize_y(200);
const float g_guiratio = float(g_minGuiSize_y)/g_minGuiSize_x;

// for the presethandler and midikeyboard
const int g_minPresetHandlerHeight(30); // in pixels
#define MIN_COMBO_WITH_PRESET 120
#define MIN_ELEMENT_DIST_PRESET 10
#define MIN_BUTTON_WIDTH_PRESET 40
#define MIN_ELEMENT_HEIGHT_PRESET 20

const float g_midikeyboardratio(0.13); // in percent of height()


