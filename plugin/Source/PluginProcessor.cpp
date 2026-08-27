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
    // Setup WebSocket callback for incoming MIDI patterns
    wsClient.onMidiReceived = [this](const std::vector<CapturedMidiNote>& notes, const juce::String& trackName)
    {
        queueIncomingMidiNotes(notes, trackName);
    };

    // Setup WebSocket callback for incoming Audio Renders
    wsClient.onAudioReceived = [this](const juce::AudioBuffer<float>& buffer, const juce::String& trackName)
    {
        queueIncomingAudioBuffer(buffer, trackName);
    };
}

FLStudioCollabAudioProcessor::~FLStudioCollabAudioProcessor()
{
}

void FLStudioCollabAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    std::lock_guard<std::mutex> lockMidi(midiMutex);
    capturedBuffer.clear();
    incomingQueue.clear();

    std::lock_guard<std::mutex> lockAudio(audioMutex);
    // Allocate 10 seconds recording buffer for audio render mode (2 channels)
    recordedAudioBuffer.setSize(2, (int) (sampleRate * 10.0));
    recordedAudioBuffer.clear();
    recordedAudioSamples = 0;

    incomingAudioBuffer.setSize(2, 0);
    incomingAudioReadPos = 0;
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

    // 1. Capture audio samples if in Audio Render mode
    if (currentMode == CollabMode::AudioRender && totalNumInputChannels > 0)
    {
        std::lock_guard<std::mutex> lockAudio(audioMutex);
        int numSamplesToCopy = juce::jmin(buffer.getNumSamples(), recordedAudioBuffer.getNumSamples() - recordedAudioSamples);
        
        if (numSamplesToCopy > 0)
        {
            for (int ch = 0; ch < juce::jmin(totalNumInputChannels, recordedAudioBuffer.getNumChannels()); ++ch)
            {
                recordedAudioBuffer.copyFrom(ch, recordedAudioSamples, buffer, ch, 0, numSamplesToCopy);
            }
            recordedAudioSamples += numSamplesToCopy;
        }
    }

    // 2. Playback incoming received Audio Render
    {
        std::lock_guard<std::mutex> lockAudio(audioMutex);
        if (incomingAudioBuffer.getNumSamples() > 0 && incomingAudioReadPos < incomingAudioBuffer.getNumSamples())
        {
            int numSamplesToPlay = juce::jmin(buffer.getNumSamples(), incomingAudioBuffer.getNumSamples() - incomingAudioReadPos);
            
            for (int ch = 0; ch < totalNumOutputChannels; ++ch)
            {
                int srcCh = juce::jmin(ch, incomingAudioBuffer.getNumChannels() - 1);
                buffer.addFrom(ch, 0, incomingAudioBuffer, srcCh, incomingAudioReadPos, numSamplesToPlay);
            }
            incomingAudioReadPos += numSamplesToPlay;
        }
    }

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 3. Capture incoming host MIDI notes from FL Studio for local draft buffer
    bool notesAdded = false;
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
            
            std::lock_guard<std::mutex> lockMidi(midiMutex);
            capturedBuffer.push_back(note);
            notesAdded = true;
        }
    }

    if (notesAdded)
    {
        wsClient.sendDraftActivity(true);
    }

    // 4. Playback incoming MIDI notes received from remote peer
    {
        std::lock_guard<std::mutex> lockMidi(midiMutex);
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
    if (currentMode == CollabMode::MIDI)
    {
        std::lock_guard<std::mutex> lock(midiMutex);
        
        // If no real MIDI notes recorded yet during testing, generate sample notes
        if (capturedBuffer.empty())
        {
            CapturedMidiNote note1{ 60, 0.8f, 0, 22050 };
            CapturedMidiNote note2{ 64, 0.9f, 11025, 22050 };
            capturedBuffer.push_back(note1);
            capturedBuffer.push_back(note2);
        }

        wsClient.sendMidiValidation(currentTrackName, capturedBuffer);
        DBG("[CollabPlugin] Validated and sent " + juce::String(capturedBuffer.size()) + " MIDI notes.");
    }
    else // CollabMode::AudioRender
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        
        // Trim recorded audio buffer
        juce::AudioBuffer<float> sendAudioBuffer(2, juce::jmax(1, recordedAudioSamples));
        if (recordedAudioSamples > 0)
        {
            for (int ch = 0; ch < sendAudioBuffer.getNumChannels(); ++ch)
            {
                sendAudioBuffer.copyFrom(ch, 0, recordedAudioBuffer, ch, 0, recordedAudioSamples);
            }
        }
        else
        {
            sendAudioBuffer.setSize(2, 44100);
            sendAudioBuffer.clear();
        }

        wsClient.sendAudioValidation(currentTrackName + " (Audio Render)", sendAudioBuffer, currentSampleRate);
        DBG("[CollabPlugin] Validated and sent " + juce::String(sendAudioBuffer.getNumSamples()) + " audio render samples.");
    }
}

void FLStudioCollabAudioProcessor::queueIncomingMidiNotes(const std::vector<CapturedMidiNote>& notes, const juce::String& trackName)
{
    std::lock_guard<std::mutex> lock(midiMutex);
    incomingQueue.insert(incomingQueue.end(), notes.begin(), notes.end());

    // Add entry to history
    ValidatedPatternItem item;
    item.index = (int) validationHistory.size() + 1;
    item.timestampStr = juce::Time::getCurrentTime().toString(false, true, true, true);
    item.trackName = trackName.isEmpty() ? "Piste MIDI reçue" : trackName;
    item.mode = CollabMode::MIDI;
    item.notes = notes;

    validationHistory.push_back(item);
}

void FLStudioCollabAudioProcessor::queueIncomingAudioBuffer(const juce::AudioBuffer<float>& buffer, const juce::String& trackName)
{
    std::lock_guard<std::mutex> lock(audioMutex);
    incomingAudioBuffer.makeCopyOf(buffer);
    incomingAudioReadPos = 0;

    // Add entry to history
    ValidatedPatternItem item;
    item.index = (int) validationHistory.size() + 1;
    item.timestampStr = juce::Time::getCurrentTime().toString(false, true, true, true);
    item.trackName = trackName.isEmpty() ? "Piste Audio reçue" : trackName;
    item.mode = CollabMode::AudioRender;
    item.audioBuffer.makeCopyOf(buffer);

    validationHistory.push_back(item);
}

void FLStudioCollabAudioProcessor::replayHistoricalPattern(int historyIndex)
{
    if (historyIndex >= 0 && historyIndex < (int) validationHistory.size())
    {
        const auto& item = validationHistory[(size_t) historyIndex];
        if (item.mode == CollabMode::MIDI)
        {
            std::lock_guard<std::mutex> lock(midiMutex);
            incomingQueue.insert(incomingQueue.end(), item.notes.begin(), item.notes.end());
        }
        else
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            incomingAudioBuffer.makeCopyOf(item.audioBuffer);
            incomingAudioReadPos = 0;
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FLStudioCollabAudioProcessor();
}
