#include "PluginProcessor.h"
#include "PluginEditor.h"

// NEW: Authentic phone colors (Phase 4)
const juce::Colour TestAudioProcessorEditor::NOKIA_BLUE = juce::Colour(0xff34495e);      // Classic Nokia blue-grey
const juce::Colour TestAudioProcessorEditor::IPHONE_GREY = juce::Colour(0xff5d5d5d);    // Space grey  
const juce::Colour TestAudioProcessorEditor::SONY_ORANGE = juce::Colour(0xffff6600);    // Iconic flip phone orange/red

//==============================================================================
// Phone display definitions (small display area within fixed window)
const TestAudioProcessorEditor::PhoneDisplay TestAudioProcessorEditor::phoneDisplays[3] = {
    // RetroCell 3300 - Classic chunky phone (Nokia-inspired)
{ "RetroCell 3300", juce::Rectangle<int>(), NOKIA_BLUE, juce::Colour(0xff9acd32), NOKIA_BLUE, 0.65f, 8, false, false, "RetroCell 3300" },
    
    // iPhone - Modern slim phone  
    { "iPhone", juce::Rectangle<int>(), IPHONE_GREY, juce::Colour(0xff000000), IPHONE_GREY, 0.50f, 20, true, false, "iPhone" },
    
    // Sony Ericsson - Flip phone
    { "Sony Ericsson", juce::Rectangle<int>(), SONY_ORANGE, juce::Colour(0xff2d3748), SONY_ORANGE, 0.75f, 6, false, true, "Sony Ericsson" }
};

//==============================================================================
TestAudioProcessorEditor::TestAudioProcessorEditor (TestAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set fixed window size
    setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Initialize with Nokia display
    currentDisplay = phoneDisplays[0];
    
    // Configure title label
    titleLabel.setText("Phone Effect", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Configure current phone label
    currentPhoneLabel.setText(currentDisplay.displayText, juce::dontSendNotification);
    currentPhoneLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    currentPhoneLabel.setJustificationType(juce::Justification::centred);
    currentPhoneLabel.setColour(juce::Label::textColourId, currentDisplay.accentColor);
    addAndMakeVisible(currentPhoneLabel);
    
    // Configure phone preset buttons (fixed at bottom)
    nokiaButton.setButtonText("RetroCell");
    nokiaButton.setColour(juce::TextButton::buttonColourId, phoneDisplays[0].bodyColor);
    nokiaButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    nokiaButton.addListener(this);
    addAndMakeVisible(nokiaButton);
    
    iphoneButton.setButtonText("TouchPro");
    iphoneButton.setColour(juce::TextButton::buttonColourId, phoneDisplays[1].bodyColor);
    iphoneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    iphoneButton.addListener(this);
    addAndMakeVisible(iphoneButton);
    
    sonyButton.setButtonText("FlipClassic");
    sonyButton.setColour(juce::TextButton::buttonColourId, phoneDisplays[2].bodyColor);
    sonyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    sonyButton.addListener(this);
    addAndMakeVisible(sonyButton);
    
    // Configure parameter controls as horizontal sliders with preset Hz markings
    // Low Cut slider - horizontal with Hz values
    lowCutSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lowCutSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30); // Bigger text box for Hz display
    lowCutSlider.setRange(0.0, 4.0, 1.0); // 5 positions: 0=Off, 1=180Hz, 2=250Hz, 3=300Hz, 4=400Hz
    lowCutSlider.setValue(0.0); // Start at "Off"
    // Configure for discrete snapping with good responsiveness
    lowCutSlider.setVelocityBasedMode(false); // Disable smooth velocity dragging
    lowCutSlider.setMouseDragSensitivity(50); // More sensitive for easier discrete movement
    lowCutSlider.setSkewFactor(1.0); // Linear response
    lowCutSlider.setDoubleClickReturnValue(true, 0.0); // Double-click returns to "Off"
    addAndMakeVisible(lowCutSlider);
    
    // High Cut slider - horizontal with Hz values  
    highCutSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    highCutSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
    highCutSlider.setRange(0.0, 4.0, 1.0); // 5 positions: 0=Off, 1=2.8kHz, 2=3.2kHz, 3=3.4kHz, 4=7kHz
    highCutSlider.setValue(0.0); // Start at "Off"
    // Configure for discrete snapping with good responsiveness
    highCutSlider.setVelocityBasedMode(false); // Disable smooth velocity dragging
    highCutSlider.setMouseDragSensitivity(50); // More sensitive for easier discrete movement
    highCutSlider.setSkewFactor(1.0); // Linear response
    highCutSlider.setDoubleClickReturnValue(true, 0.0); // Double-click returns to "Off"
    addAndMakeVisible(highCutSlider);
    
    // Main Effects: Full rotary knobs for precise control
    distortionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    distortionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(distortionSlider);
    
    compressionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compressionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(compressionSlider);
    
    // NEW: Wet/Dry Mix knob - THE MISSING PIECE!
    wetDrySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    wetDrySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(wetDrySlider);
    

    
    // GAME-CHANGING: Signal Quality ComboBox (replaces interference slider)
    signalQualityBox.addItem("Perfect (5 bars)", 1);
    signalQualityBox.addItem("Good (4 bars)", 2);
    signalQualityBox.addItem("Fair (3 bars)", 3);
    signalQualityBox.addItem("Poor (2 bars)", 4);
    signalQualityBox.addItem("Breaking Up (1 bar)", 5);
    signalQualityBox.addItem("Auto Dynamic", 6);
    signalQualityBox.setSelectedId(1); // FIX: Default to Perfect Signal (5 bars) instead of Auto Dynamic
    addAndMakeVisible(signalQualityBox);
    
    // NEW: Red interference error button
    interferenceButton.setButtonText("TV");
    interferenceButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8b0000)); // Dark red
    interferenceButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffff0000)); // Bright red when pressed
    interferenceButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    interferenceButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    interferenceButton.setClickingTogglesState(true);
    interferenceButton.addListener(this);
    addAndMakeVisible(interferenceButton);
    
    // Create parameter attachments for DAW automation
    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, TestAudioProcessor::LOW_CUT_ID, lowCutSlider);
    highCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, TestAudioProcessor::HIGH_CUT_ID, highCutSlider);
    distortionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, TestAudioProcessor::DISTORTION_ID, distortionSlider);

    // GAME-CHANGING: Signal Quality ComboBox Attachment  
    signalQualityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, TestAudioProcessor::INTERFERENCE_PRESET_ID, signalQualityBox);
    compressionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, TestAudioProcessor::COMPRESSION_ID, compressionSlider);
    wetDryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, TestAudioProcessor::WET_DRY_MIX_ID, wetDrySlider);
    
    // NEW: Interference button attachment
    interferenceButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, TestAudioProcessor::TV_INTERFERENCE_ID, interferenceButton);
    
    // Configure labels
    lowCutLabel.setText("Low Cut", juce::dontSendNotification);
    lowCutLabel.setJustificationType(juce::Justification::centred);
    lowCutLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    lowCutLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(lowCutLabel);
    
    highCutLabel.setText("High Cut", juce::dontSendNotification);
    highCutLabel.setJustificationType(juce::Justification::centred);
    highCutLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    highCutLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(highCutLabel);
    
    distortionLabel.setText("Distortion", juce::dontSendNotification);
    distortionLabel.setJustificationType(juce::Justification::centred);
    distortionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    distortionLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(distortionLabel);
    
    compressionLabel.setText("Compression", juce::dontSendNotification);
    compressionLabel.setJustificationType(juce::Justification::centred);
    compressionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    compressionLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(compressionLabel);
    
    // NEW: Wet/Dry Mix label - THE MISSING PIECE!
    wetDryLabel.setText("Wet/Dry Mix", juce::dontSendNotification);
    wetDryLabel.setJustificationType(juce::Justification::centred);
    wetDryLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    wetDryLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(wetDryLabel);
    

    
    // GAME-CHANGING: Signal Quality Label (replaces interference)
    signalQualityLabel.setText("Signal Quality", juce::dontSendNotification);
    signalQualityLabel.setJustificationType(juce::Justification::centred);
    signalQualityLabel.setColour(juce::Label::textColourId, juce::Colour(0xff66ffcc)); // Special cyan color for the game-changer!
    signalQualityLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(signalQualityLabel);
    
    // Dynamic Signal Strength Status Label (read-only display)
    signalStrengthLabel.setText("Signal: Auto Dynamic", juce::dontSendNotification);
    signalStrengthLabel.setJustificationType(juce::Justification::centred);
    signalStrengthLabel.setColour(juce::Label::textColourId, juce::Colour(0xff99ff99)); // Green for status
    signalStrengthLabel.setFont(juce::Font(10.0f));
    addAndMakeVisible(signalStrengthLabel);
    
    // Legacy interference label (hidden but kept for compatibility)
    interferenceLabel.setText("Legacy", juce::dontSendNotification);
    interferenceLabel.setJustificationType(juce::Justification::centred);
    interferenceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    interferenceLabel.setFont(juce::Font(12.0f));
    // addAndMakeVisible(interferenceLabel); // COMMENTED OUT - replaced by signal quality
    
    // NEW: Red TV interference button label
    interferenceButtonLabel.setText("TV Buzz", juce::dontSendNotification);
    interferenceButtonLabel.setJustificationType(juce::Justification::centred);
    interferenceButtonLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6666));
    interferenceButtonLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(interferenceButtonLabel);
    
    // NEW: Ghostline Audio branding label  
    ghostlineAudioLabel.setText("Ghostline Audio", juce::dontSendNotification);
    ghostlineAudioLabel.setJustificationType(juce::Justification::centredRight);
    ghostlineAudioLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888)); // Subtle grey
    ghostlineAudioLabel.setFont(juce::Font(10.0f, juce::Font::italic));
    addAndMakeVisible(ghostlineAudioLabel);
    
    // Set Nokia as initially selected
    nokiaButton.setToggleState(true, juce::dontSendNotification);
    
    // Initialize animated screen state
    screenState.scrollingText = "Cellyz - Phone Effect Active";
    screenState.textScrollPosition = 0.0f;
    screenState.signalBars = 4;
    screenState.batteryLevel = 85;
    screenState.isInCall = false;
    screenState.appName = "Phone FX";
    screenState.volumeBarLevel = 0.5f;
    screenState.isRecording = false;
    screenState.isFlipOpen = true;
    screenState.statusText = "READY";
    screenState.analogMeterLevel = 0.0f;
    screenState.hasNewMessage = false;
    screenState.timeDisplay = "12:34";
    screenState.audioLevel = 0.0f;
    screenState.isProcessingAudio = false;
    screenState.animationFrame = 0;
    
    // Initialize display and start animation timer
    currentDisplay = phoneDisplays[0];
    currentPhoneIndex = 0;
    currentPhoneLabel.setText("RetroCell 3300", juce::dontSendNotification);
    currentPhoneLabel.setColour(juce::Label::textColourId, phoneDisplays[0].accentColor);
    audioProcessor.loadPhonePreset(TestAudioProcessor::Nokia);
    
    // Set initial Nokia frequency response defaults (classic GSM: 300Hz-3.4kHz)
    audioProcessor.apvts.getParameter(TestAudioProcessor::LOW_CUT_ID)->setValueNotifyingHost(3.0f / 4.0f);  // Index 3: 300Hz
    audioProcessor.apvts.getParameter(TestAudioProcessor::HIGH_CUT_ID)->setValueNotifyingHost(3.0f / 4.0f); // Index 3: 3.4kHz
    
    // Set initial Nokia characteristic settings (realistic GSM phone)
    audioProcessor.apvts.getParameter(TestAudioProcessor::DISTORTION_ID)->setValueNotifyingHost(0.2f);    // 20% distortion

    audioProcessor.apvts.getParameter(TestAudioProcessor::INTERFERENCE_ID)->setValueNotifyingHost(0.15f); // 15% interference
    audioProcessor.apvts.getParameter(TestAudioProcessor::COMPRESSION_ID)->setValueNotifyingHost(0.5f);   // 50% compression
    
    // Start animation timer for screen updates
    startTimer(16); // ~60fps
    
    // Setup custom phone-themed LookAndFeel
    phoneLookAndFeel = std::make_unique<PhoneLookAndFeel>(0); // Start with Nokia
    
    // Apply custom styling to all controls
    lowCutSlider.setLookAndFeel(phoneLookAndFeel.get());
    highCutSlider.setLookAndFeel(phoneLookAndFeel.get());
    distortionSlider.setLookAndFeel(phoneLookAndFeel.get());
    wetDrySlider.setLookAndFeel(phoneLookAndFeel.get());
    signalQualityBox.setLookAndFeel(phoneLookAndFeel.get());  // FIXED: Changed from interferenceSlider
    compressionSlider.setLookAndFeel(phoneLookAndFeel.get());
    interferenceButton.setLookAndFeel(phoneLookAndFeel.get());
    
    // Initialize adaptive typography for RetroCell
    updateAdaptiveTypography();
    
    // IMPORTANT: Call resized() to initialize phoneDisplayArea before first paint()
    resized();
}

TestAudioProcessorEditor::~TestAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TestAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill background with dark gradient
    juce::ColourGradient gradient(juce::Colour(0xff2a2a2a), 0, 0,
                                  juce::Colour(0xff1a1a1a), 0, (float)getHeight(),
                                  false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Draw realistic phone
    drawPhoneDisplay(g, currentDisplay, phoneDisplayArea);
    
    // Draw button press effects
    drawButtonPressEffect(g);
}

void TestAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title area (more space for text)
    titleLabel.setBounds(bounds.removeFromTop(45).reduced(10));
    
    // Current phone label (more space for text)
    currentPhoneLabel.setBounds(bounds.removeFromTop(35).reduced(10));
    
    // Reserve bottom area for buttons (fixed)
    auto buttonArea = bounds.removeFromBottom(BUTTON_AREA_HEIGHT);
    
    // Create three columns: LEFT CONTROLS | PHONE | RIGHT CONTROLS
    auto leftControlsArea = bounds.removeFromLeft(140);   // Wider left side for 3 knobs
    auto rightControlsArea = bounds.removeFromRight(140); // Wider right side for 3 knobs
    auto centerArea = bounds;                             // Center for phone
    
    // Calculate phone display area (centered in middle column)
    auto displayWidth = juce::jmin(PHONE_DISPLAY_MAX_WIDTH, centerArea.getWidth() - 40);
    auto displayHeight = juce::jmin(PHONE_DISPLAY_MAX_HEIGHT, centerArea.getHeight() - 80);
    
    // Adjust display size based on current phone aspect ratio
    auto targetWidth = (int)(displayHeight * currentDisplay.aspectRatio);
    auto targetHeight = displayHeight;
    
    if (targetWidth > displayWidth)
    {
        targetWidth = displayWidth;
        targetHeight = (int)(targetWidth / currentDisplay.aspectRatio);
    }
    
    phoneDisplayArea = juce::Rectangle<int>(
        centerArea.getCentreX() - targetWidth / 2,
        centerArea.getCentreY() - targetHeight / 2,
        targetWidth,
        targetHeight
    );
    
    // Position LEFT SIDE CONTROLS (3 knobs vertically with more spacing)
    auto leftKnobHeight = 90;  // More space per knob
    auto leftKnobSpacing = 30; // More spacing between knobs
    auto leftStartY = (leftControlsArea.getHeight() - (3 * leftKnobHeight + 2 * leftKnobSpacing)) / 2;
    
    // Low Cut (top left) - Horizontal slider with Hz markings - positioned lower to avoid title clash
    lowCutSlider.setBounds(leftControlsArea.getX() + 10, leftStartY + 60, 120, 50); // More height for text box
    lowCutLabel.setBounds(leftControlsArea.getX() + 20, leftStartY + 42, 100, 15);
    
    // Distortion (middle left)
    distortionSlider.setBounds(leftControlsArea.getX() + 35, leftStartY + leftKnobHeight + leftKnobSpacing, 70, 70);
    distortionLabel.setBounds(leftControlsArea.getX() + 20, leftStartY + leftKnobHeight + leftKnobSpacing + 72, 100, 15);
    
    // GAME-CHANGING: Signal Quality (bottom left) - ComboBox instead of slider
    signalQualityBox.setBounds(leftControlsArea.getX() + 15, leftStartY + 2 * (leftKnobHeight + leftKnobSpacing), 110, 25);
    signalQualityLabel.setBounds(leftControlsArea.getX() + 15, leftStartY + 2 * (leftKnobHeight + leftKnobSpacing) - 18, 110, 15);
    signalStrengthLabel.setBounds(leftControlsArea.getX() + 15, leftStartY + 2 * (leftKnobHeight + leftKnobSpacing) + 28, 110, 12);
    
    // Legacy interference components (hidden)
    // interferenceSlider.setBounds(leftControlsArea.getX() + 35, leftStartY + 2 * (leftKnobHeight + leftKnobSpacing), 70, 70);
    // interferenceLabel.setBounds(leftControlsArea.getX() + 20, leftStartY + 2 * (leftKnobHeight + leftKnobSpacing) + 72, 100, 15);
    
    // Position RIGHT SIDE CONTROLS (3 knobs vertically with more spacing)
    auto rightStartY = leftStartY; // Same vertical alignment as left side
    
    // High Cut (top right) - Horizontal slider with Hz markings - positioned lower to avoid title clash
    highCutSlider.setBounds(rightControlsArea.getX() + 10, rightStartY + 60, 120, 50); // More height for text box
    highCutLabel.setBounds(rightControlsArea.getX() + 20, rightStartY + 42, 100, 15);
    
    // Compression (middle right) - MOVED UP from where noise was!
    compressionSlider.setBounds(rightControlsArea.getX() + 35, rightStartY + leftKnobHeight + leftKnobSpacing, 70, 70);
    compressionLabel.setBounds(rightControlsArea.getX() + 20, rightStartY + leftKnobHeight + leftKnobSpacing + 72, 100, 15);
    
    // NEW: Wet/Dry Mix (bottom right) - THE MISSING PIECE! 🔥
    wetDrySlider.setBounds(rightControlsArea.getX() + 35, rightStartY + 2 * (leftKnobHeight + leftKnobSpacing), 70, 70);
    wetDryLabel.setBounds(rightControlsArea.getX() + 20, rightStartY + 2 * (leftKnobHeight + leftKnobSpacing) + 72, 100, 15);
    
    // Position RED ERROR LED BUTTON (below phone, centered)
    auto errorButtonX = centerArea.getCentreX() - 30;
    auto errorButtonY = phoneDisplayArea.getBottom() + 20;
    interferenceButton.setBounds(errorButtonX, errorButtonY, 60, 25);
    interferenceButtonLabel.setBounds(errorButtonX - 20, errorButtonY + 27, 100, 15);
    
    // Position phone buttons at bottom (fixed positions)  
    buttonArea = buttonArea.reduced(10);
    auto buttonWidth = buttonArea.getWidth() / 3;
    nokiaButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    iphoneButton.setBounds(buttonArea.removeFromLeft(buttonWidth).reduced(2));
    sonyButton.setBounds(buttonArea.reduced(2));
    
    // Position Ghostline Audio branding in bottom right corner
    auto brandingArea = getLocalBounds().removeFromBottom(20).removeFromRight(120);
    ghostlineAudioLabel.setBounds(brandingArea);
}

//==============================================================================
void TestAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    // Show button press effect
    showButtonPressEffect(button);
    
    // Reset all button states
    nokiaButton.setToggleState(false, juce::dontSendNotification);
    iphoneButton.setToggleState(false, juce::dontSendNotification);
    sonyButton.setToggleState(false, juce::dontSendNotification);
    
    if (button == &nokiaButton)
    {
        animateToNewDisplay(phoneDisplays[0]);
        currentPhoneIndex = 0;
        nokiaButton.setToggleState(true, juce::dontSendNotification);
        audioProcessor.loadPhonePreset(TestAudioProcessor::Nokia);
        currentPhoneLabel.setText("RetroCell 3300", juce::dontSendNotification);
        currentPhoneLabel.setColour(juce::Label::textColourId, phoneDisplays[0].accentColor);
        
        // Set Nokia frequency response defaults (classic GSM: 300Hz-3.4kHz)
        audioProcessor.apvts.getParameter(TestAudioProcessor::LOW_CUT_ID)->setValueNotifyingHost(3.0f / 4.0f);  // Index 3: 300Hz
        audioProcessor.apvts.getParameter(TestAudioProcessor::HIGH_CUT_ID)->setValueNotifyingHost(3.0f / 4.0f); // Index 3: 3.4kHz
        
        // Set Nokia characteristic audio settings (realistic GSM phone) - using target values for smooth transitions
        audioProcessor.targetDistortion = 0.2f;    // 20% distortion
        audioProcessor.targetInterference = 0.15f; // 15% interference
        audioProcessor.targetCompression = 0.5f;   // 50% compression
        
        // Switch to Nokia-themed knobs
        phoneLookAndFeel->setPhoneType(0);
        updateAdaptiveTypography();
        repaint(); // Refresh knob styling
        
        // Update screen content for Nokia
        screenState.scrollingText = "Cellyz - RetroCell 3300 - Classic GSM Phone";
        screenState.signalBars = 4;
        screenState.batteryLevel = 85;
        screenState.isInCall = false;
    }
    else if (button == &iphoneButton)
    {
        animateToNewDisplay(phoneDisplays[1]);
        currentPhoneIndex = 1;
        iphoneButton.setToggleState(true, juce::dontSendNotification);
        audioProcessor.loadPhonePreset(TestAudioProcessor::iPhone);
        currentPhoneLabel.setText("TouchPro", juce::dontSendNotification);
        currentPhoneLabel.setColour(juce::Label::textColourId, phoneDisplays[1].accentColor);
        
        // Set iPhone characteristic audio settings (clean modern smartphone) - using target values for smooth transitions
        audioProcessor.targetDistortion = 0.05f;   // 5% minimal distortion
        audioProcessor.targetInterference = 0.04f; // 4% digital shielding
        audioProcessor.targetCompression = 0.2f;   // 20% modern compression
        
        // Switch to iPhone-themed knobs
        phoneLookAndFeel->setPhoneType(1);
        updateAdaptiveTypography();
        repaint(); // Refresh knob styling
        
        // Update screen content for iPhone
        screenState.appName = "Phone FX Pro";
        screenState.isRecording = true;
        screenState.volumeBarLevel = 0.7f;
        screenState.notifications.clear();
        screenState.notifications.add("Audio Effect Active");
    }
    else if (button == &sonyButton)
    {
        animateToNewDisplay(phoneDisplays[2]);
        currentPhoneIndex = 2;
        sonyButton.setToggleState(true, juce::dontSendNotification);
        audioProcessor.loadPhonePreset(TestAudioProcessor::SonyEricsson);
        currentPhoneLabel.setText("FlipClassic", juce::dontSendNotification);
        currentPhoneLabel.setColour(juce::Label::textColourId, phoneDisplays[2].accentColor);
        
        // Set Sony Ericsson frequency response defaults (vintage flip phone: 250Hz-2.8kHz)
        audioProcessor.apvts.getParameter(TestAudioProcessor::LOW_CUT_ID)->setValueNotifyingHost(2.0f / 4.0f);  // Index 2: 250Hz
        audioProcessor.apvts.getParameter(TestAudioProcessor::HIGH_CUT_ID)->setValueNotifyingHost(1.0f / 4.0f); // Index 1: 2.8kHz
        
        // Set Sony Ericsson characteristic audio settings (vintage analog flip phone) - using target values for smooth transitions
        audioProcessor.targetDistortion = 0.35f;   // 35% high distortion
        audioProcessor.targetInterference = 0.18f; // 18% RF susceptibility
        audioProcessor.targetCompression = 0.6f;   // 60% aggressive compression
        
        // Switch to Sony-themed knobs
        phoneLookAndFeel->setPhoneType(2);
        updateAdaptiveTypography(); // Update fonts for FlipClassic
        repaint(); // Refresh knob styling
        
        // Update screen content for Sony Ericsson
        screenState.isFlipOpen = true;
        screenState.statusText = "EFFECT ON";
        screenState.hasNewMessage = false;
        screenState.analogMeterLevel = 0.5f;
    }
}

//==============================================================================
void TestAudioProcessorEditor::animateToNewDisplay(const PhoneDisplay& newDisplay)
{
    if (currentDisplay.aspectRatio == newDisplay.aspectRatio)
        return; // No need to animate if same display
        
    targetDisplay = newDisplay;
    startDisplayAnimation(targetDisplay);
}

void TestAudioProcessorEditor::startDisplayAnimation(const PhoneDisplay& newDisplay)
{
    if (isAnimating)
        stopTimer();
        
    targetDisplay = newDisplay;
    animationProgress = 0.0f;
    isAnimating = true;
    startTimer(16); // ~60fps
}

void TestAudioProcessorEditor::timerCallback()
{
    // Handle display morphing animation
    if (isAnimating)
    {
        animationProgress += 0.15f; // Animation speed
        
        if (animationProgress >= 1.0f)
        {
            animationProgress = 1.0f;
            isAnimating = false;
            currentDisplay = targetDisplay;
            resized(); // Update phone display area size
        }
        else
        {
            // Interpolate between current and target display
            PhoneDisplay interpolatedDisplay;
            interpolatedDisplay.aspectRatio = currentDisplay.aspectRatio + (targetDisplay.aspectRatio - currentDisplay.aspectRatio) * animationProgress;
            interpolatedDisplay.cornerRadius = currentDisplay.cornerRadius + (targetDisplay.cornerRadius - currentDisplay.cornerRadius) * animationProgress;
            interpolatedDisplay.hasNotch = targetDisplay.hasNotch;
            interpolatedDisplay.bodyColor = currentDisplay.bodyColor.interpolatedWith(targetDisplay.bodyColor, animationProgress);
            interpolatedDisplay.screenColor = currentDisplay.screenColor.interpolatedWith(targetDisplay.screenColor, animationProgress);
            interpolatedDisplay.accentColor = currentDisplay.accentColor.interpolatedWith(targetDisplay.accentColor, animationProgress);
            interpolatedDisplay.displayText = targetDisplay.displayText;
            interpolatedDisplay.name = targetDisplay.name;
            interpolatedDisplay.isFlipPhone = targetDisplay.isFlipPhone;
            
            currentDisplay = interpolatedDisplay;
            resized(); // Update phone display area size during animation
        }
    }
    
    // Always update screen animations and audio monitoring
    updateScreenAnimations();
    updateAudioLevel();
    
    // GAME-CHANGING: Update Dynamic Signal Strength Display
    float currentSignal = audioProcessor.getCurrentSignalStrength();
    int signalBars = audioProcessor.getSignalBars();
    bool isDropping = audioProcessor.isCallDropping();
    float voiceActivity = audioProcessor.getVoiceActivity();
    
    // Update signal strength status label with real-time info
    juce::String signalStatusText;
    if (isDropping)
    {
        signalStatusText = "Signal: DROPPING";
        signalStrengthLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
    else
    {
        signalStatusText = "Signal: " + juce::String(signalBars) + "/5 bars (" + 
                          juce::String(static_cast<int>(currentSignal * 100)) + "%)";
        
        if (signalBars >= 4)
            signalStrengthLabel.setColour(juce::Label::textColourId, juce::Colour(0xff99ff99)); // Green
        else if (signalBars >= 3)
            signalStrengthLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffff99)); // Yellow
        else if (signalBars >= 2)
            signalStrengthLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffaa66)); // Orange
        else
            signalStrengthLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff6666)); // Red
    }
    
    signalStrengthLabel.setText(signalStatusText, juce::dontSendNotification);
    
    // Update screen state with real-time signal info for phone displays
    screenState.signalBars = signalBars;
    screenState.isProcessingAudio = voiceActivity > 0.1f;
    screenState.audioLevel = voiceActivity;
    
    // Handle button press effects
    if (pressedButton != nullptr)
    {
        buttonPressTimer++;
        if (buttonPressTimer > 30) // Effect duration
        {
            pressedButton = nullptr;
            buttonPressTimer = 0;
            buttonPressAlpha = 0.0f;
        }
        else
        {
            buttonPressAlpha = 1.0f - (buttonPressTimer / 30.0f);
        }
    }
    
    repaint();
}

void TestAudioProcessorEditor::drawPhoneDisplay(juce::Graphics& g, const PhoneDisplay& display, juce::Rectangle<int> displayArea)
{
    if (displayArea.isEmpty())
        return;
    
    // Draw realistic phone based on type
    if (currentPhoneIndex == 0) // Nokia 3310
    {
        drawNokia3310(g, displayArea);
    }
    else if (currentPhoneIndex == 1) // iPhone
    {
        drawIPhone(g, displayArea);
    }
    else if (currentPhoneIndex == 2) // Sony Ericsson Flip Phone
    {
        drawSonyEricssonFlip(g, displayArea);
    }
}

void TestAudioProcessorEditor::drawNokia3310(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto bounds = area.toFloat();
    
    // Nokia 3310 body - chunky rounded rectangle
    auto phoneBody = bounds.reduced(2);
    g.setColour(juce::Colour(0xff34495e)); // Dark blue-grey
    g.fillRoundedRectangle(phoneBody, 8.0f);
    
    // Phone body highlight
    g.setColour(juce::Colour(0xff5d6d7e));
    g.drawRoundedRectangle(phoneBody, 8.0f, 1.5f);
    
    // Screen area (upper 1/3) - now with animated content!
    auto screenArea = phoneBody.removeFromTop(phoneBody.getHeight() * 0.35f).reduced(8, 6);
    
    // Draw animated Nokia screen
    drawNokiaScreen(g, screenArea.toNearestInt());
    
    // Navigation cross (classic Nokia joystick)
    auto navArea = phoneBody.removeFromTop(phoneBody.getHeight() * 0.4f);
    auto navCenter = navArea.getCentre();
    auto navSize = 12.0f;
    
    // Center circle
    g.setColour(juce::Colour(0xff7f8c8d));
    g.fillEllipse(navCenter.x - navSize/4, navCenter.y - navSize/4, navSize/2, navSize/2);
    
    // Cross arms
    g.setColour(juce::Colour(0xff95a5a6));
    // Horizontal arm
    g.fillRect(navCenter.x - navSize, navCenter.y - 2.0f, navSize * 2.0f, 4.0f);
    // Vertical arm  
    g.fillRect(navCenter.x - 2.0f, navCenter.y - navSize, 4.0f, navSize * 2.0f);
    
    // Number keypad (3x4 grid)
    auto keypadArea = phoneBody.reduced(6);
    auto keyWidth = keypadArea.getWidth() / 3.0f;
    auto keyHeight = keypadArea.getHeight() / 4.0f;
    
    g.setColour(juce::Colour(0xff7f8c8d));
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            auto keyRect = juce::Rectangle<float>(
                keypadArea.getX() + col * keyWidth + 2,
                keypadArea.getY() + row * keyHeight + 2,
                keyWidth - 4,
                keyHeight - 4
            );
            g.fillRoundedRectangle(keyRect, 2.0f);
            
            // Key highlight
            g.setColour(juce::Colour(0xff95a5a6));
            g.drawRoundedRectangle(keyRect, 2.0f, 0.5f);
            g.setColour(juce::Colour(0xff7f8c8d));
        }
    }
    
    // Antenna
    g.setColour(juce::Colour(0xff95a5a6));
    auto antennaRect = juce::Rectangle<float>(bounds.getRight() - 3, bounds.getY() - 8, 2, 12);
    g.fillRoundedRectangle(antennaRect, 1.0f);
}

void TestAudioProcessorEditor::drawIPhone(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto bounds = area.toFloat();
    
    // iPhone body - sleek rounded rectangle
    auto phoneBody = bounds.reduced(1);
    g.setColour(juce::Colour(0xff1c1c1e)); // Space grey
    g.fillRoundedRectangle(phoneBody, 12.0f);
    
    // Phone body edge highlight
    g.setColour(juce::Colour(0xff48484a));
    g.drawRoundedRectangle(phoneBody, 12.0f, 1.0f);
    
    // Screen area (almost full phone) - now with animated content!
    auto screenArea = phoneBody.reduced(4);
    screenArea.removeFromTop(8); // Space for notch
    screenArea.removeFromBottom(8); // Space for home indicator
    
    // Draw animated iPhone screen
    drawIPhoneScreen(g, screenArea.toNearestInt());
    
    // Notch at top
    auto notchArea = juce::Rectangle<float>(
        phoneBody.getCentreX() - 20,
        phoneBody.getY() + 2,
        40,
        6
    );
    g.setColour(juce::Colour(0xff1c1c1e));
    g.fillRoundedRectangle(notchArea, 3.0f);
    
    // Home indicator at bottom
    auto homeIndicator = juce::Rectangle<float>(
        phoneBody.getCentreX() - 15,
        phoneBody.getBottom() - 6,
        30,
        2
    );
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.fillRoundedRectangle(homeIndicator, 1.0f);
    
    // Side buttons
    g.setColour(juce::Colour(0xff48484a));
    // Volume buttons
    g.fillRoundedRectangle(phoneBody.getX() - 2, phoneBody.getY() + 20, 2, 8, 1.0f);
    g.fillRoundedRectangle(phoneBody.getX() - 2, phoneBody.getY() + 30, 2, 8, 1.0f);
    // Power button
    g.fillRoundedRectangle(phoneBody.getRight(), phoneBody.getY() + 25, 2, 12, 1.0f);
}

void TestAudioProcessorEditor::drawSonyEricssonFlip(juce::Graphics& g, juce::Rectangle<int> area)
{
    auto bounds = area.toFloat();
    
    // Flip phone - two halves
    auto topHalf = bounds.removeFromTop(bounds.getHeight() * 0.45f);
    auto hinge = bounds.removeFromTop(bounds.getHeight() * 0.1f);
    auto bottomHalf = bounds;
    
    // Top half (screen section)
    g.setColour(juce::Colour(0xffe74c3c)); // Red/orange body
    g.fillRoundedRectangle(topHalf, 6.0f);
    
    // Top half highlight
    g.setColour(juce::Colour(0xffec7063));
    g.drawRoundedRectangle(topHalf, 6.0f, 1.0f);
    
    // External screen (small)
    auto extScreen = topHalf.reduced(4).removeFromTop(topHalf.getHeight() * 0.3f);
    g.setColour(juce::Colour(0xff2c3e50));
    g.fillRoundedRectangle(extScreen, 2.0f);
    
    // External screen content
    g.setColour(juce::Colour(0xff3498db));
    g.setFont(juce::Font(6.0f));
    g.drawText("12:34", extScreen, juce::Justification::centred);
    
    // Main screen area - now with animated content!
    auto mainScreen = topHalf.reduced(6).removeFromBottom(topHalf.getHeight() * 0.6f);
    
    // Draw animated Sony Ericsson screen
    drawSonyEricssonScreen(g, mainScreen.toNearestInt());
    
    // Hinge
    g.setColour(juce::Colour(0xff95a5a6));
    g.fillRect(hinge.reduced(4, 0));
    
    // Bottom half (keypad section)
    g.setColour(juce::Colour(0xffe74c3c));
    g.fillRoundedRectangle(bottomHalf, 6.0f);
    
    // Bottom half highlight
    g.setColour(juce::Colour(0xffec7063));
    g.drawRoundedRectangle(bottomHalf, 6.0f, 1.0f);
    
    // Keypad area
    auto keypadArea = bottomHalf.reduced(6);
    
    // Navigation buttons (top row)
    auto navRow = keypadArea.removeFromTop(keypadArea.getHeight() * 0.2f);
    g.setColour(juce::Colour(0xff7f8c8d));
    
    // Left/Right soft keys
    g.fillRoundedRectangle(navRow.removeFromLeft(navRow.getWidth() * 0.25f).reduced(1), 2.0f);
    g.fillRoundedRectangle(navRow.removeFromRight(navRow.getWidth() * 0.33f).reduced(1), 2.0f);
    
    // Center nav button
    auto centerNav = navRow.reduced(navRow.getWidth() * 0.2f, 1);
    g.fillEllipse(centerNav);
    
    // Call/End buttons
    auto callRow = keypadArea.removeFromTop(keypadArea.getHeight() * 0.25f);
    g.setColour(juce::Colour(0xff27ae60)); // Green call button
    g.fillEllipse(callRow.removeFromLeft(callRow.getWidth() * 0.4f).reduced(2));
    
    g.setColour(juce::Colour(0xffe74c3c)); // Red end button
    g.fillEllipse(callRow.removeFromRight(callRow.getWidth() * 0.67f).reduced(2));
    
    // Number keypad (3x4)
    g.setColour(juce::Colour(0xff95a5a6));
    auto keyWidth = keypadArea.getWidth() / 3.0f;
    auto keyHeight = keypadArea.getHeight() / 4.0f;
    
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            auto keyRect = juce::Rectangle<float>(
                keypadArea.getX() + col * keyWidth + 1,
                keypadArea.getY() + row * keyHeight + 1,
                keyWidth - 2,
                keyHeight - 2
            );
            g.fillRoundedRectangle(keyRect, 2.0f);
            
            // Key numbers
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(6.0f, juce::Font::bold));
            
            if (row < 3)
            {
                int number = row * 3 + col + 1;
                g.drawText(juce::String(number), keyRect, juce::Justification::centred);
            }
            else if (col == 1) // 0 key
            {
                g.drawText("0", keyRect, juce::Justification::centred);
            }
            else if (col == 0) // * key
            {
                g.drawText("*", keyRect, juce::Justification::centred);
            }
            else if (col == 2) // # key
            {
                g.drawText("#", keyRect, juce::Justification::centred);
            }
            
            g.setColour(juce::Colour(0xff95a5a6));
        }
    }
    
    // Small antenna
    g.setColour(juce::Colour(0xff95a5a6));
    auto antenna = juce::Rectangle<float>(bounds.getRight() - 2, topHalf.getY() - 6, 1, 8);
    g.fillRoundedRectangle(antenna, 0.5f);
}

void TestAudioProcessorEditor::drawPhoneFrame(juce::Graphics& g, const PhoneDisplay& display, juce::Rectangle<int> frameArea)
{
    // This method is no longer used since we draw complete realistic phones
}

juce::Path TestAudioProcessorEditor::createPhoneDisplayPath(const PhoneDisplay& display, juce::Rectangle<int> bounds)
{
    // This method is no longer used since we draw complete realistic phones
    juce::Path path;
    return path;
}

void TestAudioProcessorEditor::drawPhoneSpecificElements(juce::Graphics& g, const PhoneDisplay& display, juce::Rectangle<int> displayArea)
{
    // This method is no longer used since we draw complete realistic phones
}

//==============================================================================
// Animated Screen Methods

void TestAudioProcessorEditor::updateScreenAnimations()
{
    screenAnimationTimer++;
    screenState.animationFrame = (screenState.animationFrame + 1) % 60;
    
    // Update phone-specific animations
    switch (currentPhoneIndex)
    {
        case 0: updateNokiaAnimations(); break;
        case 1: updateIPhoneAnimations(); break;
        case 2: updateSonyEricssonAnimations(); break;
    }
    
    // Update time display
    auto now = juce::Time::getCurrentTime();
    screenState.timeDisplay = now.toString(false, true, false, false).substring(0, 5);
}

void TestAudioProcessorEditor::updateNokiaAnimations()
{
    // Scroll text - smoother scrolling with proper bounds
    screenState.textScrollPosition += 1.0f;
    float textTotalWidth = screenState.scrollingText.length() * 5.0f; // Approximate character width
    if (screenState.textScrollPosition > textTotalWidth + 80.0f) // Add padding
        screenState.textScrollPosition = 0.0f;
    
    // Animate signal bars based on audio activity
    if (screenState.isProcessingAudio)
    {
        screenState.signalBars = 3 + (screenState.animationFrame % 2);
    }
    else
    {
        screenState.signalBars = 4;
    }
    
    // Battery level slowly decreases
    if (screenState.animationFrame % 600 == 0) // Every 10 seconds
    {
        screenState.batteryLevel = juce::jmax(20, screenState.batteryLevel - 1);
    }
}

void TestAudioProcessorEditor::updateIPhoneAnimations()
{
    // Volume bar responds to audio level
    screenState.volumeBarLevel = audioLevelSmoothed;
    
    // Recording indicator blinks
    if (screenState.isProcessingAudio)
    {
        screenState.isRecording = (screenState.animationFrame % 30) < 15;
    }
    
    // Notifications appear/disappear
    if (screenState.animationFrame % 120 == 0 && screenState.isProcessingAudio)
    {
        screenState.notifications.clear();
        screenState.notifications.add("Processing Audio...");
        screenState.notifications.add("Effect: " + currentPhoneLabel.getText());
    }
}

void TestAudioProcessorEditor::updateSonyEricssonAnimations()
{
    // Analog meter responds to audio
    screenState.analogMeterLevel = audioLevelSmoothed * 0.8f + screenState.analogMeterLevel * 0.2f;
    
    // Status text changes
    if (screenState.isProcessingAudio)
    {
        if (screenState.animationFrame % 60 < 30)
            screenState.statusText = "PROCESSING";
        else
            screenState.statusText = "ACTIVE";
    }
    else
    {
        screenState.statusText = "READY";
    }
    
    // New message indicator
    screenState.hasNewMessage = (screenState.animationFrame % 180) < 90;
}

void TestAudioProcessorEditor::updateAudioLevel()
{
    // Get current audio level from processor (simplified)
    currentAudioLevel = getCurrentAudioLevel();
    
    // Smooth the audio level
    audioLevelSmoothed = audioLevelSmoothed * 0.7f + currentAudioLevel * 0.3f;
    
    // Update processing state
    screenState.isProcessingAudio = audioLevelSmoothed > 0.01f;
    screenState.audioLevel = audioLevelSmoothed;
}

float TestAudioProcessorEditor::getCurrentAudioLevel()
{
    // Simplified audio level detection
    // In a real implementation, this would get the actual audio level from the processor
    float level = 0.0f;
    
    // Simulate audio activity based on parameter changes
    level += std::abs(distortionSlider.getValue()) * 0.3f;
    level += std::abs(wetDrySlider.getValue()) * 0.1f;  // Wet/dry mix contributes to visual level
    level += (signalQualityBox.getSelectedItemIndex() / 5.0f) * 0.2f;  // FIXED: Changed from interferenceSlider
    
    return juce::jmin(1.0f, level);
}

void TestAudioProcessorEditor::showButtonPressEffect(juce::Button* button)
{
    pressedButton = button;
    buttonPressTimer = 0;
    buttonPressAlpha = 1.0f;
}

void TestAudioProcessorEditor::drawButtonPressEffect(juce::Graphics& g)
{
    if (pressedButton != nullptr && buttonPressAlpha > 0.0f)
    {
        g.setColour(juce::Colours::white.withAlpha(buttonPressAlpha * 0.3f));
        g.fillRoundedRectangle(pressedButton->getBounds().toFloat(), 5.0f);
    }
}

//==============================================================================
// Interactive Screen Element Drawing

void TestAudioProcessorEditor::drawSignalBars(juce::Graphics& g, juce::Rectangle<int> area, int bars)
{
    g.setColour(juce::Colours::lightgreen);
    
    int barWidth = area.getWidth() / 5;
    int barSpacing = 1;
    
    for (int i = 0; i < 4; ++i)
    {
        int barHeight = (i + 1) * (area.getHeight() / 4);
        juce::Rectangle<int> bar(area.getX() + i * (barWidth + barSpacing), 
                                area.getBottom() - barHeight, 
                                barWidth, barHeight);
        
        if (i < bars)
            g.fillRect(bar);
        else
            g.drawRect(bar, 1);
    }
}

void TestAudioProcessorEditor::drawBatteryIndicator(juce::Graphics& g, juce::Rectangle<int> area, int level)
{
    // Battery outline
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(area, 1);
    
    // Battery tip
    juce::Rectangle<int> tip(area.getRight(), area.getY() + 2, 2, area.getHeight() - 4);
    g.fillRect(tip);
    
    // Battery fill
    float fillWidth = (level / 100.0f) * (area.getWidth() - 2);
    juce::Rectangle<float> fill(area.getX() + 1, area.getY() + 1, fillWidth, area.getHeight() - 2);
    
    if (level > 50)
        g.setColour(juce::Colours::lightgreen);
    else if (level > 20)
        g.setColour(juce::Colours::orange);
    else
        g.setColour(juce::Colours::red);
        
    g.fillRect(fill);
}

void TestAudioProcessorEditor::drawVolumeBar(juce::Graphics& g, juce::Rectangle<int> area, float level)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(area);
    
    float fillHeight = level * area.getHeight();
    juce::Rectangle<float> fill(area.getX(), area.getBottom() - fillHeight, area.getWidth(), fillHeight);
    
    g.setColour(juce::Colours::cyan);
    g.fillRect(fill);
}

void TestAudioProcessorEditor::drawAnalogMeter(juce::Graphics& g, juce::Rectangle<int> area, float level)
{
    auto center = area.getCentre().toFloat();
    float radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.4f;
    
    // Meter background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2);
    
    // Meter needle
    float angle = -2.5f + level * 5.0f; // -2.5 to +2.5 radians
    float needleX = center.x + std::cos(angle) * radius * 0.8f;
    float needleY = center.y + std::sin(angle) * radius * 0.8f;
    
    g.setColour(juce::Colours::red);
    g.drawLine(center.x, center.y, needleX, needleY, 2.0f);
    
    // Center dot
    g.fillEllipse(center.x - 3, center.y - 3, 6, 6);
}

void TestAudioProcessorEditor::drawScrollingText(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text, float position)
{
    g.setColour(juce::Colours::lightgreen);
    g.setFont(juce::Font(8.0f));
    
    // Create a clipping region to prevent text from going outside the screen area
    g.saveState();
    g.reduceClipRegion(area);
    
    int textWidth = g.getCurrentFont().getStringWidth(text);
    int x = area.getX() + area.getWidth() - (int)position;
    
    // Draw the text
    g.drawText(text, x, area.getY(), textWidth, area.getHeight(), juce::Justification::centredLeft);
    
    g.restoreState();
}

//==============================================================================
// Animated Screen Drawing Methods

void TestAudioProcessorEditor::drawNokiaScreen(juce::Graphics& g, juce::Rectangle<int> screenArea)
{
    // Nokia 3310 LCD screen with green backlight
    g.setColour(juce::Colour(0xff1a4d1a));
    g.fillRect(screenArea);
    
    // Screen border
    g.setColour(juce::Colour(0xff0d260d));
    g.drawRect(screenArea, 1);
    
    auto contentArea = screenArea.reduced(3);
    
    // Top status bar
    auto statusArea = contentArea.removeFromTop(8);
    
    // Signal bars (top left)
    auto signalArea = statusArea.removeFromLeft(16);
    drawSignalBars(g, signalArea, screenState.signalBars);
    
    // Battery indicator (top right)
    auto batteryArea = statusArea.removeFromRight(12);
    drawBatteryIndicator(g, batteryArea, screenState.batteryLevel);
    
    // Time display (center)
    g.setColour(juce::Colours::lightgreen);
    g.setFont(juce::Font(7.0f));
    g.drawText(screenState.timeDisplay, statusArea, juce::Justification::centred);
    
    contentArea.removeFromTop(2); // spacing
    
    // Main text area with scrolling
    auto textArea = contentArea.removeFromTop(12);
    drawScrollingText(g, textArea, screenState.scrollingText, screenState.textScrollPosition);
    
    // Menu options
    g.setFont(juce::Font(6.0f));
    g.drawText("Menu", contentArea.removeFromBottom(8), juce::Justification::centredLeft);
    g.drawText("Back", contentArea.removeFromBottom(8), juce::Justification::centredRight);
}

void TestAudioProcessorEditor::drawIPhoneScreen(juce::Graphics& g, juce::Rectangle<int> screenArea)
{
    // iPhone screen with black background
    g.setColour(juce::Colours::black);
    g.fillRect(screenArea);
    
    // Screen border with rounded corners
    g.setColour(juce::Colour(0xff333333));
    g.drawRoundedRectangle(screenArea.toFloat(), 3.0f, 1.0f);
    
    auto contentArea = screenArea.reduced(4);
    
    // Top status bar
    auto statusArea = contentArea.removeFromTop(10);
    
    // Time (top left)
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(7.0f));
    g.drawText(screenState.timeDisplay, statusArea.removeFromLeft(25), juce::Justification::centredLeft);
    
    // Battery and signal (top right)
    auto batteryArea = statusArea.removeFromRight(15);
    drawBatteryIndicator(g, batteryArea.removeFromTop(6), screenState.batteryLevel);
    
    contentArea.removeFromTop(3); // spacing
    
    // App name
    auto appArea = contentArea.removeFromTop(12);
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.drawText(screenState.appName, appArea, juce::Justification::centred);
    
    contentArea.removeFromTop(2); // spacing
    
    // Volume bar
    auto volumeArea = contentArea.removeFromTop(20);
    volumeArea = volumeArea.reduced(8, 4);
    drawVolumeBar(g, volumeArea, screenState.volumeBarLevel);
    
    // Recording indicator
    if (screenState.isRecording)
    {
        auto recArea = contentArea.removeFromTop(8);
        g.setColour(juce::Colours::red);
        g.fillEllipse(recArea.getCentreX() - 3, recArea.getCentreY() - 3, 6, 6);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(6.0f));
        g.drawText("REC", recArea.reduced(10, 0), juce::Justification::centred);
    }
    
    // Notifications
    g.setFont(juce::Font(6.0f));
    for (int i = 0; i < screenState.notifications.size() && i < 2; ++i)
    {
        auto notifArea = contentArea.removeFromBottom(8);
        g.setColour(juce::Colours::lightblue);
        g.drawText(screenState.notifications[i], notifArea, juce::Justification::centred);
    }
}

void TestAudioProcessorEditor::drawSonyEricssonScreen(juce::Graphics& g, juce::Rectangle<int> screenArea)
{
    // Sony Ericsson monochrome LCD
    g.setColour(juce::Colour(0xff2a4a2a));
    g.fillRect(screenArea);
    
    // Screen border
    g.setColour(juce::Colour(0xff1a3a1a));
    g.drawRect(screenArea, 1);
    
    auto contentArea = screenArea.reduced(2);
    
    // Top line with time and signal
    auto topArea = contentArea.removeFromTop(8);
    
    g.setColour(juce::Colours::lightgreen);
    g.setFont(juce::Font(6.0f));
    g.drawText(screenState.timeDisplay, topArea.removeFromLeft(20), juce::Justification::centredLeft);
    
    // Signal strength as bars
    auto signalArea = topArea.removeFromRight(12);
    drawSignalBars(g, signalArea, screenState.signalBars);
    
    contentArea.removeFromTop(2); // spacing
    
    // Status text
    auto statusArea = contentArea.removeFromTop(10);
    g.setFont(juce::Font(7.0f, juce::Font::bold));
    g.drawText(screenState.statusText, statusArea, juce::Justification::centred);
    
    contentArea.removeFromTop(2); // spacing
    
    // Analog meter
    auto meterArea = contentArea.removeFromTop(16);
    meterArea = meterArea.reduced(6, 2);
    drawAnalogMeter(g, meterArea, screenState.analogMeterLevel);
    
    // New message indicator
    if (screenState.hasNewMessage)
    {
        auto msgArea = contentArea.removeFromBottom(8);
        g.setColour(juce::Colours::yellow);
        g.setFont(juce::Font(6.0f));
        g.drawText("1 NEW MSG", msgArea, juce::Justification::centred);
    }
}

//==============================================================================
// CUSTOM PHONE-THEMED CONTROLS (Authentic Hardware Look)

void TestAudioProcessorEditor::PhoneLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                                  float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                                                  juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height);
    
    // Get the current phone type from the editor
    if (auto* editor = dynamic_cast<TestAudioProcessorEditor*>(slider.getParentComponent()))
    {
        editor->drawPhoneKnob(g, bounds, sliderPos, currentPhoneType);
    }
}

void TestAudioProcessorEditor::PhoneLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                                                      const juce::Colour& backgroundColour,
                                                                      bool shouldDrawButtonAsHighlighted, 
                                                                      bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds();
    
    // Get the current phone type from the editor
    if (auto* editor = dynamic_cast<TestAudioProcessorEditor*>(button.getParentComponent()))
    {
        editor->drawLEDErrorButton(g, bounds, button.getToggleState(), shouldDrawButtonAsDown);
    }
}

void TestAudioProcessorEditor::drawPhoneKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value, int phoneType)
{
    switch (phoneType)
    {
        case 0: drawNokiaKnob(g, bounds, value); break;
        case 1: drawIPhoneKnob(g, bounds, value); break; 
        case 2: drawSonyKnob(g, bounds, value); break;
        default: drawNokiaKnob(g, bounds, value); break;
    }
}

void TestAudioProcessorEditor::drawNokiaKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value)
{
    auto centre = bounds.getCentre().toFloat();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
    
    // Nokia-style chunky metal knob with blue-grey finish
    g.setColour(juce::Colour(0xff4a5568)); // Dark blue-grey base
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    
    // Metallic rim highlight
    g.setColour(juce::Colour(0xff6b7280));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);
    
    // Inner knob surface with Nokia blue tint
    auto innerRadius = radius * 0.7f;
    g.setColour(juce::Colour(0xff5d6d7e));
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
    
    // Chunky grip notches (Nokia style)
    g.setColour(juce::Colour(0xff374151));
    for (int i = 0; i < 8; ++i)
    {
        auto angle = i * juce::MathConstants<float>::twoPi / 8.0f;
        auto notchRadius = radius * 0.85f;
        auto x1 = centre.x + std::cos(angle) * notchRadius;
        auto y1 = centre.y + std::sin(angle) * notchRadius;
        auto x2 = centre.x + std::cos(angle) * (notchRadius - 8);
        auto y2 = centre.y + std::sin(angle) * (notchRadius - 8);
        g.drawLine(x1, y1, x2, y2, 2.0f);
    }
    
    // Value indicator (chunky line)
    auto angle = juce::MathConstants<float>::twoPi * 0.75f + value * juce::MathConstants<float>::twoPi * 0.5f;
    auto indicatorRadius = radius * 0.6f;
    auto indicatorX = centre.x + std::cos(angle) * indicatorRadius;
    auto indicatorY = centre.y + std::sin(angle) * indicatorRadius;
    
    g.setColour(juce::Colour(0xff9ca3af)); // Light grey indicator
    g.fillEllipse(indicatorX - 3, indicatorY - 3, 6, 6);
}

void TestAudioProcessorEditor::drawIPhoneKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value)
{
    auto centre = bounds.getCentre().toFloat();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
    
    // iPhone-style sleek aluminum knob
    g.setColour(juce::Colour(0xff8e8e93)); // Space grey
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    
    // Subtle highlight ring
    g.setColour(juce::Colour(0xffaeaeb2));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);
    
    // Smooth inner surface
    auto innerRadius = radius * 0.8f;
    g.setColour(juce::Colour(0xff98989d));
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
    
    // Minimal precision notches (iPhone style)
    g.setColour(juce::Colour(0xff6d6d70));
    for (int i = 0; i < 12; ++i)
    {
        auto angle = i * juce::MathConstants<float>::twoPi / 12.0f;
        auto notchRadius = radius * 0.9f;
        auto x1 = centre.x + std::cos(angle) * notchRadius;
        auto y1 = centre.y + std::sin(angle) * notchRadius;
        auto x2 = centre.x + std::cos(angle) * (notchRadius - 4);
        auto y2 = centre.y + std::sin(angle) * (notchRadius - 4);
        g.drawLine(x1, y1, x2, y2, 1.0f);
    }
    
    // Precise value indicator
    auto angle = juce::MathConstants<float>::twoPi * 0.75f + value * juce::MathConstants<float>::twoPi * 0.5f;
    auto indicatorRadius = radius * 0.7f;
    auto indicatorX = centre.x + std::cos(angle) * indicatorRadius;
    auto indicatorY = centre.y + std::sin(angle) * indicatorRadius;
    
    g.setColour(juce::Colour(0xffd1d1d6)); // Light indicator
    g.fillEllipse(indicatorX - 2, indicatorY - 2, 4, 4);
}

void TestAudioProcessorEditor::drawSonyKnob(juce::Graphics& g, juce::Rectangle<int> bounds, float value)
{
    auto centre = bounds.getCentre().toFloat();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;
    
    // Sony-style orange/red accent knob
    g.setColour(juce::Colour(0xffd97706)); // Orange base
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
    
    // Metallic orange rim
    g.setColour(juce::Colour(0xfff59e0b));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 2.0f);
    
    // Textured inner surface
    auto innerRadius = radius * 0.75f;
    g.setColour(juce::Colour(0xffea580c));
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
    
    // Vintage-style ridged grip
    g.setColour(juce::Colour(0xffc2410c));
    for (int i = 0; i < 16; ++i)
    {
        auto angle = i * juce::MathConstants<float>::twoPi / 16.0f;
        auto notchRadius = radius * 0.88f;
        auto x1 = centre.x + std::cos(angle) * notchRadius;
        auto y1 = centre.y + std::sin(angle) * notchRadius;
        auto x2 = centre.x + std::cos(angle) * (notchRadius - 6);
        auto y2 = centre.y + std::sin(angle) * (notchRadius - 6);
        g.drawLine(x1, y1, x2, y2, 1.5f);
    }
    
    // Retro value indicator
    auto angle = juce::MathConstants<float>::twoPi * 0.75f + value * juce::MathConstants<float>::twoPi * 0.5f;
    auto indicatorRadius = radius * 0.5f;
    auto indicatorX = centre.x + std::cos(angle) * indicatorRadius;
    auto indicatorY = centre.y + std::sin(angle) * indicatorRadius;
    
    g.setColour(juce::Colour(0xfffbbf24)); // Bright yellow indicator
    g.fillEllipse(indicatorX - 3, indicatorY - 3, 6, 6);
}

void TestAudioProcessorEditor::drawLEDErrorButton(juce::Graphics& g, juce::Rectangle<int> bounds, bool isActive, bool isPressed)
{
    auto buttonBounds = bounds.toFloat().reduced(2);
    
    // LED housing (black plastic)
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(buttonBounds, 4.0f);
    
    // LED housing rim
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(buttonBounds, 4.0f, 1.0f);
    
    // LED light area
    auto ledBounds = buttonBounds.reduced(6);
    
    if (isActive || isPressed)
    {
        // Active LED - bright red with glow
        g.setColour(juce::Colour(0xffff0000)); // Bright red
        g.fillRoundedRectangle(ledBounds, 2.0f);
        
        // LED glow effect
        g.setColour(juce::Colour(0x40ff0000)); // Red glow
        g.fillRoundedRectangle(ledBounds.expanded(2), 3.0f);
        
        // LED text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("ERR", ledBounds.toNearestInt(), juce::Justification::centred);
    }
    else
    {
        // Inactive LED - dark red
        g.setColour(juce::Colour(0xff4a0000)); // Dark red
        g.fillRoundedRectangle(ledBounds, 2.0f);
        
        // Dim text
        g.setColour(juce::Colour(0xff808080));
        g.setFont(juce::Font(10.0f));
        g.drawText("ERR", ledBounds.toNearestInt(), juce::Justification::centred);
    }
    
    // LED surface reflection
    auto reflectionBounds = ledBounds.removeFromTop(ledBounds.getHeight() * 0.4f);
    g.setColour(juce::Colour(0x20ffffff));
    g.fillRoundedRectangle(reflectionBounds, 1.0f);
}

//==============================================================================
// ADAPTIVE TYPOGRAPHY - Changes fonts based on selected phone
//==============================================================================

void TestAudioProcessorEditor::updateAdaptiveTypography()
{
    juce::Font currentFont;
    
    switch (currentPhoneIndex)
    {
        case 0: // RetroCell 3300 (Nokia-inspired) - Pixelated retro font
            currentFont = juce::Font("Monaco", 12.0f, juce::Font::bold);
            titleLabel.setText("Cellyz", juce::dontSendNotification);
            break;
            
        case 1: // TouchPro (iPhone-inspired) - Clean modern font  
            currentFont = juce::Font("SF Pro Display", 12.0f, juce::Font::plain);
            titleLabel.setText("Cellyz", juce::dontSendNotification);
            break;
            
        case 2: // FlipClassic (Sony-inspired) - LCD-style font
            currentFont = juce::Font("Courier New", 12.0f, juce::Font::bold);
            titleLabel.setText("Cellyz", juce::dontSendNotification);
            break;
            
        default:
            currentFont = juce::Font(12.0f);
            break;
    }
    
    // Apply adaptive font to all labels
    titleLabel.setFont(currentFont.withHeight(18.0f));
    currentPhoneLabel.setFont(currentFont.withHeight(14.0f));
    lowCutLabel.setFont(currentFont);
    highCutLabel.setFont(currentFont);
    distortionLabel.setFont(currentFont);
    wetDryLabel.setFont(currentFont);
    interferenceLabel.setFont(currentFont);
    compressionLabel.setFont(currentFont);
    interferenceButtonLabel.setFont(currentFont.withHeight(10.0f));
    
    repaint();
}

//==============================================================================
// GAME-CHANGING: Dynamic Signal Strength Display Methods

void TestAudioProcessorEditor::drawDynamicSignalBars(juce::Graphics& g, juce::Rectangle<int> area, float signalStrength, bool isDropping)
{
    // Modern signal bars with dynamic strength indication
    auto barWidth = 4;
    auto barSpacing = 2;
    auto maxBars = 5;
    auto totalWidth = maxBars * barWidth + (maxBars - 1) * barSpacing;
    auto startX = area.getCentreX() - totalWidth / 2;
    
    // Calculate active bars based on signal strength
    int activeBars = static_cast<int>(signalStrength * maxBars);
    activeBars = juce::jlimit(0, maxBars, activeBars);
    
    for (int i = 0; i < maxBars; ++i)
    {
        auto barHeight = 6 + i * 3; // Increasing height
        auto barY = area.getBottom() - barHeight;
        auto barX = startX + i * (barWidth + barSpacing);
        
        juce::Rectangle<int> bar(barX, barY, barWidth, barHeight);
        
        if (i < activeBars)
        {
            // Active bars - color based on signal quality
            if (isDropping)
                g.setColour(juce::Colours::red); // Dropping signal
            else if (signalStrength > 0.8f)
                g.setColour(juce::Colours::green); // Excellent signal
            else if (signalStrength > 0.6f)
                g.setColour(juce::Colours::yellow); // Good signal
            else if (signalStrength > 0.3f)
                g.setColour(juce::Colours::orange); // Fair signal
            else
                g.setColour(juce::Colours::red); // Poor signal
        }
        else
        {
            // Inactive bars
            g.setColour(juce::Colour(0xff333333));
        }
        
        g.fillRoundedRectangle(bar.toFloat(), 1.0f);
    }
}

void TestAudioProcessorEditor::drawVoiceActivityIndicator(juce::Graphics& g, juce::Rectangle<int> area, float voiceActivity)
{
    // Voice activity level indicator (like a VU meter)
    auto indicatorWidth = area.getWidth() - 4;
    auto indicatorHeight = 8;
    auto indicatorArea = juce::Rectangle<int>(
        area.getX() + 2, 
        area.getCentreY() - indicatorHeight / 2,
        indicatorWidth, 
        indicatorHeight
    );
    
    // Background
    g.setColour(juce::Colour(0xff222222));
    g.fillRoundedRectangle(indicatorArea.toFloat(), 2.0f);
    
    // Voice activity level
    auto activeWidth = static_cast<int>(voiceActivity * indicatorWidth);
    if (activeWidth > 0)
    {
        auto activeArea = indicatorArea.withWidth(activeWidth);
        
        // Color based on activity level
        if (voiceActivity > 0.8f)
            g.setColour(juce::Colours::lime);
        else if (voiceActivity > 0.5f)
            g.setColour(juce::Colours::yellow);
        else if (voiceActivity > 0.2f)
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::green);
            
        g.fillRoundedRectangle(activeArea.toFloat(), 2.0f);
    }
}

void TestAudioProcessorEditor::drawCallQualityStatus(juce::Graphics& g, juce::Rectangle<int> area, int signalBars, bool isDropping)
{
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    
    juce::String statusText;
    juce::Colour statusColor;
    
    if (isDropping)
    {
        statusText = "CALL DROPPING";
        statusColor = juce::Colours::red;
    }
    else
    {
        switch (signalBars)
        {
            case 5: statusText = "EXCELLENT"; statusColor = juce::Colours::green; break;
            case 4: statusText = "GOOD"; statusColor = juce::Colours::lightgreen; break;
            case 3: statusText = "FAIR"; statusColor = juce::Colours::yellow; break;
            case 2: statusText = "POOR"; statusColor = juce::Colours::orange; break;
            case 1: statusText = "WEAK"; statusColor = juce::Colours::red; break;
            default: statusText = "NO SIGNAL"; statusColor = juce::Colours::darkred; break;
        }
    }
    
    g.setColour(statusColor);
    g.drawText(statusText, area, juce::Justification::centred, true);
}

void TestAudioProcessorEditor::drawSignalStrengthMeter(juce::Graphics& g, juce::Rectangle<int> area, float strength, bool animated)
{
    // Circular signal strength meter
    auto centerX = area.getCentreX();
    auto centerY = area.getCentreY();
    auto radius = juce::jmin(area.getWidth(), area.getHeight()) / 2 - 2;
    
    // Background circle
    g.setColour(juce::Colour(0xff222222));
    g.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2, 2.0f);
    
    // Signal strength arc
    auto startAngle = juce::MathConstants<float>::pi * 1.25f; // Start from bottom-left
    auto endAngle = juce::MathConstants<float>::pi * 2.75f;   // End at bottom-right
    auto currentAngle = startAngle + (endAngle - startAngle) * strength;
    
    // Color based on strength
    juce::Colour arcColor;
    if (strength > 0.8f) arcColor = juce::Colours::green;
    else if (strength > 0.6f) arcColor = juce::Colours::yellow;
    else if (strength > 0.3f) arcColor = juce::Colours::orange;
    else arcColor = juce::Colours::red;
    
    g.setColour(arcColor);
    
    // Draw arc
    juce::Path arcPath;
    arcPath.addCentredArc(centerX, centerY, radius - 4, radius - 4, 0.0f, startAngle, currentAngle, true);
    g.strokePath(arcPath, juce::PathStrokeType(3.0f));
    
    // Animated pulse effect
    if (animated && strength > 0.1f)
    {
        auto pulseAlpha = 0.3f + 0.2f * std::sin(screenAnimationTimer * 0.1f);
        g.setColour(arcColor.withAlpha(pulseAlpha));
        g.strokePath(arcPath, juce::PathStrokeType(6.0f));
    }
}

 