/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VHFRadioAudioProcessor::VHFRadioAudioProcessor() :
#if JUCE_IOS || JUCE_MAC
    AudioProcessor(juce::JUCEApplicationBase::isStandaloneApp() ?
        BusesProperties().withInput("Input", juce::AudioChannelSet::mono(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true) :
        BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
#else
    AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
#endif
{
    MemoryInputStream jsonStream(BinaryData::model_json , BinaryData::model_jsonSize, false);
    auto jsonInput = nlohmann::json::parse(jsonStream.readEntireStreamAsString().toStdString());
    neuralNet[0] = RTNeural::json_parser::parseJson<float>(jsonInput);
    neuralNet[1] = RTNeural::json_parser::parseJson<float>(jsonInput);
}

VHFRadioAudioProcessor::~VHFRadioAudioProcessor()
{
}

//==============================================================================
const juce::String VHFRadioAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VHFRadioAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VHFRadioAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VHFRadioAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VHFRadioAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VHFRadioAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VHFRadioAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VHFRadioAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VHFRadioAudioProcessor::getProgramName (int index)
{
    return {};
}

void VHFRadioAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VHFRadioAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    constexpr float Q = 10.0f;
    constexpr float lowCutoff = 100.0f;
    constexpr float highCutoff = 3000.0f;

    dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 2 };

    *highPass.state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowCutoff,Q);
    *lowPass.state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, highCutoff, Q);

    inputGain.prepare(spec);
    inputGain.setRampDurationSeconds(0.05);
    lowPass.prepare(spec);
    highPass.prepare(spec);

    inputGain.setGainDecibels(25.0f);

    neuralNet[0]->reset();
    neuralNet[1]->reset();
}

void VHFRadioAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VHFRadioAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VHFRadioAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    dsp::AudioBlock<float> block(buffer);
    dsp::ProcessContextReplacing<float> context(block);

    inputGain.process(context);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* x = buffer.getWritePointer(ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            float input[] = { x[n] };
            x[n] = neuralNet[ch]->forward(input);
        }
    }

    lowPass.process(context);
    highPass.process(context);

    buffer.applyGain(4.0f);

    ignoreUnused(midiMessages);
}

//==============================================================================
bool VHFRadioAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VHFRadioAudioProcessor::createEditor()
{
    return new VHFRadioAudioProcessorEditor (*this);
}

//==============================================================================
void VHFRadioAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VHFRadioAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VHFRadioAudioProcessor();
}
