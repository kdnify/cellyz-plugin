# Phone Effect Audio Plugin 📱🎵

A professional JUCE audio plugin that transforms your audio with authentic vintage phone effects. Features morphing phone displays, realistic phone graphics, and phone-specific audio processing.

![Plugin Screenshot](https://img.shields.io/badge/Status-Complete-brightgreen)
![JUCE](https://img.shields.io/badge/JUCE-Framework-blue)
![Platform](https://img.shields.io/badge/Platform-macOS-lightgrey)
![Format](https://img.shields.io/badge/Format-AU%20%7C%20VST3-orange)

## ✨ Features

### 🎨 Interactive Phone GUI
- **Morphing Phone Displays**: Smooth 60fps animations between different phone shapes
- **Realistic Phone Graphics**: Authentic representations of iconic phones
- **Animated Screens**: Live phone screens with real-time audio-responsive elements
- **Fixed Layout**: Controls positioned on sides for consistent user experience

### 📱 Three Iconic Phones

#### Nokia 3310 (2003)
- **Visual**: Dark blue-grey chunky body, classic LCD screen, 5-way joystick, 3×4 keypad, antenna
- **Screen**: Scrolling text, animated signal bars, live battery indicator, current time
- **Audio**: GSM interference (217Hz burst), digital noise, aggressive compression
- **Colors**: Classic Nokia blue theme

#### iPhone (2007)
- **Visual**: Space grey sleek body, black screen with app icons, notch, home indicator
- **Screen**: Live volume bar, blinking recording indicator, dynamic notifications, status bar
- **Audio**: Digital compression artifacts, minimal noise, smooth compression
- **Colors**: Modern grey/silver theme

#### Sony Ericsson Flip (2003)
- **Visual**: Red/orange flip phone, external screen, main screen, hinge, navigation buttons
- **Screen**: Analog VU meter with moving needle, status text, blinking message indicator
- **Audio**: Analog hiss and crackle, vintage compression, tape saturation
- **Colors**: Retro orange/red theme

### 🎛️ Professional Audio Controls

#### Core Parameters (DAW Automatable)
- **Low Cut Filter**: 50-2000Hz bandpass filtering
- **High Cut Filter**: 1000-20000Hz bandpass filtering  
- **Distortion**: 0-100% soft clipping distortion
- **Noise Level**: 0-100% phone-specific noise generation
- **Interference**: 0-100% GSM/digital/analog interference
- **Compression**: 0-100% phone-specific compression

#### Advanced Features
- **Real-time Parameter Display**: Shows "400 Hz", "50%" etc.
- **Thread-safe Audio Processing**: Atomic parameter access
- **Smooth Filter Updates**: No audio artifacts during parameter changes
- **Phone-specific Presets**: Authentic frequency responses and artifacts

## 🛠️ Technical Implementation

### Audio Processing
- **Modern JUCE Architecture**: Uses `AudioProcessorValueTreeState` for parameter management
- **Professional Parameter System**: Proper IDs, ranges, units, and DAW automation
- **Thread-safe Design**: `std::atomic<float>*` for real-time parameter access
- **State Management**: XML serialization for session saving/loading
- **Multiple Audio Artifacts**: GSM interference, digital noise, analog hiss, compression

### GUI System
- **Custom Look & Feel**: Phone-themed knobs and controls
- **Morphing Animation System**: Smooth transitions between phone shapes
- **Real-time Audio Monitoring**: Screen elements respond to audio activity
- **Professional Layout**: Fixed 400×500px window with side-mounted controls
- **Interactive Elements**: Button press feedback, visual animations

### Phone-Specific Features
- **Nokia**: GSM interference generation, scrolling text, signal bars
- **iPhone**: Digital processing, volume bars, app-style interface
- **Sony Ericsson**: Analog simulation, VU meters, flip phone mechanics

## 🚀 Development Journey

### Phase 1: Foundation (V1.0)
- Basic lo-fi phone effect with bandpass filtering
- 3 hardcoded presets with different frequency ranges
- Simple GUI showing only text
- Working AU plugin format

### Phase 2: GUI Revolution
- Interactive phone-themed interface with preset buttons
- Parameter sliders for real-time control
- Phone morphing system with dynamic window reshaping
- Fixed-size GUI design for better UX

### Phase 3: Professional Rewrite (V2.0)
- Complete processor rewrite with modern JUCE patterns
- Proper parameter management and DAW automation
- Thread-safe audio processing
- State saving/loading functionality

### Phase 4: Visual Enhancement
- Realistic phone graphics replacing simple blocks
- Authentic phone representations with detailed elements
- Phone-specific visual styling and colors

### Phase 5: Audio Realism
- Phone-specific audio artifacts and processing
- Enhanced noise generation and interference
- Compression and saturation effects

### Phase 6: Interactive Screens
- Animated phone screens with real-time elements
- Audio-responsive visual feedback
- 60fps smooth animations

### Final Phase: Layout Optimization
- Side-mounted controls to prevent overlapping
- Improved spacing and professional layout
- Enhanced user experience

## 🔧 Building

### Requirements
- **JUCE Framework**: Latest version
- **Xcode**: For macOS builds
- **macOS**: 10.13+ deployment target

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/yourusername/phone-effect-plugin.git
cd phone-effect-plugin

# Open in Projucer (if needed)
# Or build directly with Xcode
cd Builds/MacOSX
xcodebuild -project test.xcodeproj -scheme "test - AU" -configuration Release
```

### Supported Formats
- ✅ **Audio Unit (AU)**: Fully working
- ⚠️ **VST3**: SDK conflicts (AU recommended)

## 🎵 Usage

1. **Load the Plugin**: Install the AU component in your DAW
2. **Select Phone Type**: Click Nokia, iPhone, or Sony Ericsson buttons
3. **Adjust Parameters**: Use the side-mounted knobs for real-time control
4. **Watch the Magic**: Enjoy the morphing displays and animated screens
5. **Automate Everything**: All parameters support DAW automation

## 🎯 Perfect For

- **Lo-fi Hip Hop**: Authentic vintage phone textures
- **Vocal Processing**: Telephone-style vocal effects
- **Creative Sound Design**: Unique phone-specific artifacts
- **Retro Productions**: 2003 East London grime vibes
- **Podcast Processing**: Professional telephone simulation

## 📁 Project Structure

```
phone-effect-plugin/
├── Source/
│   ├── PluginProcessor.cpp    # Audio processing engine
│   ├── PluginProcessor.h      # Parameter definitions
│   ├── PluginEditor.cpp       # GUI implementation
│   └── PluginEditor.h         # GUI declarations
├── Builds/
│   └── MacOSX/               # Xcode project files
├── JuceLibraryCode/          # JUCE framework modules
├── test.jucer                # Projucer project file
└── README.md                 # This file
```

## 🏆 Achievements

- ✅ Professional parameter management system
- ✅ Realistic phone graphics and animations
- ✅ Thread-safe audio processing
- ✅ DAW automation support
- ✅ State saving/loading
- ✅ Multiple plugin formats
- ✅ Phone-specific audio artifacts
- ✅ Interactive animated screens
- ✅ Optimized user interface

## 🔮 Future Ideas

- Additional phone models (Blackberry, Motorola Razr)
- Preset management system
- MIDI control support
- Advanced modulation options
- Cross-platform builds (Windows, Linux)

## 📝 License

This project is open source. Feel free to use, modify, and distribute.

## 🙏 Acknowledgments

- **JUCE Framework**: For the excellent audio plugin framework
- **Vintage Phone Culture**: For the inspiration and nostalgic vibes
- **2003 East London Grime Scene**: For the authentic sound aesthetic

---

*Built with ❤️ and lots of phone nostalgia* 