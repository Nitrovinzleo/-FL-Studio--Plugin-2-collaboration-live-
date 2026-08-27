# FL Studio Collaboration Relay Server

Node.js WebSocket relay server handling real-time room pairing and MIDI payload distribution between FL Studio plugin instances.

## Protocol Specification

### Client -> Server Messages

#### 1. Create Room
```json
{
  "type": "CREATE_ROOM"
}
```

#### 2. Join Room
```json
{
  "type": "JOIN_ROOM",
  "roomCode": "XK4R-92"
}
```

#### 3. Validate MIDI Pattern (Push)
```json
{
  "type": "VALIDATE_MIDI",
  "payload": {
    "trackName": "Synth Lead",
    "tempoBpm": 128.0,
    "notes": [
      { "noteNumber": 60, "velocity": 100, "timestampSample": 0, "durationSamples": 22050 }
    ]
  }
}
```

### Server -> Client Messages

#### 1. Room Created
```json
{
  "type": "ROOM_CREATED",
  "roomCode": "XK4R-92",
  "clientId": "abc1234",
  "peerCount": 1
}
```

#### 2. Room Joined
```json
{
  "type": "ROOM_JOINED",
  "roomCode": "XK4R-92",
  "clientId": "xyz5678",
  "peerCount": 2
}
```

#### 3. MIDI Received (Forwarded to peer)
```json
{
  "type": "MIDI_RECEIVED",
  "senderId": "abc1234",
  "payload": { ... },
  "timestamp": 1693123456789
}
```
