#pragma once

#include <JuceHeader.h>
#include <ixwebsocket/IXWebSocket.h>
#include <functional>
#include <string>

class WebSocketClient
{
public:
    WebSocketClient();
    ~WebSocketClient();

    void connectToServer(const juce::String& serverUrl = "wss://flstudio-collab.onrender.com");
    void disconnectFromServer();

    void createRoom();
    void joinRoom(const juce::String& roomCode);
    void sendDraftActivity(bool isComposing);
    void sendMidiValidation(const juce::String& trackName, const std::vector<struct CapturedMidiNote>& notes);
    void sendAudioValidation(const juce::String& trackName, const juce::AudioBuffer<float>& audioBuffer, double sampleRate);

    // Callbacks setup
    std::function<void(const juce::String& roomCode, const juce::String& joinUrl)> onRoomCreated;
    std::function<void(const juce::String& roomCode, int peerCount)> onRoomJoined;
    std::function<void(const juce::String& peerId, int peerCount)> onPeerJoined;
    std::function<void(const juce::String& peerId, int peerCount)> onPeerLeft;
    std::function<void(const juce::String& peerId, bool isComposing)> onPeerTyping;
    std::function<void(const std::vector<struct CapturedMidiNote>& notes, const juce::String& trackName)> onMidiReceived;
    std::function<void(const juce::AudioBuffer<float>& audioBuffer, const juce::String& trackName)> onAudioReceived;
    std::function<void(const juce::String& errorMsg)> onError;
    std::function<void(bool isConnected)> onConnectionStatusChanged;

    bool isConnected() const { return connected; }
    juce::String getCurrentRoomCode() const { return currentRoomCode; }

private:
    ix::WebSocket webSocket;
    bool connected = false;
    juce::String currentRoomCode = "";
    juce::String lastServerUrl = "wss://flstudio-collab.onrender.com";

    void handleIncomingMessage(const std::string& messageStr);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebSocketClient)
};
