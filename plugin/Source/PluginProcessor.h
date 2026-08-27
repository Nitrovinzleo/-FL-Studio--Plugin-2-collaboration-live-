#pragma once

#include <JuceHeader.h>
#include <vector>
#include <mutex>
#include <string>

// Represents a captured MIDI note event
struct CapturedMidiNote
{
    int noteNumber = 60;
    float velocity = 0.8f;
    int sampleOffset = 0;
    int lengthSamples = 22050;
};

class FLStudioCollabAudioProcessor  : public juce::AudioProcessor
{
public:
    FLStudioCollabAudioProcessor();
    ~FLStudioCollabAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const juce::String getProgramName (int index) override { return {}; }
    void changeProgramName (int index, const juce::String& newName) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Collaboration API methods for UI & Network Thread
    void validateAndSendCurrentPattern();
    void queueIncomingMidiNotes(const std::vector<CapturedMidiNote>& notes);
    
    juce::String getRoomCode() const { return currentRoomCode; }
    void setRoomCode(const juce::String& code) { currentRoomCode = code; }

    bool isConnected() const { return connectedState; }
    void setConnectedState(bool connected) { connectedState = connected; }

private:
    // Audio-thread safe FIFO / vectors for captured MIDI
    std::mutex midiMutex;
    std::vector<CapturedMidiNote> capturedBuffer;
    std::vector<CapturedMidiNote> incomingQueue;

    juce::String currentRoomCode = "";
    bool connectedState = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FLStudioCollabAudioProcessor)
};
