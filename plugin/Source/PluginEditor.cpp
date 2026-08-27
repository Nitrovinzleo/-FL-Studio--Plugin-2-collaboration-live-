#include "PluginProcessor.h"
#include "PluginEditor.h"

FLStudioCollabAudioProcessorEditor::FLStudioCollabAudioProcessorEditor (FLStudioCollabAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (420, 320);

    // Header Title
    headerTitleLabel.setText ("FL COLLAB LIVE", juce::dontSendNotification);
    headerTitleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    headerTitleLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (240, 240, 245));
    headerTitleLabel.setJustificationType (juce::Justification::left);
    addAndMakeVisible (headerTitleLabel);

    // Status Badge
    statusBadgeLabel.setText ("DECONNECTE", juce::dontSendNotification);
    statusBadgeLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (255, 90, 90));
    statusBadgeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusBadgeLabel);

    // Room Code Label & Input
    roomCodePromptLabel.setText ("Code de salon (ex: XK4R-92) :", juce::dontSendNotification);
    roomCodePromptLabel.setFont (juce::FontOptions (13.0f));
    roomCodePromptLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (180, 185, 200));
    addAndMakeVisible (roomCodePromptLabel);

    roomCodeInput.setTextToShowWhenEmpty ("EX: XK4R-92", juce::Colour::fromRGB (120, 125, 140));
    roomCodeInput.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    roomCodeInput.setColour (juce::TextEditor::backgroundColourId, juce::Colour::fromRGB (25, 28, 36));
    roomCodeInput.setColour (juce::TextEditor::outlineColourId, juce::Colour::fromRGB (50, 55, 70));
    roomCodeInput.setColour (juce::TextEditor::textColourId, juce::Colour::fromRGB (255, 255, 255));
    addAndMakeVisible (roomCodeInput);

    // Buttons
    createRoomButton.setButtonText ("Créer une session");
    createRoomButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (45, 50, 65));
    createRoomButton.onClick = [this]
    {
        // Simulated room creation action
        juce::String mockCode = "XK4R-92";
        audioProcessor.setRoomCode (mockCode);
        audioProcessor.setConnectedState (true);
        roomCodeInput.setText (mockCode);
        statusBadgeLabel.setText ("SALON ACTIVE (" + mockCode + ")", juce::dontSendNotification);
        statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (80, 220, 120));
    };
    addAndMakeVisible (createRoomButton);

    joinRoomButton.setButtonText ("Rejoindre");
    joinRoomButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (55, 60, 80));
    joinRoomButton.onClick = [this]
    {
        juce::String enteredCode = roomCodeInput.getText().toUpperCase();
        if (enteredCode.isNotEmpty())
        {
            audioProcessor.setRoomCode (enteredCode);
            audioProcessor.setConnectedState (true);
            statusBadgeLabel.setText ("CONNECTE (" + enteredCode + ")", juce::dontSendNotification);
            statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (80, 220, 120));
        }
    };
    addAndMakeVisible (joinRoomButton);

    // Main Action Button: VALIDER PATTERN
    validateButton.setButtonText ("VALIDER PATTERN (PUSH MIDI)");
    validateButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (110, 45, 220)); // Vibrant purple accent
    validateButton.setColour (juce::TextButton::textColourOffId, juce::Colour::fromRGB (255, 255, 255));
    validateButton.onClick = [this]
    {
        audioProcessor.validateAndSendCurrentPattern();
    };
    addAndMakeVisible (validateButton);
}

FLStudioCollabAudioProcessorEditor::~FLStudioCollabAudioProcessorEditor()
{
}

void FLStudioCollabAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Sleek dark background with subtle gradient
    juce::ColourGradient bgGradient (
        juce::Colour::fromRGB (18, 20, 26), 0.0f, 0.0f,
        juce::Colour::fromRGB (10, 11, 15), 0.0f, (float) getHeight(), false
    );
    g.setGradientFill (bgGradient);
    g.fillAll();

    // Header divider line
    g.setColour (juce::Colour::fromRGB (40, 44, 58));
    g.drawLine (15.0f, 48.0f, (float) getWidth() - 15.0f, 48.0f, 1.0f);
}

void FLStudioCollabAudioProcessorEditor::resized()
{
    headerTitleLabel.setBounds (15, 10, 200, 30);
    statusBadgeLabel.setBounds (getWidth() - 160, 12, 145, 26);

    roomCodePromptLabel.setBounds (20, 65, 380, 20);
    roomCodeInput.setBounds (20, 90, 380, 38);

    createRoomButton.setBounds (20, 140, 180, 40);
    joinRoomButton.setBounds (210, 140, 190, 40);

    validateButton.setBounds (20, 210, 380, 70);
}
