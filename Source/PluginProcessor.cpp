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

// NEW: Simplified interference preset system
const juce::String TestAudioProcessor::INTERFERENCE_PRESET_ID = "interferencePreset";

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
    
    // NEW: Simplified interference preset parameter
    interferencePresetParam = apvts.getRawParameterValue(INTERFERENCE_PRESET_ID);
    
    // Initialize noise buffer

    compressionGain = 1.0f;
    hissLevel = 0.0f;
    hissPhase = 0.0f;
    
    // Initialize tonal coloring phases for authentic phone character
    nokiaDigitalPhase = 0.0f;
    iphoneWarmthPhase = 0.0f;
    sonyAnalogPhase = 0.0f;
}

TestAudioProcessor::~TestAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TestAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    // Low Cut Frequency (50Hz - 2000Hz)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        LOW_CUT_ID, "Low Cut", 
        juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f), 400.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value)) + " Hz"; }
    ));
    
    // High Cut Frequency (1000Hz - 20000Hz)
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        HIGH_CUT_ID, "High Cut",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f), 4000.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value)) + " Hz"; }
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
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.2f,
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
    
    // NEW: Simplified interference preset parameter
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        INTERFERENCE_PRESET_ID, "Interference Preset",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f
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

    // Update filter coefficients
    auto lowCutFreq = lowCutParam->load();
    auto highCutFreq = highCutParam->load();
    
    // High-pass filter (low cut)
    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        currentSampleRate, lowCutFreq, 0.707f);
    
    // Low-pass filter (high cut)  
    *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        currentSampleRate, highCutFreq, 0.707f);
    
    // Create audio block for DSP processing
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Apply filters
    lowCutFilter.process(context);
    highCutFilter.process(context);
    
    // Get current parameter values
    auto phoneType = static_cast<PhoneType>(static_cast<int>(phoneTypeParam->load()));
    auto distortionAmount = distortionParam->load();
    auto noiseLevel = noiseLevelParam->load();
    auto interferenceLevel = interferenceParam->load();
    auto compressionLevel = compressionParam->load();
    auto interferencePreset = static_cast<int>(interferencePresetParam->load());
    
    // Apply phone-specific processing with authentic interference
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto input = channelData[sample];
            
            // Apply phone-specific tonal coloring FIRST (authentic sound character)
            input = applyPhoneTonalColor(input, phoneType, tonalColoringIntensity);
            
            // Apply phone-specific interference based on preset
            input = applyAuthenticInterference(input, phoneType, interferencePreset, 
                                             noiseLevel, interferenceLevel);
            
            // Apply distortion
            if (distortionAmount > 0.0f)
            {
                input = applyPhoneDistortion(input, phoneType, distortionAmount);
            }
            
            // Apply compression
            if (compressionLevel > 0.0f)
            {
                input = applyPhoneCompression(input, phoneType, compressionLevel);
            }
            
            channelData[sample] = input;
        }
    }
}

//==============================================================================
// AUTHENTIC INTERFERENCE METHODS (Phase 4)

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

float TestAudioProcessor::applyNokiaInterference(float input, int preset, float noiseLevel, float interferenceLevel)
{
    switch (preset)
    {
        case Nokia_NearTV:
            // Classic GSM buzz - 217Hz burst pattern
            gsmBurstTimer += 1.0f / static_cast<float>(currentSampleRate);
            if (gsmBurstTimer >= 0.0046f) // GSM burst every 4.6ms
            {
                gsmBurstTimer = 0.0f;
                gsmBurstActive = random.nextFloat() > 0.7f;
            }
            if (gsmBurstActive)
            {
                gsmPhase += 2.0f * juce::MathConstants<float>::pi * 217.0f / static_cast<float>(currentSampleRate);
                input += std::sin(gsmPhase) * 0.02f * interferenceLevel; // Much more subtle
            }
            break;
            
        case Nokia_AlarmClock:
            // Disrupting alarm clock/radio - random digital clicks
            if (random.nextFloat() > 0.9985f) // Much less frequent
            {
                input += (random.nextFloat() * 2.0f - 1.0f) * 0.03f * interferenceLevel; // Much quieter
            }
            break;
            
        case Nokia_CarRadio:
            // In-car interference - periodic bursts
            if (static_cast<int>(gsmPhase * 100) % 200 < 50)
            {
                input += (random.nextFloat() * 2.0f - 1.0f) * 0.015f * interferenceLevel; // Much more subtle
            }
            break;
    }
    
    // Add base Nokia digital noise - much more subtle
    if (noiseLevel > 0.0f)
    {
        input += (random.nextFloat() * 2.0f - 1.0f) * 0.008f * noiseLevel; // Very quiet background noise
    }
    
    return input;
}

float TestAudioProcessor::applyIPhoneInterference(float input, int preset, float noiseLevel, float interferenceLevel)
{
    switch (preset)
    {
        case iPhone_NearSpeakers:
            // Digital buzz near computer speakers - cleaner than Nokia
            if (random.nextFloat() > 0.9992f) // Much less frequent
            {
                input += std::sin(gsmPhase * 2.0f) * 0.015f * interferenceLevel; // Much more subtle
            }
            break;
            
        case iPhone_Charging:
            // Interference while charging - periodic hum (much more predictable)
            gsmPhase += 0.005f; // Slower, more predictable phase
            input += std::sin(gsmPhase) * 0.003f * interferenceLevel; // Much quieter hum
            break;
            
        case iPhone_DataSync:
            // During email/data transfer - digital artifacts
            if (random.nextFloat() > 0.9995f) // Less frequent
            {
                input += (random.nextFloat() * 2.0f - 1.0f) * 0.02f * interferenceLevel; // Much quieter
            }
            break;
    }
    
    // Add base iPhone clean digital noise - very subtle
    if (noiseLevel > 0.0f)
    {
        input += (random.nextFloat() * 2.0f - 1.0f) * 0.005f * noiseLevel; // Extremely quiet
    }
    
    return input;
}

float TestAudioProcessor::applySonyEricssonInterference(float input, int preset, float noiseLevel, float interferenceLevel)
{
    switch (preset)
    {
        case Sony_Underground:
            // Signal cutting out in tube/basement - periodic dropouts
            if (random.nextFloat() > 0.995f) // Less frequent
            {
                return input * (0.7f + random.nextFloat() * 0.2f); // Less dramatic fade
            }
            break;
            
        case Sony_WindNoise:
            // Background noise pickup in car - wind/road noise
            hissPhase += 0.1f + random.nextFloat() * 0.05f;
            input += std::sin(hissPhase) * (random.nextFloat() * 2.0f - 1.0f) * 0.02f * interferenceLevel; // Much quieter
            break;
            
        case Sony_FlipStatic:
            // Flip phone opening/closing static - crackle bursts
            if (random.nextFloat() > 0.9998f) // Much less frequent
            {
                input += (random.nextFloat() * 2.0f - 1.0f) * 0.05f * interferenceLevel; // Much quieter
            }
            break;
    }
    
    // Add base Sony Ericsson analog hiss - very subtle
    if (noiseLevel > 0.0f)
    {
        hissPhase += 0.08f;
        input += std::sin(hissPhase) * (random.nextFloat() * 2.0f - 1.0f) * 0.012f * noiseLevel; // Much quieter
    }
    
    return input;
}

float TestAudioProcessor::applyPhoneDistortion(float input, PhoneType phoneType, float amount)
{
    switch (phoneType)
    {
        case Nokia:
            // Nokia digital clipping
            return juce::jlimit(-amount, amount, input * (1.0f + amount * 2.0f));
        case iPhone:
            // iPhone soft limiting
            return std::tanh(input * (1.0f + amount * 1.5f)) * 0.8f;
        case SonyEricsson:
            // Sony analog saturation
            return std::atan(input * (1.0f + amount * 3.0f)) * 0.7f;
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
            apvts.getParameter(LOW_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(LOW_CUT_ID)->convertTo0to1(400.0f));
            apvts.getParameter(HIGH_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(HIGH_CUT_ID)->convertTo0to1(4000.0f));
            apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(0.0f);
            apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(0.05f); // Much lower
            apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(0.1f); // Much lower
            apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(0.3f); // Lower
            break;
            
        case iPhone:
            apvts.getParameter(LOW_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(LOW_CUT_ID)->convertTo0to1(300.0f));
            apvts.getParameter(HIGH_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(HIGH_CUT_ID)->convertTo0to1(8000.0f));
            apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(0.0f);
            apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(0.02f); // Much lower
            apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(0.05f); // Much lower
            apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(0.15f); // Lower
            break;
            
        case SonyEricsson:
            apvts.getParameter(LOW_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(LOW_CUT_ID)->convertTo0to1(250.0f));
            apvts.getParameter(HIGH_CUT_ID)->setValueNotifyingHost(
                apvts.getParameter(HIGH_CUT_ID)->convertTo0to1(2500.0f));
            apvts.getParameter(DISTORTION_ID)->setValueNotifyingHost(0.1f); // Lower
            apvts.getParameter(NOISE_LEVEL_ID)->setValueNotifyingHost(0.08f); // Much lower
            apvts.getParameter(INTERFERENCE_ID)->setValueNotifyingHost(0.12f); // Much lower
            apvts.getParameter(COMPRESSION_ID)->setValueNotifyingHost(0.4f); // Lower
            break;
    }
    
    // Update phone type parameter
    apvts.getParameter(PHONE_TYPE_ID)->setValueNotifyingHost(static_cast<float>(phoneType) / 2.0f);
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
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestAudioProcessor();
}
