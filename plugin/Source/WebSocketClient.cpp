#include "WebSocketClient.h"
#include "PluginProcessor.h"

WebSocketClient::WebSocketClient()
{
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
    {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            handleIncomingMessage(msg->str);
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            connected = true;
            juce::MessageManager::callAsync([this]()
            {
                if (onConnectionStatusChanged) onConnectionStatusChanged(true);
            });
        }
        else if (msg->type == ix::WebSocketMessageType::Close || msg->type == ix::WebSocketMessageType::Error)
        {
            connected = false;
            juce::MessageManager::callAsync([this, msg]()
            {
                if (onConnectionStatusChanged) onConnectionStatusChanged(false);
                if (msg->type == ix::WebSocketMessageType::Error && onError)
                {
                    onError("Erreur réseau: " + juce::String(msg->errorInfo.reason));
                }
            });
        }
    });
}

WebSocketClient::~WebSocketClient()
{
    disconnectFromServer();
}

void WebSocketClient::connectToServer(const juce::String& serverUrl)
{
    webSocket.setUrl(serverUrl.toStdString());
    webSocket.start();
}

void WebSocketClient::disconnectFromServer()
{
    webSocket.stop();
    connected = false;
}

void WebSocketClient::createRoom()
{
    if (!connected)
    {
        connectToServer();
    }
    
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", "CREATE_ROOM");
    
    juce::var jsonVar(obj.get());
    juce::String jsonStr = juce::JSON::toString(jsonVar);
    webSocket.send(jsonStr.toStdString());
}

void WebSocketClient::joinRoom(const juce::String& roomCode)
{
    if (!connected)
    {
        connectToServer();
    }

    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", "JOIN_ROOM");
    obj->setProperty("roomCode", roomCode.toUpperCase());

    juce::var jsonVar(obj.get());
    juce::String jsonStr = juce::JSON::toString(jsonVar);
    webSocket.send(jsonStr.toStdString());
}

void WebSocketClient::sendMidiValidation(const juce::String& trackName, const std::vector<CapturedMidiNote>& notes)
{
    if (!connected || currentRoomCode.isEmpty()) return;

    juce::DynamicObject::Ptr payloadObj = new juce::DynamicObject();
    payloadObj->setProperty("trackName", trackName);

    juce::Array<juce::var> notesArray;
    for (const auto& note : notes)
    {
        juce::DynamicObject::Ptr noteObj = new juce::DynamicObject();
        noteObj->setProperty("noteNumber", note.noteNumber);
        noteObj->setProperty("velocity", note.velocity);
        noteObj->setProperty("sampleOffset", note.sampleOffset);
        noteObj->setProperty("lengthSamples", note.lengthSamples);
        notesArray.add(juce::var(noteObj.get()));
    }
    payloadObj->setProperty("notes", notesArray);

    juce::DynamicObject::Ptr mainObj = new juce::DynamicObject();
    mainObj->setProperty("type", "VALIDATE_MIDI");
    mainObj->setProperty("roomCode", currentRoomCode);
    mainObj->setProperty("payload", juce::var(payloadObj.get()));

    juce::var jsonVar(mainObj.get());
    juce::String jsonStr = juce::JSON::toString(jsonVar);
    webSocket.send(jsonStr.toStdString());
}

void WebSocketClient::handleIncomingMessage(const std::string& messageStr)
{
    juce::String juceStr (messageStr);
    juce::var parsed = juce::JSON::parse(juceStr);
    
    if (!parsed.isObject()) return;

    juce::String type = parsed["type"].toString();

    if (type == "ROOM_CREATED")
    {
        currentRoomCode = parsed["roomCode"].toString();
        juce::String joinUrl = parsed["joinUrl"].toString();
        juce::MessageManager::callAsync([this, joinUrl]()
        {
            if (onRoomCreated) onRoomCreated(currentRoomCode, joinUrl);
        });
    }
    else if (type == "ROOM_JOINED")
    {
        currentRoomCode = parsed["roomCode"].toString();
        int count = (int) parsed["peerCount"];
        juce::MessageManager::callAsync([this, count]()
        {
            if (onRoomJoined) onRoomJoined(currentRoomCode, count);
        });
    }
    else if (type == "PEER_JOINED")
    {
        juce::String peerId = parsed["peerId"].toString();
        int count = (int) parsed["peerCount"];
        juce::MessageManager::callAsync([this, peerId, count]()
        {
            if (onPeerJoined) onPeerJoined(peerId, count);
        });
    }
    else if (type == "PEER_LEFT")
    {
        juce::String peerId = parsed["peerId"].toString();
        int count = (int) parsed["peerCount"];
        juce::MessageManager::callAsync([this, peerId, count]()
        {
            if (onPeerLeft) onPeerLeft(peerId, count);
        });
    }
    else if (type == "MIDI_RECEIVED")
    {
        juce::var payload = parsed["payload"];
        juce::var notesArray = payload["notes"];
        
        std::vector<CapturedMidiNote> receivedNotes;
        if (notesArray.isArray())
        {
            for (int i = 0; i < notesArray.size(); ++i)
            {
                juce::var n = notesArray[i];
                CapturedMidiNote note;
                note.noteNumber = (int) n["noteNumber"];
                note.velocity = (float) n["velocity"];
                note.sampleOffset = (int) n["sampleOffset"];
                note.lengthSamples = (int) n["lengthSamples"];
                receivedNotes.push_back(note);
            }
        }

        juce::MessageManager::callAsync([this, receivedNotes]()
        {
            if (onMidiReceived) onMidiReceived(receivedNotes);
        });
    }
    else if (type == "ERROR")
    {
        juce::String errorMsg = parsed["message"].toString();
        juce::MessageManager::callAsync([this, errorMsg]()
        {
            if (onError) onError(errorMsg);
        });
    }
}
