#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Parameter ID definitions
const juce::String TestAudioProcessor::LOW_CUT_ID = "lowCut";
const juce::String TestAudioProcessor::HIGH_CUT_ID = "highCut";
const juce::String TestAudioProcessor::DISTORTION_ID = "distortion";
const juce::String TestAudioProcessor::PHONE_TYPE_ID = "phoneType";
const juce::String TestAudioProcessor::NOISE_LEVEL_ID = "noiseLevel";
const juce::String TestAudioProcessor::INTERFERENCE_ID = "interference";
const juce::String TestAudioProcessor::COMPRESSION_ID = "compression";
const juce::String TestAudioProcessor::TV_INTERFERENCE_ID = "tvInterference";  // NEW: TV interference toggle

// NEW: Simplified interference preset system
const juce::String TestAudioProcessor::INTERFERENCE_PRESET_ID = "interferencePreset";

// PHASE 5: Advanced Audio Processing Parameter IDs
const juce::String TestAudioProcessor::CODEC_TYPE_ID = "codecType";
const juce::String TestAudioProcessor::PACKET_LOSS_ID = "packetLoss";  
const juce::String TestAudioProcessor::CALL_POSITION_ID = "callPosition";
const juce::String TestAudioProcessor::AMBIENCE_TYPE_ID = "ambienceType";
const juce::String TestAudioProcessor::AMBIENCE_LEVEL_ID = "ambienceLevel";

//==============================================================================
TestAudioProcessor::TestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameterLayout())
#else
    : apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    // Get atomic parameter pointers for thread-safe access
    lowCutParam = apvts.getRawParameterValue(LOW_CUT_ID);
    highCutParam = apvts.getRawParameterValue(HIGH_CUT_ID);
    distortionParam = apvts.getRawParameterValue(DISTORTION_ID);
    phoneTypeParam = apvts.getRawParameterValue(PHONE_TYPE_ID);
    noiseLevelParam = apvts.getRawParameterValue(NOISE_LEVEL_ID);
    interferenceParam = apvts.getRawParameterValue(INTERFERENCE_ID);
    compressionParam = apvts.getRawParameterValue(COMPRESSION_ID);
    tvInterferenceParam = apvts.getRawParameterValue(TV_INTERFERENCE_ID);  // NEW: TV interference toggle
    
    // NEW: Simplified interference preset parameter
    interferencePresetParam = apvts.getRawParameterValue(INTERFERENCE_PRESET_ID);
    
    // PHASE 5: Advanced Audio Processing Parameter Pointers
    codecTypeParam = apvts.getRawParameterValue(CODEC_TYPE_ID);
    packetLossParam = apvts.getRawParameterValue(PACKET_LOSS_ID);
    callPositionParam = apvts.getRawParameterValue(CALL_POSITION_ID);
    ambienceTypeParam = apvts.getRawParameterValue(AMBIENCE_TYPE_ID);
    ambienceLevelParam = apvts.getRawParameterValue(AMBIENCE_LEVEL_ID);
    
    // Initialize noise buffer

    compressionGain = 1.0f;
    hissLevel = 0.0f;
    hissPhase = 0.0f;
    
    // Initialize tonal coloring phases for authentic phone character
    nokiaDigitalPhase = 0.0f;
    iphoneWarmthPhase = 0.0f;
    sonyAnalogPhase = 0.0f;
    
    // Initialize TV interference state
    tvInterferencePhase = 0.0f;
    tvScanlinePhase = 0.0f;
    tvBurstTimer = 0.0f;
    tvBurstState = 0;
}

TestAudioProcessor::~TestAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TestAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Low Cut Frequency - Discrete preset positions (using AudioParameterFloat with discrete steps)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        LOW_CUT_ID, "Low Cut", 
        juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            int index = static_cast<int>(value);
            switch (index) {
                case 0: return juce::String("Off");
                case 1: return juce::String("180Hz");
                case 2: return juce::String("250Hz");
                case 3: return juce::String("300Hz");
                case 4: return juce::String("400Hz");
                default: return juce::String("Off");
            }
        }
    ));
    
    // High Cut Frequency - Discrete preset positions (using AudioParameterFloat with discrete steps)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        HIGH_CUT_ID, "High Cut",
        juce::NormalisableRange<float>(0.0f, 4.0f, 1.0f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { 
            int index = static_cast<int>(value);
            switch (index) {
                case 0: return juce::String("Off");
                case 1: return juce::String("2.8kHz");
                case 2: return juce::String("3.2kHz");
                case 3: return juce::String("3.4kHz");
                case 4: return juce::String("7kHz");
                default: return juce::String("Off");
            }
        }
    ));
    
    // Distortion Amount (0% - 100%)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        DISTORTION_ID, "Distortion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + " %"; }
    ));
    
    // Phone Type (0=Nokia, 1=iPhone, 2=SonyEricsson)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        PHONE_TYPE_ID, "Phone Type",
        juce::NormalisableRange<float>(0.0f, 2.0f, 1.0f), 0.0f
    ));
    
    // Background Noise Level (0% - 100%)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        NOISE_LEVEL_ID, "Noise Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + " %"; }
    ));
    
    // Interference/Artifacts (0% - 100%)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        INTERFERENCE_ID, "Interference",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + " %"; }
    ));
    
    // Compression/Limiting (0% - 100%)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        COMPRESSION_ID, "Compression",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + " %"; }
    ));
    
    // TV Interference (On/Off toggle)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        TV_INTERFERENCE_ID, "TV Interference",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return value > 0.5f ? "ON" : "OFF"; }
    ));
    
    // REMOVED: Old interference preset parameter (replaced with Signal Quality Choice)
    
    // PHASE 5: Advanced Audio Processing Parameters
    
    // Codec Type (0-6: GSM Full/Half, CDMA, AMR 4.75/12.2, VoIP, Digital)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        CODEC_TYPE_ID, "Codec Type",
        juce::NormalisableRange<float>(0.0f, 6.0f, 1.0f), 0.0f
    ));
    
    // Packet Loss (0% - 50%)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        PACKET_LOSS_ID, "Packet Loss",
        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + " %"; }
    ));
    
    // Call Position (0-6: Center, Left/Right Ear, Speaker Near/Far, Bluetooth L/R)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        CALL_POSITION_ID, "Call Position", 
        juce::NormalisableRange<float>(0.0f, 6.0f, 1.0f), 0.0f
    ));
    
    // Ambience Type (0-7: Silent, Café, Car, Street, Tube, Office, Train, Airport)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        AMBIENCE_TYPE_ID, "Ambience Type",
        juce::NormalisableRange<float>(0.0f, 7.0f, 1.0f), 0.0f
    ));
    
    // Ambience Level (0% - 100%)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        AMBIENCE_LEVEL_ID, "Ambience Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + " %"; }
    ));
    
    // Dynamic Signal Quality parameter (GAME-CHANGING!)
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        INTERFERENCE_PRESET_ID,
        "Signal Quality",
        juce::StringArray{"Perfect (5 bars)", "Good (4 bars)", "Fair (3 bars)", "Poor (2 bars)", "Breaking Up (1 bar)", "Auto Dynamic"},
        0 // FIX: Default to Perfect Signal (5 bars) instead of Auto Dynamic
    ));
    
    return { parameters.begin(), parameters.end() };
}

//==============================================================================
const juce::String TestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TestAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TestAudioProcessor::getNumPrograms()
{
    return 3; // Nokia, iPhone, Sony Ericsson
}

int TestAudioProcessor::getCurrentProgram()
{
    return static_cast<int>(*phoneTypeParam);
}

void TestAudioProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < 3)
    {
        *phoneTypeParam = static_cast<float>(index);
        loadPhonePreset(static_cast<PhoneType>(index));
    }
}

const juce::String TestAudioProcessor::getProgramName (int index)
{
    switch (index)
    {
        case 0: return "Nokia 3310";
        case 1: return "iPhone";
        case 2: return "Sony Ericsson";
        default: return "Unknown";
    }
}

void TestAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    // Program names are fixed for phone types
}

//==============================================================================
void TestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Prepare DSP components
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);
    
    // Reset effect states
    gsmPhase = 0.0f;
    gsmBurstTimer = 0;
    compressionGain = 1.0f;
    hissLevel = 0.0f;
    hissPhase = 0.0f;
    
    // Initialize tonal coloring phases
    nokiaDigitalPhase = 0.0f;
    iphoneWarmthPhase = 0.0f;
    sonyAnalogPhase = 0.0f;
    
    // Initialize TV interference state
    tvInterferencePhase = 0.0f;
    tvScanlinePhase = 0.0f;
    tvBurstTimer = 0.0f;
    tvBurstState = 0;
}

void TestAudioProcessor::releaseResources()
{
    lowCutFilter.reset();
    highCutFilter.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TestAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Update parameter smoothing for professional transitions
    updateParameterSmoothing();

    // Update filter coefficients - Convert discrete indices to actual frequencies
    auto lowCutIndex = static_cast<int>(lowCutParam->load());
    auto highCutIndex = static_cast<int>(highCutParam->load());
    
    // Create audio block for DSP processing
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Apply filters only if not "Off" (index 0)
    if (lowCutIndex > 0)
    {
        // Convert index to frequency
        float lowCutFreq = getLowCutFrequency(lowCutIndex);
        // High-pass filter (low cut)
        *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            currentSampleRate, lowCutFreq, 0.707f);
        lowCutFilter.process(context);
    }
    
    if (highCutIndex > 0)
    {
        // Convert index to frequency
        float highCutFreq = getHighCutFrequency(highCutIndex);
        // Low-pass filter (high cut)  
        *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            currentSampleRate, highCutFreq, 0.707f);
        highCutFilter.process(context);
    }
    
    // Get current parameter values
    auto phoneType = static_cast<PhoneType>(static_cast<int>(phoneTypeParam->load()));
    auto distortionAmount = distortionParam->load();
    auto noiseLevel = noiseLevelParam->load();
    auto interferenceLevel = interferenceParam->load();
    auto compressionLevel = compressionParam->load();
    auto interferencePreset = static_cast<int>(interferencePresetParam->load());
    auto tvInterferenceLevel = tvInterferenceParam->load();  // NEW: TV interference toggle
    
    // PHASE 5: Advanced Audio Processing Parameters
    auto codecType = static_cast<CodecType>(static_cast<int>(codecTypeParam->load()));
    auto packetLoss = packetLossParam->load();
    auto callPosition = static_cast<CallPosition>(static_cast<int>(callPositionParam->load()));
    auto ambienceType = static_cast<AmbienceType>(static_cast<int>(ambienceTypeParam->load()));
    auto ambienceLevel = ambienceLevelParam->load();
    
    // Generate background ambience first (affects entire buffer)
    if (ambienceLevel > 0.0f && ambienceType != Silent)
    {
        generateBackgroundAmbience(buffer, ambienceType, ambienceLevel);
    }
    
    // Apply phone-specific processing with authentic interference
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto input = channelData[sample];
            
            // Apply phone-specific tonal coloring FIRST (authentic sound character)
            // TEMPORARILY DISABLED: Testing if this contributes to buzzy sound
            // input = applyPhoneTonalColor(input, phoneType, tonalColoringIntensity);
            
            // GAME-CHANGING: Dynamic Signal Strength System (replaces old interference)
            detectVoiceActivity(std::abs(input));
            
            // Update signal strength based on voice activity and phone type  
            updateSignalStrength(std::abs(input));
            
            // Apply intelligent signal quality simulation
            input = applySignalQuality(input, phoneType, static_cast<SignalQuality>(interferencePreset));
            
            // Apply smart compression that adapts to signal strength
            if (compressionLevel > 0.0f)
            {
                input = applySmartCompression(input, currentSignalStrength);
            }
            
            // NEW: Apply TV interference if enabled (THE FEATURE YOU'VE BEEN WAITING FOR!)
            if (tvInterferenceLevel > 0.5f)
            {
                input = applyTVInterference(input, phoneType, tvInterferenceLevel);
            }
            
            // Apply distortion
            if (distortionAmount > 0.0f)
            {
                input = applyPhoneDistortion(input, phoneType, distortionAmount);
            }
            
            // TEMPORARILY DISABLED: Too many processing layers making it sound robotic
            /*
            // PHASE 5: Advanced Audio Processing
            
            // Apply codec simulation for authentic digital compression
            if (codecType != GSM_FullRate || packetLoss > 0.0f)
            {
                input = applyCodecSimulation(input, codecType, 0.8f);
            }
            
            // Apply packet loss and jitter
            if (packetLoss > 0.0f)
            {
                input = applyPacketLoss(input, packetLoss);
                input = applyJitter(input, packetLoss * 0.5f); // Jitter related to packet loss
            }
            */
            
            channelData[sample] = input;
        }
    }
    
    // Apply stereo positioning (affects entire buffer after processing)
    if (callPosition != Center)
    {
        applyStereoPositioning(buffer, callPosition, 1.0f);
    }
}

//==============================================================================
// AUTHENTIC INTERFERENCE METHODS (REPLACED BY DYNAMIC SIGNAL STRENGTH - COMMENTED OUT)

/*
float TestAudioProcessor::applyAuthenticInterference(float input, PhoneType phoneType, int preset, float noiseLevel, float interferenceLevel)
{
    // Apply phone-specific authentic interference based on preset
    switch (phoneType)
    {
        case Nokia:
            return applyNokiaInterference(input, preset, noiseLevel, interferenceLevel);
        case iPhone:
            return applyIPhoneInterference(input, preset, noiseLevel, interferenceLevel);
        case SonyEricsson:
            return applySonyEricssonInterference(input, preset, noiseLevel, interferenceLevel);
        default:
            return input;
    }
}

// OLD INTERFERENCE METHODS - COMMENTED OUT AND REPLACED BY DYNAMIC SIGNAL STRENGTH
/*
float TestAudioProcessor::applyNokiaInterference(float input, int preset, float noiseLevel, float interferenceLevel)
{
    // OLD INTERFERENCE CODE - REPLACED BY DYNAMIC SIGNAL STRENGTH
    return input;
}

float TestAudioProcessor::applyIPhoneInterference(float input, int preset, float noiseLevel, float interferenceLevel)
{
    // OLD INTERFERENCE CODE - REPLACED BY DYNAMIC SIGNAL STRENGTH
    return input;
}

float TestAudioProcessor::applySonyEricssonInterference(float input, int preset, float noiseLevel, float interferenceLevel)
{
    // OLD INTERFERENCE CODE - REPLACED BY DYNAMIC SIGNAL STRENGTH
    return input;
}
*/

float TestAudioProcessor::applyPhoneDistortion(float input, PhoneType phoneType, float amount)
{
    switch (phoneType)
    {
        case Nokia:
        {
            // Nokia: MUCH GENTLER digital characteristics (less buzzy!)
            float gain = 1.0f + amount * 1.2f; // FIXED: Less aggressive gain (was 2.5f)
            float amplified = input * gain;
            
            // Softer digital clipping (less harsh)
            float clipped = juce::jlimit(-0.9f, 0.9f, amplified); // FIXED: Softer clipping ceiling
            
            // MUCH LESS quantization noise (was too buzzy!)
            if (amount > 0.3f) // FIXED: Only add at higher settings
            {
                float quantizationNoise = (random.nextFloat() * 2.0f - 1.0f) * 0.001f * amount; // FIXED: Much quieter
                clipped += quantizationNoise;
            }
            
            // MUCH SUBTLER digital artifacts
            float digitalBite = std::sin(clipped * 8.0f) * 0.02f * amount; // FIXED: Much gentler (was 15.0f freq, 0.08f level)
            
            return clipped + digitalBite;
        }
        
        case iPhone:
        {
            // iPhone: Smooth soft saturation with high-quality digital processing
            float gain = 1.0f + amount * 1.8f;
            float amplified = input * gain;
            
            // Smooth soft clipping (high-quality DAC with oversampling)
            float softClipped = std::tanh(amplified) * 0.85f;
            
            // Add subtle digital warmth (high-quality processing artifacts)
            if (amount > 0.05f)
            {
                float digitalWarmth = std::sin(softClipped * 8.0f) * 0.02f * amount;
                softClipped += digitalWarmth;
            }
            
            // Very subtle high-frequency roll-off (anti-aliasing filtering)
            return softClipped * (1.0f - amount * 0.1f);
        }
        
        case SonyEricsson:
        {
            // Sony Ericsson: Analog tube-like warmth with vintage character
            float gain = 1.0f + amount * 3.5f;
            float amplified = input * gain;
            
            // Asymmetric analog saturation (vintage op-amp characteristics)
            float analogSat;
            if (amplified >= 0.0f)
            {
                analogSat = std::atan(amplified * 1.2f) * 0.8f; // Positive saturation
            }
            else
            {
                analogSat = std::atan(amplified * 0.9f) * 0.85f; // Slightly different negative saturation
            }
            
            // Add analog harmonic content (tube-like even harmonics)
            float analogHarmonics = std::sin(analogSat * 6.0f) * 0.12f * amount;
            
            // Vintage component aging (slight frequency-dependent distortion)
            float agingEffect = analogSat * (1.0f + std::sin(analogSat * 25.0f) * 0.05f * amount);
            
            return agingEffect + analogHarmonics;
        }
        
        default:
            return input;
    }
}

float TestAudioProcessor::applyPhoneCompression(float input, PhoneType phoneType, float amount)
{
    float threshold = 0.3f - (amount * 0.2f);
    float ratio = 2.0f + (amount * 6.0f);
    
    float absInput = std::abs(input);
    if (absInput > threshold)
    {
        float excess = absInput - threshold;
        float compressedExcess = excess / ratio;
        float sign = (input >= 0.0f) ? 1.0f : -1.0f;
        return sign * (threshold + compressedExcess);
    }
    
    return input;
}

//==============================================================================
// PHONE-SPECIFIC TONAL COLORING (Phase 4: Authentic Sound Colors)

float TestAudioProcessor::applyPhoneTonalColor(float input, PhoneType phoneType, float intensity)
{
    switch (phoneType)
    {
        case Nokia:
            return applyNokiaTonalColor(input, intensity);
        case iPhone:
            return applyIPhoneTonalColor(input, intensity);  
        case SonyEricsson:
            return applySonyEricssonTonalColor(input, intensity);
        default:
            return input;
    }
}

float TestAudioProcessor::applyNokiaTonalColor(float input, float intensity)
{
    // Nokia 3310: Digital bite with mid-range punch
    // Characteristic: Aggressive digital compression with 800Hz-2kHz emphasis
    
    nokiaDigitalPhase += 0.01f;
    
    // Add subtle digital "bite" - much more subtle
    float digitalBite = std::sin(nokiaDigitalPhase * 3.7f) * 0.015f * intensity; // Reduced from 0.08f
    
    // Mid-range emphasis (classic Nokia voice clarity) - more subtle
    float midBoost = input * (1.0f + intensity * 0.1f); // Reduced from 0.3f
    
    // Slight digital saturation - much gentler
    float saturated = std::tanh(midBoost * (1.0f + intensity * 0.2f)) * 0.95f; // Reduced from 0.5f
    
    return saturated + digitalBite;
}

float TestAudioProcessor::applyIPhoneTonalColor(float input, float intensity)
{
    // iPhone: Warm digital clarity with smooth compression
    // Characteristic: Clean, warm digital processing with subtle harmonics
    
    iphoneWarmthPhase += 0.008f;
    
    // Add warm digital harmonics - more subtle
    float warmth = std::sin(iphoneWarmthPhase * 2.1f) * 0.01f * intensity; // Reduced from 0.04f
    
    // Smooth digital processing - less harsh than Nokia
    float processed = input * (1.0f + intensity * 0.08f); // Reduced from 0.2f
    
    // Soft saturation for warmth - much gentler
    float warmed = std::atan(processed * (1.0f + intensity * 0.1f)) * 0.9f; // Reduced from 0.3f
    
    return warmed + warmth;
}

float TestAudioProcessor::applySonyEricssonTonalColor(float input, float intensity)
{
    // Sony Ericsson: Analog grit with tape-like saturation
    // Characteristic: Warm analog distortion with slight wow/flutter
    
    sonyAnalogPhase += 0.012f + (random.nextFloat() * 0.001f); // Reduced flutter
    
    // Add analog grit and warmth - much more subtle
    float analogGrit = std::sin(sonyAnalogPhase * 1.8f) * 0.02f * intensity; // Reduced from 0.12f
    
    // Analog tape saturation - gentler
    float analogSat = std::atan(input * (1.0f + intensity * 0.15f)) * 0.85f; // Reduced from higher values
    
    return analogSat + analogGrit;
}

//==============================================================================
void TestAudioProcessor::loadPhonePreset(PhoneType phoneType)
{
    switch (phoneType)
    {
        case Nokia:
            // Nokia 3310 - Classic GSM characteristics
            apvts.getParameter(LOW_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(LOW_CUT_ID)->convertTo0to1(400.0f));
            apvts.getParameter(HIGH_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(HIGH_CUT_ID)->convertTo0to1(4000.0f));
            
            // FIX: Immediately update ALL parameters (no carryover)
            apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(0.2f);    // 20% - Small speaker distortion
            apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(0.05f);  // 5% - GSM codec noise (reduced from 8% to fix stuck issue)
            apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(0.15f); // 15% - GSM RF interference
            apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(0.7f);   // 70% - MUCH stronger GSM compression
            break;
            
        case iPhone:
            // iPhone - Modern smartphone with clean digital processing
            apvts.getParameter(LOW_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(LOW_CUT_ID)->convertTo0to1(300.0f));
            apvts.getParameter(HIGH_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(HIGH_CUT_ID)->convertTo0to1(8000.0f));
            
            // FIX: Immediately update ALL parameters (no carryover)
            apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(0.03f);   // 3% - Minimal digital distortion
            apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(0.02f);  // 2% - Advanced noise cancellation
            apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(0.04f); // 4% - Digital shielding  
            apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(0.35f);  // 35% - Modern codec compression
            break;
            
        case SonyEricsson:
            // Sony Ericsson - Vintage flip phone with analog circuits
            apvts.getParameter(LOW_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(LOW_CUT_ID)->convertTo0to1(250.0f));
            apvts.getParameter(HIGH_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(HIGH_CUT_ID)->convertTo0to1(2500.0f));
            
            // FIX: Immediately update ALL parameters (no carryover)
            apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(0.35f);   // 35% - Tiny speaker, analog circuits
            apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(0.12f);  // 12% - Poor electrical shielding
            apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(0.18f); // 18% - Very susceptible to RF
            apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(0.8f);   // 80% - VERY aggressive analog compression
            break;
    }
    
    // Update phone type parameter
    apvts.getParameter(PHONE_TYPE_ID)->setValueNotifyingHost(static_cast<float>(phoneType) / 2.0f);
    
    // FIX: Remove target value system that was causing carryover
    // All parameters now update immediately via setValueNotifyingHost
}

//==============================================================================
bool TestAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TestAudioProcessor::createEditor()
{
    return new TestAudioProcessorEditor (*this);
}

//==============================================================================
void TestAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// TV INTERFERENCE METHODS (Phase 4: The TV Interference You've Been Waiting For!)

float TestAudioProcessor::applyTVInterference(float input, PhoneType phoneType, float intensity)
{
    if (intensity < 0.5f) return input; // TV interference is OFF
    
    switch (phoneType)
    {
        case Nokia:
            return generateNokiaTVInterference(input, intensity);
        case iPhone:
            return generateIPhoneTVInterference(input, intensity);
        case SonyEricsson:
            return generateSonyTVInterference(input, intensity);
        default:
            return input;
    }
}

float TestAudioProcessor::generateNokiaTVInterference(float input, float intensity)
{
    // Nokia 3310 near CRT TV: Classic GSM interference with TV scanline buzz
    // FIX: Much safer levels to prevent speaker damage
    
    // TV scanline frequency reduced to safer range (1kHz instead of 15.625 kHz)
    tvScanlinePhase += 2.0f * juce::MathConstants<float>::pi * 1000.0f / static_cast<float>(currentSampleRate);
    
    // GSM burst pattern interfering with TV
    tvBurstTimer += 1.0f / static_cast<float>(currentSampleRate);
    if (tvBurstTimer >= 0.0046f) // GSM burst every 4.6ms
    {
        tvBurstTimer = 0.0f;
        tvBurstState = (tvBurstState + 1) % 4; // 4-state burst pattern
    }
    
    // Generate authentic Nokia TV buzz - MUCH SAFER LEVELS
    float tvBuzz = 0.0f;
    if (tvBurstState == 0 || tvBurstState == 2) // Active burst states
    {
        // 217Hz GSM carrier with TV scanline modulation
        tvInterferencePhase += 2.0f * juce::MathConstants<float>::pi * 217.0f / static_cast<float>(currentSampleRate);
        float gsmCarrier = std::sin(tvInterferencePhase);
        float scanlineModulation = std::sin(tvScanlinePhase) * 0.1f; // Reduced from 0.3f
        
        // FIX: Much safer amplitude (0.03f instead of 0.15f)
        tvBuzz = gsmCarrier * (0.8f + scanlineModulation) * 0.03f * intensity;
        
        // FIX: Much quieter digital clicking (0.02f instead of 0.2f)
        if (tvRandom.nextFloat() > 0.98f) // Less frequent pops
        {
            tvBuzz += (tvRandom.nextFloat() * 2.0f - 1.0f) * 0.02f * intensity;
        }
    }
    
    return input + tvBuzz;
}

float TestAudioProcessor::generateIPhoneTVInterference(float input, float intensity)
{
    // iPhone near modern LCD/LED TV: Digital switching noise and power supply whine
    // FIX: Much safer levels
    
    // LCD refresh rate interference (60Hz and harmonics)
    tvScanlinePhase += 2.0f * juce::MathConstants<float>::pi * 60.0f / static_cast<float>(currentSampleRate);
    tvInterferencePhase += 2.0f * juce::MathConstants<float>::pi * 120.0f / static_cast<float>(currentSampleRate);
    
    // Digital switching noise from iPhone's power management
    float switchingNoise = std::sin(tvInterferencePhase) * 0.03f; // Reduced from 0.1f
    float refreshNoise = std::sin(tvScanlinePhase) * 0.02f;       // Reduced from 0.05f
    
    // Combine with subtle digital artifacts - MUCH SAFER
    float digitalBuzz = (switchingNoise + refreshNoise) * intensity * 0.02f; // Reduced from 0.08f
    
    // FIX: Much quieter occasional digital pops
    if (tvRandom.nextFloat() > 0.998f) // Much less frequent
    {
        digitalBuzz += (tvRandom.nextFloat() * 2.0f - 1.0f) * 0.01f * intensity; // Reduced from 0.1f
    }
    
    return input + digitalBuzz;
}

float TestAudioProcessor::generateSonyTVInterference(float input, float intensity)
{
    // Sony Ericsson near old CRT TV: Analog interference with magnetic field buzz
    // FIX: Much safer levels
    
    // CRT horizontal sweep frequency reduced to safer range (500Hz instead of 15.625 kHz)
    float flutterAmount = tvRandom.nextFloat() * 0.01f - 0.005f; // Reduced flutter
    tvScanlinePhase += 2.0f * juce::MathConstants<float>::pi * (500.0f + flutterAmount * 50.0f) / static_cast<float>(currentSampleRate);
    
    // Magnetic field interference from CRT deflection coils
    tvInterferencePhase += 2.0f * juce::MathConstants<float>::pi * 50.0f / static_cast<float>(currentSampleRate); // 50Hz mains hum
    
    // Generate analog TV interference - MUCH SAFER LEVELS
    float magneticBuzz = std::sin(tvScanlinePhase) * 0.03f; // Reduced from 0.12f
    float mainsHum = std::sin(tvInterferencePhase) * 0.02f; // Reduced from 0.06f
    
    // Add analog static and crackle - QUIETER
    float analogStatic = (tvRandom.nextFloat() * 2.0f - 1.0f) * 0.01f; // Reduced from 0.03f
    
    // Combine all analog interference - MUCH SAFER
    float analogInterference = (magneticBuzz + mainsHum + analogStatic) * intensity * 0.03f; // Reduced from 0.1f
    
    // FIX: Much quieter occasional analog pops
    if (tvRandom.nextFloat() > 0.995f) // Less frequent
    {
        analogInterference += (tvRandom.nextFloat() * 2.0f - 1.0f) * 0.03f * intensity; // Reduced from 0.15f
    }
    
    return input + analogInterference;
}

//==============================================================================
// PHASE 5: ADVANCED AUDIO PROCESSING METHODS

// Codec Simulation Methods
float TestAudioProcessor::applyCodecSimulation(float input, CodecType codec, float intensity)
{
    switch (codec)
    {
        case GSM_FullRate:
            return applyGSMCodec(input, false, intensity);
        case GSM_HalfRate:
            return applyGSMCodec(input, true, intensity);
        case CDMA_QCELP:
            return applyCDMACodec(input, intensity);
        case AMR_4_75:
            return applyAMRCodec(input, 4.75f, intensity);
        case AMR_12_2:
            return applyAMRCodec(input, 12.2f, intensity);
        case Early_VoIP:
            return applyVoIPArtifacts(input, intensity);
        case Digital_Artifact:
            return applyVoIPArtifacts(input, intensity * 1.5f); // More extreme
        default:
            return input;
    }
}

float TestAudioProcessor::applyGSMCodec(float input, bool halfRate, float intensity)
{
    // GSM codec simulation: aggressive quantization and temporal artifacts
    
    // Quantization (8-bit to 13-bit depending on rate)
    int quantLevels = halfRate ? 256 : 8192; // Half-rate = 8-bit, Full-rate = 13-bit
    float quantized = std::round(input * quantLevels) / quantLevels;
    
    // Buffer for codec delay and artifacts
    codecBuffer[codecBufferIndex] = quantized;
    codecBufferIndex = (codecBufferIndex + 1) % 8;
    
    // GSM frame artifacts (20ms frames)
    codecPhase += 1.0f / static_cast<float>(currentSampleRate);
    if (codecPhase >= 0.02f) // 20ms frame
    {
        codecPhase = 0.0f;
        quantizationNoise = (random.nextFloat() * 2.0f - 1.0f) * 0.02f;
    }
    
    // Apply quantization noise and codec delay
    float delayed = codecBuffer[(codecBufferIndex + 4) % 8]; // 4-sample delay
    return juce::jlimit(-1.0f, 1.0f, delayed + quantizationNoise * intensity);
}

float TestAudioProcessor::applyCDMACodec(float input, float intensity)  
{
    // CDMA QCELP codec: variable rate with silence detection
    
    // Simulate voice activity detection
    float threshold = 0.05f;
    bool voiceActive = std::abs(input) > threshold;
    
    if (!voiceActive)
    {
        // Comfort noise generation during silence
        return (random.nextFloat() * 2.0f - 1.0f) * 0.01f * intensity;
    }
    
    // QCELP quantization (more aggressive than GSM)
    int quantLevels = 512; // 9-bit equivalent
    float quantized = std::round(input * quantLevels) / quantLevels;
    
    // Add CDMA-specific digital artifacts
    codecPhase += 2.0f * juce::MathConstants<float>::pi * 8000.0f / static_cast<float>(currentSampleRate);
    float digitalNoise = std::sin(codecPhase) * 0.01f * intensity;
    
    return juce::jlimit(-1.0f, 1.0f, quantized + digitalNoise);
}

float TestAudioProcessor::applyAMRCodec(float input, float bitrate, float intensity)
{
    // AMR codec simulation based on bitrate
    
    // Lower bitrate = more aggressive compression
    float compressionFactor = 12.2f / bitrate; // Scale based on max AMR rate
    
    // Quantization levels based on bitrate
    int quantLevels = static_cast<int>(8192 / compressionFactor);
    float quantized = std::round(input * quantLevels) / quantLevels;
    
    // AMR artifacts: spectral shaping and noise
    codecPhase += 0.1f;
    float spectralNoise = std::sin(codecPhase * 2.3f) * 0.005f * compressionFactor * intensity;
    
    // Frame-based artifacts (20ms AMR frames)
    if (static_cast<int>(codecPhase * 50) % 40 == 0) // Every 20ms at 50Hz update
    {
        quantizationNoise = (random.nextFloat() * 2.0f - 1.0f) * 0.01f * compressionFactor;
    }
    
    return juce::jlimit(-1.0f, 1.0f, quantized + spectralNoise + quantizationNoise * intensity);
}

float TestAudioProcessor::applyVoIPArtifacts(float input, float intensity)
{
    // Early VoIP artifacts: packet reconstruction, echo cancellation artifacts
    
    // Simulate packet reconstruction errors
    if (random.nextFloat() > 0.995f)
    {
        // Packet reconstruction glitch
        return reconstructionBuffer[reconstructionIndex] * 0.7f;
    }
    
    // Store in reconstruction buffer
    reconstructionBuffer[reconstructionIndex] = input;
    reconstructionIndex = (reconstructionIndex + 1) % 16;
    
    // Echo cancellation artifacts
    float echoArtifact = reconstructionBuffer[(reconstructionIndex + 8) % 16] * 0.05f;
    
    // Internet jitter simulation
    codecPhase += (1.0f + random.nextFloat() * 0.2f) / static_cast<float>(currentSampleRate);
    float jitterNoise = std::sin(codecPhase * 4000.0f) * 0.02f * intensity;
    
    return juce::jlimit(-1.0f, 1.0f, input + echoArtifact + jitterNoise);
}

// Packet Loss and Jitter Methods
float TestAudioProcessor::applyPacketLoss(float input, float lossAmount)
{
    // Simulate packet loss with realistic reconstruction
    
    packetLossTimer += 1.0f / static_cast<float>(currentSampleRate);
    
    // Packet loss probability based on lossAmount
    if (packetLossTimer >= 0.02f) // Check every 20ms (packet boundary)
    {
        packetLossTimer = 0.0f;
        packetDropped = random.nextFloat() < lossAmount;
    }
    
    if (packetDropped)
    {
        // Simulate packet reconstruction: linear interpolation or repetition
        float lastGood = reconstructionBuffer[(reconstructionIndex + 15) % 16];
        float nextEstimate = lastGood * 0.8f; // Decay estimate
        
        return nextEstimate;
    }
    
    // Store good packet
    reconstructionBuffer[reconstructionIndex] = input;
    reconstructionIndex = (reconstructionIndex + 1) % 16;
    
    return input;
}

float TestAudioProcessor::applyJitter(float input, float jitterAmount)
{
    // Simulate network jitter with variable delay
    
    // Update jitter phase for modulation
    jitterPhase += 2.0f * juce::MathConstants<float>::pi * 0.5f / static_cast<float>(currentSampleRate);
    
    // Calculate variable delay (0-63 samples based on jitter amount)
    int maxDelay = static_cast<int>(jitterAmount * 63.0f);
    int currentDelay = static_cast<int>((std::sin(jitterPhase) + 1.0f) * 0.5f * maxDelay);
    
    // Store input in delay line
    jitterDelay[jitterWriteIndex] = input;
    jitterWriteIndex = (jitterWriteIndex + 1) % 64;
    
    // Read from delayed position
    int readIndex = (jitterWriteIndex - currentDelay - 1 + 64) % 64;
    
    return jitterDelay[readIndex];
}

// Stereo Positioning Methods
void TestAudioProcessor::applyStereoPositioning(juce::AudioBuffer<float>& buffer, CallPosition position, float intensity)
{
    if (buffer.getNumChannels() < 2) return; // Skip if not stereo
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float left = leftChannel[sample];
        float right = rightChannel[sample];
        float mono = (left + right) * 0.5f;
        
        switch (position)
        {
            case LeftEar:
                // Phone held to left ear
                leftChannel[sample] = mono;
                rightChannel[sample] = mono * 0.2f; // Slight bleed to right
                break;
                
            case RightEar:
                // Phone held to right ear  
                rightChannel[sample] = mono;
                leftChannel[sample] = mono * 0.2f; // Slight bleed to left
                break;
                
            case Speaker_Near:
                // Speakerphone close - slight stereo widening
                leftChannel[sample] = left * 1.1f - right * 0.1f;
                rightChannel[sample] = right * 1.1f - left * 0.1f;
                break;
                
            case Speaker_Far:
                // Speakerphone distant - more ambient
                leftChannel[sample] = mono * 0.8f + left * 0.4f;
                rightChannel[sample] = mono * 0.8f + right * 0.4f;
                break;
                
            case Bluetooth_Left:
                // Bluetooth earpiece left
                leftChannel[sample] = mono * 0.9f;
                rightChannel[sample] = mono * 0.1f;
                break;
                
            case Bluetooth_Right:
                // Bluetooth earpiece right
                rightChannel[sample] = mono * 0.9f;
                leftChannel[sample] = mono * 0.1f;
                break;
                
            default: // Center
                break;
        }
    }
}

// Background Ambience Methods  
void TestAudioProcessor::generateBackgroundAmbience(juce::AudioBuffer<float>& buffer, AmbienceType type, float level)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float ambience = 0.0f;
            
            switch (type)
            {
                case Cafe_Busy:
                    // Busy café: chatter, dishes, coffee machine
                    ambiencePhase[0] += 0.01f + ambienceRandom.nextFloat() * 0.02f; // Chatter
                    ambiencePhase[1] += 0.003f; // Low rumble
                    ambience = std::sin(ambiencePhase[0]) * 0.3f + 
                              std::sin(ambiencePhase[1]) * 0.1f +
                              (ambienceRandom.nextFloat() * 2.0f - 1.0f) * 0.1f; // Random noise
                    break;
                    
                case Car_Highway:
                    // Car on highway: engine, wind, road noise
                    ambiencePhase[0] += 0.008f; // Engine rumble
                    ambiencePhase[1] += 0.15f;  // Wind noise
                    ambience = std::sin(ambiencePhase[0]) * 0.4f +
                              std::sin(ambiencePhase[1]) * (ambienceRandom.nextFloat() * 0.2f + 0.1f);
                    break;
                    
                case Street_Traffic:
                    // City street: cars, horns, general urban noise
                    ambiencePhase[0] += 0.005f + ambienceRandom.nextFloat() * 0.01f;
                    if (ambienceRandom.nextFloat() > 0.998f) // Occasional car horn
                    {
                        ambienceLevel[0] = 0.5f;
                    }
                    ambienceLevel[0] *= 0.95f; // Decay
                    ambience = std::sin(ambiencePhase[0]) * 0.2f + ambienceLevel[0];
                    break;
                    
                case Underground_Tube:
                    // London Underground: train rumble, announcements, echoes
                    ambiencePhase[0] += 0.003f; // Deep rumble
                    ambiencePhase[1] += 0.02f;  // Electrical hum
                    ambience = std::sin(ambiencePhase[0]) * 0.5f +
                              std::sin(ambiencePhase[1]) * 0.1f +
                              (ambienceRandom.nextFloat() * 2.0f - 1.0f) * 0.05f;
                    break;
                    
                case Office_Quiet:
                    // Quiet office: air conditioning, keyboards, quiet conversations
                    ambiencePhase[0] += 0.001f; // AC hum
                    if (ambienceRandom.nextFloat() > 0.995f) // Occasional keyboard
                    {
                        ambienceLevel[1] = 0.1f;
                    }
                    ambienceLevel[1] *= 0.8f; // Quick decay
                    ambience = std::sin(ambiencePhase[0]) * 0.05f + ambienceLevel[1];
                    break;
                    
                case Train_Interior:
                {
                    // Inside moving train: rhythmic clacking, gentle swaying
                    ambiencePhase[0] += 0.02f;  // Track rhythm
                    ambiencePhase[1] += 0.004f; // Train rumble
                    float trackRhythm = std::sin(ambiencePhase[0]) > 0.8f ? 0.2f : 0.0f;
                    ambience = trackRhythm + std::sin(ambiencePhase[1]) * 0.3f;
                    break;
                }
                    
                case Airport_Terminal:
                    // Airport background: announcements, people, air conditioning
                    ambiencePhase[0] += 0.002f; // AC system
                    ambiencePhase[1] += 0.01f + ambienceRandom.nextFloat() * 0.02f; // People
                    if (ambienceRandom.nextFloat() > 0.9995f) // Rare announcement
                    {
                        ambienceLevel[2] = 0.3f;
                    }
                    ambienceLevel[2] *= 0.98f; // Slow decay
                    ambience = std::sin(ambiencePhase[0]) * 0.1f +
                              std::sin(ambiencePhase[1]) * 0.2f + ambienceLevel[2];
                    break;
                    
                default: // Silent
                    ambience = 0.0f;
                    break;
            }
            
            // Mix ambience with existing audio
            channelData[sample] += ambience * level * 0.15f; // Keep ambience subtle
        }
    }
}

//==============================================================================
// Frequency conversion functions for discrete choice parameters

float TestAudioProcessor::getLowCutFrequency(int choiceIndex) const
{
    switch (choiceIndex)
    {
        case 0: return 0.0f;    // Off - return 0 to indicate no filtering
        case 1: return 180.0f;  // 180Hz
        case 2: return 250.0f;  // 250Hz
        case 3: return 300.0f;  // 300Hz
        case 4: return 400.0f;  // 400Hz
        default: return 0.0f;   // Default to Off
    }
}

float TestAudioProcessor::getHighCutFrequency(int choiceIndex) const
{
    switch (choiceIndex)
    {
        case 0: return 0.0f;     // Off - return 0 to indicate no filtering
        case 1: return 2800.0f;  // 2.8kHz
        case 2: return 3200.0f;  // 3.2kHz
        case 3: return 3400.0f;  // 3.4kHz
        case 4: return 7000.0f;  // 7kHz
        default: return 0.0f;    // Default to Off
    }
}

//==============================================================================
// Parameter smoothing for professional transitions

void TestAudioProcessor::updateParameterSmoothing()
{
    // Smooth parameter transitions for professional feel
    currentDistortion += (targetDistortion - currentDistortion) * smoothingSpeed;
    currentNoise += (targetNoise - currentNoise) * smoothingSpeed;
    currentInterference += (targetInterference - currentInterference) * smoothingSpeed;
    currentCompression += (targetCompression - currentCompression) * smoothingSpeed;
    
    // Apply smoothed values to actual parameters (with slight delay to avoid clicks)
    if (std::abs(targetDistortion - currentDistortion) < 0.001f)
        apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(targetDistortion);
    if (std::abs(targetNoise - currentNoise) < 0.001f)  
        apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(targetNoise);
    if (std::abs(targetInterference - currentInterference) < 0.001f)
        apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(targetInterference);
    if (std::abs(targetCompression - currentCompression) < 0.001f)
        apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(targetCompression);
}

//==============================================================================
// GAME-CHANGING: Dynamic Signal Strength System (replaces interference)

void TestAudioProcessor::detectVoiceActivity(float inputLevel)
{
    // Smooth voice activity detection
    float targetActivity = (inputLevel > 0.01f) ? inputLevel : 0.0f;
    voiceActivityLevel += (targetActivity - voiceActivityLevel) * voiceActivitySmoothing;
    
    // Update silence timer
    if (voiceActivityLevel > 0.05f)
    {
        silenceTimer = 0.0f; // Reset silence timer when voice detected
    }
    else
    {
        silenceTimer += 1.0f / static_cast<float>(currentSampleRate);
    }
}

void TestAudioProcessor::updateSignalStrength(float inputLevel)
{
    // Update signal change timer
    signalChangeTimer += 1.0f / static_cast<float>(currentSampleRate);
    
    // INTELLIGENT: Signal degrades during silence (realistic!) - BUT MORE STABLE
    if (silenceTimer > silenceThreshold)
    {
        // Signal degrades when quiet (like real phones!) - MUCH MORE GRADUAL  
        targetSignalStrength = 0.6f + random.nextFloat() * 0.3f; // FIXED: 60-90% strength (was 30-70%)
    }
    else if (voiceActivityLevel > 0.1f)
    {
        // Signal improves when talking (realistic behavior!) - STABLE
        targetSignalStrength = 0.8f + random.nextFloat() * 0.2f; // FIXED: 80-100% strength (was 70-100%)
    }
    
    // MUCH LESS FREQUENT signal variations (every 5-10 seconds instead of 2-5)
    if (signalChangeTimer > (5.0f + random.nextFloat() * 5.0f)) // FIXED: Much more stable
    {
        signalChangeTimer = 0.0f;
        
        // Phone-specific signal behavior - MORE STABLE
        switch (static_cast<PhoneType>(static_cast<int>(phoneTypeParam->load())))
        {
            case Nokia:
                // Nokia: Very stable signal, rare drops
                targetSignalStrength = 0.8f + random.nextFloat() * 0.2f; // FIXED: Much more stable (was 0.6-1.0)
                if (random.nextFloat() > 0.98f) targetSignalStrength = 0.6f; // FIXED: Much rarer dropouts
                break;
                
            case iPhone:
                // iPhone: Excellent signal, very stable
                targetSignalStrength = 0.9f + random.nextFloat() * 0.1f; // FIXED: Very stable (was 0.8-1.0)
                break;
                
            case SonyEricsson:
                // Sony: Slightly more variable but still reasonable
                targetSignalStrength = 0.7f + random.nextFloat() * 0.3f; // FIXED: More stable (was 0.4-1.0)
                break;
        }
    }
    
    // MUCH SMOOTHER signal strength transitions 
    signalSmoothingSpeed = 0.005f; // FIXED: Even smoother (was 0.01f)
    currentSignalStrength += (targetSignalStrength - currentSignalStrength) * signalSmoothingSpeed;
    currentSignalStrength = juce::jlimit(0.0f, 1.0f, currentSignalStrength);
    
    // Update signal bars for GUI (1-5 bars)
    signalBars = static_cast<int>(currentSignalStrength * 4.0f) + 1;
    signalBars = juce::jlimit(1, 5, signalBars);
}

float TestAudioProcessor::applySignalQuality(float input, PhoneType phoneType, SignalQuality quality)
{
    float effectiveSignalStrength = currentSignalStrength;
    
    // Override signal strength based on quality setting
    switch (quality)
    {
        case Perfect_Signal:
            effectiveSignalStrength = 1.0f;
            break;
        case Good_Signal:
            effectiveSignalStrength = 0.9f;  // FIXED: More forgiving
            break;
        case Fair_Signal:
            effectiveSignalStrength = 0.75f; // FIXED: Less harsh
            break;
        case Poor_Signal:
            effectiveSignalStrength = 0.6f;  // FIXED: Still usable
            break;
        case Breaking_Up:
            effectiveSignalStrength = 0.4f;  // FIXED: Less extreme
            break;
        case Auto_Dynamic:
            // Use intelligent signal strength (no override)
            break;
    }
    
    // Simulate call dynamics (dropouts, recovery)
    simulateCallDynamics();
    
    // Apply signal-based effects - MUCH MORE SUBTLE
    float processedInput = input;
    
    // Signal dropouts (breaking up) - LESS AGGRESSIVE
    if (isInDropout || effectiveSignalStrength < 0.4f)
    {
        // Gentle signal loss - less jarring
        if (random.nextFloat() > (effectiveSignalStrength + 0.5f)) // FIXED: Much less frequent
        {
            processedInput *= 0.3f; // FIXED: Less extreme (was 0.1f)
        }
        
        // Very subtle crackling - MUCH LESS FREQUENT
        if (random.nextFloat() > 0.998f) // FIXED: Only 0.2% chance (was 5%)
        {
            processedInput += (random.nextFloat() * 2.0f - 1.0f) * 0.02f; // FIXED: Much quieter
        }
    }
    
    // Signal-dependent compression and distortion - VERY SUBTLE
    if (effectiveSignalStrength < 0.8f) // FIXED: Only apply when signal is quite poor
    {
        // Very gentle compression - like real phones
        processedInput = applySmartCompression(processedInput, effectiveSignalStrength);
        
        // Minimal background noise - MUCH CLEANER
        float noiseLevel = (1.0f - effectiveSignalStrength) * 0.015f; // FIXED: Much less noise (was 0.08f)
        processedInput += (random.nextFloat() * 2.0f - 1.0f) * noiseLevel;
    }
    
    return processedInput;
}

void TestAudioProcessor::simulateCallDynamics()
{
    callQualityTimer += 1.0f / static_cast<float>(currentSampleRate);
    
    // Handle current dropout
    if (isInDropout)
    {
        dropoutRecoveryTimer += 1.0f / static_cast<float>(currentSampleRate);
        
        if (dropoutRecoveryTimer >= dropoutDuration)
        {
            // Recovery from dropout
            isInDropout = false;
            dropoutRecoveryTimer = 0.0f;
            targetSignalStrength = 0.7f + random.nextFloat() * 0.3f; // Signal recovers
        }
    }
    else
    {
        // Check for new dropout events (more likely with poor signal)
        float dropoutProbability = (1.0f - currentSignalStrength) * 0.0002f; // Very low base probability
        
        if (random.nextFloat() < dropoutProbability)
        {
            // Start a dropout
            isInDropout = true;
            dropoutDuration = 0.5f + random.nextFloat() * 2.0f; // 0.5-2.5 seconds
            dropoutRecoveryTimer = 0.0f;
            targetSignalStrength = 0.1f; // Signal drops dramatically
        }
    }
}

float TestAudioProcessor::applySmartCompression(float input, float signalStrength)
{
    // MUCH GENTLER compression - like real phone calls (not aggressive studio compression!)
    float compressionIntensity = (1.0f - signalStrength) * 0.3f; // FIXED: Much less intense (was 0.8f)
    float threshold = 0.5f - (compressionIntensity * 0.1f);       // FIXED: Higher threshold, less compression
    float ratio = 1.5f + (compressionIntensity * 2.0f);          // FIXED: Gentle ratios 1.5:1 to 3.5:1 (was 2:1 to 10:1!)
    
    float absInput = std::abs(input);
    if (absInput > threshold)
    {
        float excess = absInput - threshold;
        float compressedExcess = excess / ratio;
        float sign = (input >= 0.0f) ? 1.0f : -1.0f;
        return sign * (threshold + compressedExcess);
    }
    
    return input;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestAudioProcessor();
}
