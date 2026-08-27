#include "PluginProcessor.h"
#include "PluginEditor.h"

FLStudioCollabAudioProcessor::FLStudioCollabAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if JucePlugin_IsSynth
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      #else
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      #endif
                     #endif
                       )
#endif
{
}

FLStudioCollabAudioProcessor::~FLStudioCollabAudioProcessor()
{
}

void FLStudioCollabAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    std::lock_guard<std::mutex> lock(midiMutex);
    capturedBuffer.clear();
    incomingQueue.clear();
}

void FLStudioCollabAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FLStudioCollabAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FLStudioCollabAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 1. Capture incoming host MIDI notes for local draft buffer
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            CapturedMidiNote note;
            note.noteNumber = msg.getNoteNumber();
            note.velocity = msg.getFloatVelocity();
            note.sampleOffset = metadata.samplePosition;
            
            std::lock_guard<std::mutex> lock(midiMutex);
            capturedBuffer.push_back(note);
        }
    }

    // 2. Playback incoming MIDI notes received from remote peer
    {
        std::lock_guard<std::mutex> lock(midiMutex);
        if (!incomingQueue.empty())
        {
            for (const auto& note : incomingQueue)
            {
                auto noteOnMsg = juce::MidiMessage::noteOn(1, note.noteNumber, note.velocity);
                midiMessages.addEvent(noteOnMsg, note.sampleOffset);
            }
            incomingQueue.clear();
        }
    }
}

juce::AudioProcessorEditor* FLStudioCollabAudioProcessor::createEditor()
{
    return new FLStudioCollabAudioProcessorEditor (*this);
}

void FLStudioCollabAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void FLStudioCollabAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

void FLStudioCollabAudioProcessor::validateAndSendCurrentPattern()
{
    std::lock_guard<std::mutex> lock(midiMutex);
    DBG("[CollabPlugin] Validating & sending " + juce::String(capturedBuffer.size()) + " notes to room " + currentRoomCode);
    // In production network layer: Serializes capturedBuffer to JSON payload and dispatches over WebSocket thread
}

void FLStudioCollabAudioProcessor::queueIncomingMidiNotes(const std::vector<CapturedMidiNote>& notes)
{
    std::lock_guard<std::mutex> lock(midiMutex);
    incomingQueue.insert(incomingQueue.end(), notes.begin(), notes.end());
}

// JUCE plugin entry point factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FLStudioCollabAudioProcessor();
}
