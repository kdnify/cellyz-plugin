#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TestAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                  public juce::Button::Listener,
                                  public juce::Timer
{
public:
    TestAudioProcessorEditor (TestAudioProcessor&);
    ~TestAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Button listener
    void buttonClicked (juce::Button* button) override;

private:
    // Phone display configurations
    struct PhoneDisplay
    {
        juce::String name;
        juce::Rectangle<int> bounds;
        juce::Colour bodyColor;
        juce::Colour screenColor;
        juce::Colour accentColor;
        float aspectRatio;
        int cornerRadius;
        bool hasNotch;
        bool isFlipPhone;
        juce::String displayText;
    };
    
    // NEW: Authentic phone colors (Phase 4)
    static const juce::Colour NOKIA_BLUE;      // Classic Nokia blue-grey
    static const juce::Colour IPHONE_GREY;     // Space grey
    static const juce::Colour SONY_ORANGE;     // Iconic flip phone orange/red
    
    // Animated screen elements
    struct AnimatedScreen
    {
        // Nokia 3310 elements
        juce::String scrollingText;
        float textScrollPosition;
        int signalBars;
        int batteryLevel;
        bool isInCall;
        
        // iPhone elements
        juce::String appName;
        float volumeBarLevel;
        bool isRecording;
        juce::Array<juce::String> notifications;
        
        // Sony Ericsson elements
        bool isFlipOpen;
        juce::String statusText;
        float analogMeterLevel;
        bool hasNewMessage;
        
        // Common elements
        juce::String timeDisplay;
        float audioLevel;
        bool isProcessingAudio;
        int animationFrame;
    };
    
    static const PhoneDisplay phoneDisplays[3];
    
    // Fixed window dimensions
    static constexpr int WINDOW_WIDTH = 400;
    static constexpr int WINDOW_HEIGHT = 500;
    static constexpr int BUTTON_AREA_HEIGHT = 80;
    static constexpr int PHONE_DISPLAY_MAX_WIDTH = 160;
    static constexpr int PHONE_DISPLAY_MAX_HEIGHT = 200;
    
    // Methods for phone display rendering
    void drawPhoneDisplay(juce::Graphics& g, const PhoneDisplay& display, juce::Rectangle<int> displayArea);
    void drawNokia3310(juce::Graphics& g, juce::Rectangle<int> area);
    void drawIPhone(juce::Graphics& g, juce::Rectangle<int> area);
    void drawSonyEricssonFlip(juce::Graphics& g, juce::Rectangle<int> area);
    void animateToNewDisplay(const PhoneDisplay& newDisplay);
    
    // Animated screen rendering methods
    void drawNokiaScreen(juce::Graphics& g, juce::Rectangle<int> screenArea);
    void drawIPhoneScreen(juce::Graphics& g, juce::Rectangle<int> screenArea);
    void drawSonyEricssonScreen(juce::Graphics& g, juce::Rectangle<int> screenArea);
    
    // Interactive elements
    void drawSignalBars(juce::Graphics& g, juce::Rectangle<int> area, int bars);
    void drawBatteryIndicator(juce::Graphics& g, juce::Rectangle<int> area, int level);
    void drawVolumeBar(juce::Graphics& g, juce::Rectangle<int> area, float level);
    void drawAnalogMeter(juce::Graphics& g, juce::Rectangle<int> area, float level);
    void drawScrollingText(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text, float position);
    
    // GAME-CHANGING: Dynamic Signal Strength Display
    void drawDynamicSignalBars(juce::Graphics& g, juce::Rectangle<int> area, float signalStrength, bool isDropping);
    void drawVoiceActivityIndicator(juce::Graphics& g, juce::Rectangle<int> area, float voiceActivity);
    void drawCallQualityStatus(juce::Graphics& g, juce::Rectangle<int> area, int signalBars, bool isDropping);
    void drawSignalStrengthMeter(juce::Graphics& g, juce::Rectangle<int> area, float strength, bool animated);
    
    // Audio level monitoring
    void updateAudioLevel();
    void updatePhoneState();
    void loadPhoneState(int phoneIndex);
    void updateAdaptiveTypography();  // NEW: Changes fonts based on selected phone
    float getCurrentAudioLevel();
    
    // Screen animation updates
    void updateScreenAnimations();
    void updateNokiaAnimations();
    void updateIPhoneAnimations();
    void updateSonyEricssonAnimations();
    
    // Button press feedback
    void showButtonPressEffect(juce::Button* button);
    void drawButtonPressEffect(juce::Graphics& g);
    
    // Legacy methods (no longer used but kept for compatibility)
    void drawPhoneFrame(juce::Graphics& g, const PhoneDisplay& display, juce::Rectangle<int> frameArea);
    juce::Path createPhoneDisplayPath(const PhoneDisplay& display, juce::Rectangle<int> bounds);
    void drawPhoneSpecificElements(juce::Graphics& g, const PhoneDisplay& display, juce::Rectangle<int> displayArea);
    
    // Animation support
    void startDisplayAnimation(const PhoneDisplay& targetDisplay);
    void timerCallback() override;
    
    // Animation state
    bool isAnimating = false;
    PhoneDisplay currentDisplay;
    PhoneDisplay targetDisplay;
    float animationProgress = 0.0f;
    
    // Screen animation state
    AnimatedScreen screenState;
    int screenAnimationTimer = 0;
    
    // Button press effect state
    juce::Button* pressedButton = nullptr;
    float buttonPressAlpha = 0.0f;
    int buttonPressTimer = 0;
    
    // Audio level monitoring
    float currentAudioLevel = 0.0f;
    float audioLevelSmoothed = 0.0f;
    
    TestAudioProcessor& audioProcessor;
    
    // GUI Components - Fixed positions around phone display
    juce::Label titleLabel;
    juce::Label currentPhoneLabel;
    
    // Phone preset buttons (fixed at bottom)
    juce::TextButton nokiaButton;
    juce::TextButton iphoneButton;
    juce::TextButton sonyButton;
    
    // Parameter controls (positioned around phone display) - DAW Automatable!
    juce::Slider lowCutSlider;      // Back to slider for better control
    juce::Slider highCutSlider;     // Back to slider for better control
    juce::Slider distortionSlider;
    juce::Slider noiseLevelSlider;
    juce::Slider compressionSlider;
    
    // GAME-CHANGING: Signal Quality Control (replaces interference)
    juce::ComboBox signalQualityBox; // Choice between Perfect, Good, Fair, Poor, Breaking Up, Auto Dynamic
    
    // DYNAMIC: Signal Strength Display Components (read-only, shows real-time status)
    juce::Rectangle<int> signalBarsArea;        // Area for signal bars display
    juce::Rectangle<int> voiceActivityArea;     // Area for voice activity indicator
    juce::Rectangle<int> callQualityArea;       // Area for call quality status
    
    // NEW: Red interference error button (legacy - may remove)
    juce::TextButton interferenceButton;
    bool interferenceActive = false;
    
    juce::Label lowCutLabel;
    juce::Label highCutLabel;
    juce::Label distortionLabel;
    juce::Label noiseLevelLabel;
    juce::Label compressionLabel;
    juce::Label signalQualityLabel;         // NEW: Signal Quality label
    juce::Label signalStrengthLabel;        // NEW: Signal Strength status label
    
    // Legacy labels (may remove)
    juce::Label interferenceLabel;
    juce::Label interferenceButtonLabel;

    // Parameter Attachments for DAW automation
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compressionAttachment;
    
    // GAME-CHANGING: Signal Quality Attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> signalQualityAttachment;
    
    // Legacy attachments (may remove)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> interferenceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> interferenceButtonAttachment;
    
    // Current phone index
    int currentPhoneIndex = 0;
    
    // Phone display area (small centered area)
    juce::Rectangle<int> phoneDisplayArea;

    // Custom LookAndFeel for phone-themed controls
    class PhoneLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        PhoneLookAndFeel(int phoneType) : currentPhoneType(phoneType) {}
        
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                             float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                             juce::Slider& slider) override;
                             
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                 bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
        
        void setPhoneType(int phoneType) { currentPhoneType = phoneType; }
        
    private:
        int currentPhoneType = 0; // 0=Nokia, 1=iPhone, 2=Sony
    };
    
    std::unique_ptr<PhoneLookAndFeel> phoneLookAndFeel;
    
    // Custom drawing methods for phone-themed controls
    void drawPhoneKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value, int phoneType);
    void drawLEDErrorButton(juce::Graphics& g, juce::Rectangle<int> bounds, bool isActive, bool isPressed);
    void drawNokiaKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value);
    void drawIPhoneKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value);
    void drawSonyKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestAudioProcessorEditor)
}; 