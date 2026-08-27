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

    juce::TextButton validateButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FLStudioCollabAudioProcessorEditor)
};
