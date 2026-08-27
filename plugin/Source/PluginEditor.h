#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class FLStudioCollabAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FLStudioCollabAudioProcessorEditor (FLStudioCollabAudioProcessor&);
    ~FLStudioCollabAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    FLStudioCollabAudioProcessor& audioProcessor;

    // UI Controls
    juce::Label headerTitleLabel;
    juce::Label statusBadgeLabel;
    
    juce::Label roomCodePromptLabel;
    juce::TextEditor roomCodeInput;

    juce::TextButton createRoomButton;
    juce::TextButton joinRoomButton;
    juce::TextButton copyCodeButton;

    juce::Label activityLabel;
    juce::Label logLabel;

    juce::Label historyLabel;
    juce::ComboBox historyComboBox;
    juce::TextButton replayHistoryButton;

    juce::TextButton validateButton;

    void setupWebSocketCallbacks();
    void updateHistoryComboBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FLStudioCollabAudioProcessorEditor)
};
