#include "PluginProcessor.h"
#include "PluginEditor.h"

FLStudioCollabAudioProcessorEditor::FLStudioCollabAudioProcessorEditor (FLStudioCollabAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (440, 360);

    // Header Title
    headerTitleLabel.setText ("FL COLLAB LIVE", juce::dontSendNotification);
    headerTitleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    headerTitleLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (240, 240, 245));
    headerTitleLabel.setJustificationType (juce::Justification::left);
    addAndMakeVisible (headerTitleLabel);

    // Status Badge
    statusBadgeLabel.setText ("HORS LIGNE", juce::dontSendNotification);
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

    // Log & Info Label
    logLabel.setText ("Prêt. Entrez un code ou créez un salon.", juce::dontSendNotification);
    logLabel.setFont (juce::FontOptions (12.0f));
    logLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (140, 145, 160));
    logLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (logLabel);

    // Buttons
    createRoomButton.setButtonText ("Créer une session");
    createRoomButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (45, 50, 65));
    createRoomButton.onClick = [this]
    {
        logLabel.setText ("Création du salon en cours...", juce::dontSendNotification);
        audioProcessor.getWebSocketClient().createRoom();
    };
    addAndMakeVisible (createRoomButton);

    joinRoomButton.setButtonText ("Rejoindre");
    joinRoomButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (55, 60, 80));
    joinRoomButton.onClick = [this]
    {
        juce::String enteredCode = roomCodeInput.getText().toUpperCase();
        if (enteredCode.isNotEmpty())
        {
            logLabel.setText ("Connexion au salon " + enteredCode + "...", juce::dontSendNotification);
            audioProcessor.getWebSocketClient().joinRoom(enteredCode);
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
        logLabel.setText ("✓ Pattern MIDI validé et envoyé !", juce::dontSendNotification);
    };
    addAndMakeVisible (validateButton);

    setupWebSocketCallbacks();
}

FLStudioCollabAudioProcessorEditor::~FLStudioCollabAudioProcessorEditor()
{
}

void FLStudioCollabAudioProcessorEditor::setupWebSocketCallbacks()
{
    auto& ws = audioProcessor.getWebSocketClient();

    ws.onRoomCreated = [this](const juce::String& code, const juce::String& joinUrl)
    {
        roomCodeInput.setText (code);
        statusBadgeLabel.setText ("SALON (" + code + " - 1/2)", juce::dontSendNotification);
        statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (255, 180, 50));
        logLabel.setText ("Salon créé ! Lien d'invitation: " + joinUrl, juce::dontSendNotification);
    };

    ws.onRoomJoined = [this](const juce::String& code, int peerCount)
    {
        roomCodeInput.setText (code);
        statusBadgeLabel.setText ("CONNECTÉ (" + code + " - " + juce::String(peerCount) + "/2)", juce::dontSendNotification);
        statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (80, 220, 120));
        logLabel.setText ("Rejoint le salon " + code, juce::dontSendNotification);
    };

    ws.onPeerJoined = [this](const juce::String& peerId, int peerCount)
    {
        statusBadgeLabel.setText ("CONNECTÉ (2/2)", juce::dontSendNotification);
        statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (80, 220, 120));
        logLabel.setText ("Un collaborateur a rejoint la session !", juce::dontSendNotification);
    };

    ws.onPeerLeft = [this](const juce::String& peerId, int peerCount)
    {
        statusBadgeLabel.setText ("EN ATTENTE (1/2)", juce::dontSendNotification);
        statusBadgeLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (255, 180, 50));
        logLabel.setText ("Le collaborateur s'est déconnecté.", juce::dontSendNotification);
    };

    ws.onError = [this](const juce::String& errorMsg)
    {
        logLabel.setText ("⚠️ " + errorMsg, juce::dontSendNotification);
    };
}

void FLStudioCollabAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Sleek dark background gradient
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
    statusBadgeLabel.setBounds (getWidth() - 180, 12, 165, 26);

    roomCodePromptLabel.setBounds (20, 60, 400, 20);
    roomCodeInput.setBounds (20, 82, 400, 38);

    createRoomButton.setBounds (20, 130, 190, 40);
    joinRoomButton.setBounds (220, 130, 200, 40);

    logLabel.setBounds (20, 182, 400, 22);

    validateButton.setBounds (20, 215, 400, 75);
}
