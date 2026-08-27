#include "WebSocketClient.h"
#include "PluginProcessor.h"

WebSocketClient::WebSocketClient()
{
    webSocket.enableAutomaticReconnection();
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
            ix::WebSocketMessageType msgType = msg->type;
            juce::String errReason = juce::String(msg->errorInfo.reason);
            juce::MessageManager::callAsync([this, msgType, errReason]()
            {
                if (onConnectionStatusChanged) onConnectionStatusChanged(false);
                if (msgType == ix::WebSocketMessageType::Error && onError)
                {
                    onError("Erreur réseau: " + errReason);
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
    lastServerUrl = serverUrl;
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
        connectToServer(lastServerUrl);
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
        connectToServer(lastServerUrl);
    }

    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", "JOIN_ROOM");
    obj->setProperty("roomCode", roomCode.toUpperCase());

    juce::var jsonVar(obj.get());
    juce::String jsonStr = juce::JSON::toString(jsonVar);
    webSocket.send(jsonStr.toStdString());
}

void WebSocketClient::sendDraftActivity(bool isComposing)
{
    if (!connected || currentRoomCode.isEmpty()) return;

    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", "DRAFT_ACTIVITY");
    obj->setProperty("isComposing", isComposing);

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

void WebSocketClient::sendAudioValidation(const juce::String& trackName, const juce::AudioBuffer<float>& audioBuffer, double sampleRate)
{
    if (!connected || currentRoomCode.isEmpty()) return;

    juce::MemoryOutputStream memStream;
    juce::WavAudioFormat wavFormat;
    
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(&memStream, sampleRate, (unsigned int) audioBuffer.getNumChannels(), 16, {}, 0)
    );

    if (writer != nullptr)
    {
        writer->writeFromAudioSampleBuffer(audioBuffer, 0, audioBuffer.getNumSamples());
        writer.reset(); // Flush writer

        juce::String base64Wav = juce::Base64::toBase64(memStream.getData(), memStream.getDataSize());

        juce::DynamicObject::Ptr payloadObj = new juce::DynamicObject();
        payloadObj->setProperty("trackName", trackName);
        payloadObj->setProperty("sampleRate", sampleRate);
        payloadObj->setProperty("numChannels", audioBuffer.getNumChannels());
        payloadObj->setProperty("samplesLength", audioBuffer.getNumSamples());
        payloadObj->setProperty("pcmDataBase64", base64Wav);

        juce::DynamicObject::Ptr mainObj = new juce::DynamicObject();
        mainObj->setProperty("type", "VALIDATE_AUDIO");
        mainObj->setProperty("roomCode", currentRoomCode);
        mainObj->setProperty("payload", juce::var(payloadObj.get()));

        juce::var jsonVar(mainObj.get());
        juce::String jsonStr = juce::JSON::toString(jsonVar);
        webSocket.send(jsonStr.toStdString());
    }
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
    else if (type == "PEER_TYPING")
    {
        juce::String peerId = parsed["peerId"].toString();
        bool isComposing = (bool) parsed["isComposing"];
        juce::MessageManager::callAsync([this, peerId, isComposing]()
        {
            if (onPeerTyping) onPeerTyping(peerId, isComposing);
        });
    }
    else if (type == "MIDI_RECEIVED")
    {
        juce::var payload = parsed["payload"];
        juce::String trackName = payload["trackName"].toString();
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

        juce::MessageManager::callAsync([this, receivedNotes, trackName]()
        {
            if (onMidiReceived) onMidiReceived(receivedNotes, trackName);
        });
    }
    else if (type == "AUDIO_RECEIVED")
    {
        juce::var payload = parsed["payload"];
        juce::String trackName = payload["trackName"].toString();
        juce::String base64Wav = payload["pcmDataBase64"].toString();
        
        juce::MemoryOutputStream memStream;
        if (juce::Base64::convertFromBase64(memStream, base64Wav))
        {
            juce::MemoryInputStream inputStream(memStream.getData(), memStream.getDataSize(), false);
            juce::WavAudioFormat wavFormat;
            std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(&inputStream, false));
            
            if (reader != nullptr)
            {
                juce::AudioBuffer<float> receivedBuffer((int) reader->numChannels, (int) reader->lengthInSamples);
                reader->read(&receivedBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

                juce::MessageManager::callAsync([this, receivedBuffer, trackName]()
                {
                    if (onAudioReceived) onAudioReceived(receivedBuffer, trackName);
                });
            }
        }
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
