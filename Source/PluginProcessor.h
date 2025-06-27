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
    
    // NEW: Authentic interference preset system
    static const juce::String INTERFERENCE_PRESET_ID;
    
    // Phone-specific interference presets
    enum InterferencePreset
    {
        // Nokia 3310 presets
        Nokia_Clean = 0,
        Nokia_NearTV = 1,           // Classic GSM buzz near TV/speakers
        Nokia_AlarmClock = 2,       // Disrupting alarm clocks/radios
        Nokia_CarRadio = 3,         // In-car interference
        
        // iPhone presets  
        iPhone_Clean = 4,
        iPhone_NearSpeakers = 5,    // Digital buzz near computer speakers
        iPhone_Charging = 6,        // Interference while charging
        iPhone_DataSync = 7,        // During email/data transfer
        
        // Sony Ericsson presets
        Sony_Clean = 8,
        Sony_Underground = 9,       // Signal cutting out in tube/basement
        Sony_WindNoise = 10,        // Background noise pickup in car
        Sony_FlipStatic = 11        // Flip phone opening/closing static
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

    // Phone preset loading
    void loadPhonePreset(PhoneType phoneType);
    
    // NEW: Authentic interference methods (Phase 4)
    float applyAuthenticInterference(float input, PhoneType phoneType, int preset, float noiseLevel, float interferenceLevel);
    float applyNokiaInterference(float input, int preset, float noiseLevel, float interferenceLevel);
    float applyIPhoneInterference(float input, int preset, float noiseLevel, float interferenceLevel);
    float applySonyEricssonInterference(float input, int preset, float noiseLevel, float interferenceLevel);
    float applyPhoneDistortion(float input, PhoneType phoneType, float amount);
    float applyPhoneCompression(float input, PhoneType phoneType, float amount);

    // NEW: Phone-specific tonal coloring methods
    float applyPhoneTonalColor(float input, PhoneType phoneType, float intensity);
    float applyNokiaTonalColor(float input, float intensity);
    float applyIPhoneTonalColor(float input, float intensity);
    float applySonyEricssonTonalColor(float input, float intensity);

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
    void applyPacketLoss(juce::AudioBuffer<float>& buffer, float lossAmount);
    void applyJitter(juce::AudioBuffer<float>& buffer, float jitterAmount);
    
    // Audio level monitoring
    std::atomic<float> currentAudioLevel{0.0f};
    
    // Sample rate
    double currentSampleRate = 44100.0;
    
    // Random number generator for noise
    juce::Random random;
    
    // GSM interference simulation
    float gsmPhase = 0.0f;
    float gsmBurstTimer = 0.0f;
    bool gsmBurstActive = false;
    float hissPhase = 0.0f;
    
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestAudioProcessor)
}; 