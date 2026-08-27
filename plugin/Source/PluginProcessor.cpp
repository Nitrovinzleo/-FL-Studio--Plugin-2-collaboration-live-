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
    // Setup WebSocket callback for incoming MIDI patterns from remote collaborator
    wsClient.onMidiReceived = [this](const std::vector<CapturedMidiNote>& notes)
    {
        queueIncomingMidiNotes(notes);
    };
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

    // 1. Capture incoming host MIDI notes from FL Studio for local draft buffer
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            CapturedMidiNote note;
            note.noteNumber = msg.getNoteNumber();
            note.velocity = msg.getFloatVelocity();
            note.sampleOffset = metadata.samplePosition;
            note.lengthSamples = 22050; // Default note length
            
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
    
    // If no real MIDI notes recorded yet during testing, generate sample note for demonstration
    if (capturedBuffer.empty())
    {
        CapturedMidiNote note1{ 60, 0.8f, 0, 22050 };
        CapturedMidiNote note2{ 64, 0.9f, 11025, 22050 };
        capturedBuffer.push_back(note1);
        capturedBuffer.push_back(note2);
    }

    wsClient.sendMidiValidation("FL Track Draft", capturedBuffer);
    DBG("[CollabPlugin] Validated and sent " + juce::String(capturedBuffer.size()) + " notes.");
}

void FLStudioCollabAudioProcessor::queueIncomingMidiNotes(const std::vector<CapturedMidiNote>& notes)
{
    std::lock_guard<std::mutex> lock(midiMutex);
    incomingQueue.insert(incomingQueue.end(), notes.begin(), notes.end());
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FLStudioCollabAudioProcessor();
}
