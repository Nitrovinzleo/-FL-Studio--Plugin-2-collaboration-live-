#pragma once

#include <JuceHeader.h>
#include <vector>
#include <mutex>
#include <string>
#include "WebSocketClient.h"

// Mode selector: MIDI vs Audio Render
enum class CollabMode
{
    MIDI = 0,
    AudioRender = 1
};

// Represents a captured MIDI note event
struct CapturedMidiNote
{
    int noteNumber = 60;
    float velocity = 0.8f;
    int sampleOffset = 0;
    int lengthSamples = 22050;
};

// Represents a validated historical pattern entry
struct ValidatedPatternItem
{
    int index = 1;
    juce::String timestampStr;
    juce::String trackName;
    CollabMode mode = CollabMode::MIDI;
    std::vector<CapturedMidiNote> notes;
    juce::AudioBuffer<float> audioBuffer;
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

    // Collaboration API methods
    void validateAndSendCurrentPattern();
    void queueIncomingMidiNotes(const std::vector<CapturedMidiNote>& notes, const juce::String& trackName = "Piste reçue");
    void queueIncomingAudioBuffer(const juce::AudioBuffer<float>& buffer, const juce::String& trackName = "Rendu Audio reçu");
    void replayHistoricalPattern(int historyIndex);

    CollabMode getCurrentMode() const { return currentMode; }
    void setCurrentMode(CollabMode mode) { currentMode = mode; }

    juce::String getTrackName() const { return currentTrackName; }
    void setTrackName(const juce::String& name) { currentTrackName = name; }

    const std::vector<ValidatedPatternItem>& getValidationHistory() const { return validationHistory; }

    WebSocketClient& getWebSocketClient() { return wsClient; }

private:
    WebSocketClient wsClient;

    CollabMode currentMode = CollabMode::MIDI;
    juce::String currentTrackName = "Piste 1: Main";
    double currentSampleRate = 44100.0;

    // Audio-thread safe vectors & buffers
    std::mutex midiMutex;
    std::mutex audioMutex;

    std::vector<CapturedMidiNote> capturedBuffer;
    std::vector<CapturedMidiNote> incomingQueue;

    juce::AudioBuffer<float> recordedAudioBuffer;
    int recordedAudioSamples = 0;

    juce::AudioBuffer<float> incomingAudioBuffer;
    int incomingAudioReadPos = 0;

    std::vector<ValidatedPatternItem> validationHistory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FLStudioCollabAudioProcessor)
};
