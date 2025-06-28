#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class TestAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TestAudioProcessor();
    ~TestAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Phone presets
    enum PhoneType
    {
        Nokia = 0,
        iPhone = 1,
        SonyEricsson = 2
    };
    
    // Parameter ID constants for DAW automation
    static const juce::String LOW_CUT_ID;
    static const juce::String HIGH_CUT_ID;
    static const juce::String DISTORTION_ID;
    static const juce::String PHONE_TYPE_ID;
    static const juce::String NOISE_LEVEL_ID;
    static const juce::String INTERFERENCE_ID;
    static const juce::String COMPRESSION_ID;
    static const juce::String TV_INTERFERENCE_ID;  // NEW: TV interference toggle
    
    // PHASE 5: Advanced Audio Processing
    static const juce::String CODEC_TYPE_ID;       // Real codec simulation
    static const juce::String PACKET_LOSS_ID;      // Packet loss simulation
    static const juce::String CALL_POSITION_ID;    // Stereo positioning
    static const juce::String AMBIENCE_TYPE_ID;    // Background ambience
    static const juce::String AMBIENCE_LEVEL_ID;   // Ambience volume
    
    // NEW: Authentic interference preset system
    static const juce::String INTERFERENCE_PRESET_ID;
    
    // Phone-specific interference presets (REPLACED WITH DYNAMIC SIGNAL STRENGTH)
    enum SignalQuality
    {
        // Dynamic signal strength modes (GAME-CHANGING!)
        Perfect_Signal = 0,         // Crystal clear (5 bars)
        Good_Signal = 1,            // Minor compression (4 bars)  
        Fair_Signal = 2,            // Noticeable quality loss (3 bars)
        Poor_Signal = 3,            // Frequent dropouts (2 bars)
        Breaking_Up = 4,            // Severe cutting out (1 bar)
        Auto_Dynamic = 5            // INTELLIGENT: Quality changes based on voice activity!
    };
    
    // PHASE 5: Advanced Audio Processing Enums
    
    // Real codec simulation types
    enum CodecType
    {
        GSM_FullRate = 0,       // Original GSM codec (13 kbps)
        GSM_HalfRate = 1,       // GSM half-rate (5.6 kbps)  
        CDMA_QCELP = 2,         // CDMA QCELP codec
        AMR_4_75 = 3,           // AMR 4.75 kbps (very compressed)
        AMR_12_2 = 4,           // AMR 12.2 kbps (higher quality)
        Early_VoIP = 5,         // Early internet calling artifacts
        Digital_Artifact = 6    // Extreme digital compression
    };
    
    // Call positioning in stereo field
    enum CallPosition  
    {
        Center = 0,             // Standard mono center
        LeftEar = 1,            // Phone held to left ear
        RightEar = 2,           // Phone held to right ear
        Speaker_Near = 3,       // Speakerphone close
        Speaker_Far = 4,        // Speakerphone distant
        Bluetooth_Left = 5,     // Bluetooth earpiece left
        Bluetooth_Right = 6     // Bluetooth earpiece right
    };
    
    // Background ambience types
    enum AmbienceType
    {
        Silent = 0,             // No background
        Cafe_Busy = 1,          // Busy café chatter
        Car_Highway = 2,        // Car on highway
        Street_Traffic = 3,     // City street noise
        Underground_Tube = 4,   // London Underground
        Office_Quiet = 5,       // Quiet office hum
        Train_Interior = 6,     // Inside moving train
        Airport_Terminal = 7    // Airport background
    };
    
    // Call states
    enum CallState
    {
        Idle = 0,
        Ringing = 1,
        Connected = 2,
        Disconnected = 3
    };
    
    // UK Network types (2003 era)
    enum NetworkType
    {
        Orange = 0,
        Vodafone = 1,
        O2 = 2,
        TMobile = 3
    };
    
    // Nokia ringtones
    enum RingtoneType
    {
        Badinerie = 0,
        GrandeValse = 1,
        ForElise = 2,
        NokiaTune = 3,
        Mosquito = 4
    };
    
    // Public parameter tree for DAW automation
    juce::AudioProcessorValueTreeState apvts;
    
    // Simplified interference system (no more complex call simulation)
    
    // Audio level monitoring for GUI
    float getCurrentAudioLevel() const { return currentAudioLevel.load(); }
    
    // GAME-CHANGING: Dynamic Signal Strength System (public for GUI display)
    float getCurrentSignalStrength() const { return currentSignalStrength; }
    int getSignalBars() const { return signalBars; }
    bool isCallDropping() const { return isInDropout; }
    float getVoiceActivity() const { return voiceActivityLevel; }
    
    // Phone preset loading
    void loadPhonePreset(PhoneType phoneType);
    
    // AUTHENTIC INTERFERENCE METHODS (REPLACED WITH DYNAMIC SIGNAL STRENGTH)
    float applyAuthenticInterference(float input, PhoneType phoneType, int preset, float noiseLevel, float interferenceLevel);
    float applyNokiaInterference(float input, int preset, float noiseLevel, float interferenceLevel);
    float applyIPhoneInterference(float input, int preset, float noiseLevel, float interferenceLevel);
    float applySonyEricssonInterference(float input, int preset, float noiseLevel, float interferenceLevel);
    
    // GAME-CHANGING: Dynamic Signal Strength Methods
    void updateSignalStrength(float inputLevel);           // Analyze audio and update signal
    float applySignalQuality(float input, PhoneType phoneType, SignalQuality quality);
    void detectVoiceActivity(float inputLevel);            // Voice activity detection
    void simulateCallDynamics();                          // Realistic call behavior simulation
    float applySmartCompression(float input, float signalStrength); // Adaptive compression
    
    // Phone-specific interference presets (REPLACED WITH DYNAMIC SIGNAL STRENGTH)
    float applyPhoneDistortion(float input, PhoneType phoneType, float amount);
    float applyPhoneCompression(float input, PhoneType phoneType, float amount);

    // NEW: Phone-specific tonal coloring methods
    float applyPhoneTonalColor(float input, PhoneType phoneType, float intensity);
    float applyNokiaTonalColor(float input, float intensity);
    float applyIPhoneTonalColor(float input, float intensity);
    float applySonyEricssonTonalColor(float input, float intensity);

    // NEW: TV Interference methods (Phase 4)
    float applyTVInterference(float input, PhoneType phoneType, float intensity);
    float generateNokiaTVInterference(float input, float intensity);
    float generateIPhoneTVInterference(float input, float intensity);
    float generateSonyTVInterference(float input, float intensity);

    // PHASE 5: Advanced Audio Processing Methods
    float applyCodecSimulation(float input, CodecType codec, float intensity);
    float applyGSMCodec(float input, bool halfRate, float intensity);
    float applyCDMACodec(float input, float intensity);
    float applyAMRCodec(float input, float bitrate, float intensity);
    float applyVoIPArtifacts(float input, float intensity);
    
    float applyPacketLoss(float input, float lossAmount);
    float applyJitter(float input, float jitterAmount);
    
    void applyStereoPositioning(juce::AudioBuffer<float>& buffer, CallPosition position, float intensity);
    void generateBackgroundAmbience(juce::AudioBuffer<float>& buffer, AmbienceType type, float level);
    
    // Frequency conversion functions for discrete choice parameters
    float getLowCutFrequency(int choiceIndex) const;
    float getHighCutFrequency(int choiceIndex) const;
    
    // NEW: Parameter smoothing for professional transitions
    void updateParameterSmoothing();
    
    // Public target values for smooth parameter transitions (accessible from editor)
    float targetDistortion = 0.0f;       // Target distortion value
    float targetNoise = 0.0f;             // Target noise value  
    float targetInterference = 0.0f;      // Target interference value
    float targetCompression = 0.0f;       // Target compression value

private:
    //==============================================================================
    
    // Parameter layout creation
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Audio processing components
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowCutFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highCutFilter;
    
    // Parameter atomic pointers for thread-safe access
    std::atomic<float>* lowCutParam = nullptr;
    std::atomic<float>* highCutParam = nullptr;
    std::atomic<float>* distortionParam = nullptr;
    std::atomic<float>* phoneTypeParam = nullptr;
    std::atomic<float>* noiseLevelParam = nullptr;
    std::atomic<float>* interferenceParam = nullptr;
    std::atomic<float>* compressionParam = nullptr;
    std::atomic<float>* tvInterferenceParam = nullptr;  // NEW: TV interference toggle
    
    // PHASE 5: Advanced Audio Processing Parameters
    std::atomic<float>* codecTypeParam = nullptr;       // Codec simulation type
    std::atomic<float>* packetLossParam = nullptr;       // Packet loss amount
    std::atomic<float>* callPositionParam = nullptr;     // Stereo positioning
    std::atomic<float>* ambienceTypeParam = nullptr;     // Background ambience type
    std::atomic<float>* ambienceLevelParam = nullptr;    // Ambience volume
    
    // NEW: Simplified interference preset parameter
    std::atomic<float>* interferencePresetParam = nullptr;
    
    // Simplified system - no call state management needed
    
    // Ringtone generation
    class RingtoneGenerator
    {
    public:
        void prepareToPlay(double sampleRate);
        void generateRingtone(juce::AudioBuffer<float>& buffer, RingtoneType type);
        void reset();
        
    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        int noteIndex = 0;
        int noteDuration = 0;
        
        // Nokia ringtone note sequences
        struct Note { float frequency; int duration; };
        std::vector<Note> getRingtoneSequence(RingtoneType type);
    };
    
    RingtoneGenerator ringtoneGen;
    
    // UK Network connection sounds
    class NetworkSoundGenerator
    {
    public:
        void prepareToPlay(double sampleRate);
        void generateNetworkSound(juce::AudioBuffer<float>& buffer, NetworkType network, CallState state);
        void reset();
        
    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        int soundTimer = 0;
        
        void generateOrangeSound(juce::AudioBuffer<float>& buffer, CallState state);
        void generateVodafoneSound(juce::AudioBuffer<float>& buffer, CallState state);
        void generateO2Sound(juce::AudioBuffer<float>& buffer, CallState state);
        void generateTMobileSound(juce::AudioBuffer<float>& buffer, CallState state);
    };
    
    NetworkSoundGenerator networkGen;
    
    // Pirate radio static generator
    class PirateRadioGenerator
    {
    public:
        void prepareToPlay(double sampleRate);
        void generatePirateStatic(juce::AudioBuffer<float>& buffer, float intensity);
        void reset();
        
    private:
        double sampleRate = 44100.0;
        juce::Random random;
        float staticPhase = 0.0f;
        
        // Underground radio interference patterns
        void generateStaticBurst(juce::AudioBuffer<float>& buffer, float intensity);
        void generateRadioSweep(juce::AudioBuffer<float>& buffer, float intensity);
        void generateDigitalGlitch(juce::AudioBuffer<float>& buffer, float intensity);
    };
    
    PirateRadioGenerator pirateGen;
    
    // Enhanced noise generators
    void generateNokiaGSMNoise(juce::AudioBuffer<float>& buffer, float level);
    void generateIPhoneDigitalNoise(juce::AudioBuffer<float>& buffer, float level);
    void generateSonyEricssonAnalogNoise(juce::AudioBuffer<float>& buffer, float level);
    
    // Call quality degradation
    void applyCallQualityDegradation(juce::AudioBuffer<float>& buffer, float quality);
    
    // Audio level monitoring
    std::atomic<float> currentAudioLevel{0.0f};
    
    // Sample rate
    double currentSampleRate = 44100.0;
    
    // Random number generator for noise
    juce::Random random;
    
    // GSM interference simulation (REPLACED WITH DYNAMIC SIGNAL STRENGTH)
    float gsmPhase = 0.0f;
    float gsmBurstTimer = 0.0f;
    bool gsmBurstActive = false;
    int gsmBurstCounter = 0;        // NEW: TDMA frame counter for authentic GSM pattern
    float gsmBurstIntensity = 0.0f; // NEW: Dynamic burst intensity (da-da-da-dit pattern)
    float hissPhase = 0.0f;
    
    // GAME-CHANGING: Dynamic Signal Strength System
    float currentSignalStrength = 1.0f;    // Current signal quality (0.0 = no signal, 1.0 = perfect)
    float targetSignalStrength = 1.0f;     // Target signal strength (for smooth transitions)
    float signalChangeTimer = 0.0f;        // Timer for signal strength changes
    float signalSmoothingSpeed = 0.01f;    // How fast signal strength changes
    
    // INTELLIGENT: Voice Activity Detection  
    float voiceActivityLevel = 0.0f;       // Current detected voice activity (0.0-1.0)
    float voiceActivitySmoothing = 0.05f;  // Smoothing for voice detection
    float silenceTimer = 0.0f;             // How long since last voice activity
    float silenceThreshold = 2.0f;         // Seconds of silence before signal degrades
    
    // REALISTIC: Call Quality Dynamics
    float callQualityTimer = 0.0f;         // Timer for realistic call events
    bool isInDropout = false;              // Currently experiencing signal dropout
    float dropoutDuration = 0.0f;          // How long current dropout lasts
    float dropoutRecoveryTimer = 0.0f;     // Timer for signal recovery
    int signalBars = 5;                    // Visual signal strength (1-5 bars)
    
    // AUTHENTIC: Phone-specific signal characteristics  
    float phoneSignalCharacter = 1.0f;     // Each phone type has different signal behavior
    
    // Digital compression simulation
    float compressionGain = 1.0f;
    float compressionThreshold = 0.7f;
    
    // Analog hiss simulation
    float hissLevel = 0.0f;
    
    // NEW: Tonal coloring variables for authentic phone character
    float nokiaDigitalPhase = 0.0f;        // Nokia's digital character
    float iphoneWarmthPhase = 0.0f;        // iPhone's warm digital processing
    float sonyAnalogPhase = 0.0f;          // Sony's analog character
    float tonalColoringIntensity = 0.15f;  // Much more subtle overall tonal coloring
    
    // NEW: TV Interference variables
    float tvInterferencePhase = 0.0f;      // Main TV interference oscillation
    float tvScanlinePhase = 0.0f;          // CRT scanline frequency
    float tvBurstTimer = 0.0f;             // Burst pattern timing
    int tvBurstState = 0;                  // Current burst state
    juce::Random tvRandom;                 // Random for TV static

    // PHASE 5: Advanced Audio Processing Variables
    
    // Codec simulation state
    float codecPhase = 0.0f;              // Phase for codec artifacts
    float codecBuffer[8] = {0};           // Small buffer for codec delays
    int codecBufferIndex = 0;             // Current buffer position
    float quantizationNoise = 0.0f;       // Quantization artifacts
    
    // Packet loss simulation
    float packetLossTimer = 0.0f;         // Timer for packet loss events
    bool packetDropped = false;           // Current packet state
    float reconstructionBuffer[16] = {0}; // Buffer for reconstruction artifacts
    int reconstructionIndex = 0;         // Reconstruction buffer position
    
    // Jitter simulation  
    float jitterDelay[64] = {0};          // Delay line for jitter
    int jitterWriteIndex = 0;             // Write position in jitter buffer
    float jitterPhase = 0.0f;             // Phase for jitter modulation
    
    // Background ambience generation
    float ambiencePhase[4] = {0};         // Multiple phases for complex ambience
    float ambienceLevel[8] = {0};         // Level tracking for ambience layers
    juce::Random ambienceRandom;          // Random for ambience variations
    
    // NEW: Parameter smoothing for professional transitions (private implementation details)
    float currentDistortion = 0.0f;       // Current smoothed distortion
    float currentNoise = 0.0f;            // Current smoothed noise
    float currentInterference = 0.0f;     // Current smoothed interference
    float currentCompression = 0.0f;      // Current smoothed compression
    float smoothingSpeed = 0.002f;        // Smoothing rate (200ms transition)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestAudioProcessor)
}; 