Lo-Fi Phone Effect Plugin - Usage Examples
==========================================

This JUCE audio plugin creates a lo-fi "phone call" effect using bandpass filtering.

PRESET SWITCHING:
-----------------
The plugin has 3 presets that can be switched programmatically:

Preset 0 - "Low": 
  - Bandpass: 400 Hz to 4,000 Hz
  - No distortion
  - Subtle phone effect

Preset 1 - "Medium": 
  - Bandpass: 300 Hz to 3,000 Hz  
  - No distortion
  - More pronounced phone effect

Preset 2 - "Intense": 
  - Bandpass: 250 Hz to 2,500 Hz
  - Includes soft clipping distortion (30% mix)
  - Heavy lo-fi phone effect

HOW TO SWITCH PRESETS IN CODE:
------------------------------
In your PluginProcessor instance, call:

    processor.setPreset(0);  // Low preset
    processor.setPreset(1);  // Medium preset  
    processor.setPreset(2);  // Intense preset

The current preset can be queried with:
    int currentPreset = processor.getCurrentPreset();

TECHNICAL DETAILS:
------------------
- Uses juce::dsp::IIR::Filter with high-pass + low-pass for bandpass effect
- Supports both mono and stereo processing
- Automatic filter state reset when switching presets
- Soft clipping distortion using tanh() function for the "Intense" preset
- Sample rate adaptive (filters update when sample rate changes)

BUILDING:
---------
1. Open the .jucer file in Projucer
2. Generate project files for your platform (Xcode, Visual Studio, etc.)
3. Build as AU/VST3 plugin
4. Load in your DAW and process vocal tracks

The effect works best on vocal recordings to simulate old phone call quality. 