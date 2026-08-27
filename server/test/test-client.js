const { WebSocket } = require('ws');
const { server } = require('../src/server.js');

async function runTest() {
  console.log('--- STARTING RELAY SERVER INTEGRATION TEST ---');
  const WS_URL = 'ws://localhost:8080';

  let client1, client2;
  let roomCodeCreated = null;

  try {
    // 1. Client 1 connects and creates a room
    client1 = new WebSocket(WS_URL);

    await new Promise((resolve, reject) => {
      client1.on('open', () => {
        console.log('[Test] Client 1 connected');
        client1.send(JSON.stringify({ type: 'CREATE_ROOM' }));
      });

      client1.on('message', (data) => {
        const msg = JSON.parse(data.toString());
        if (msg.type === 'ROOM_CREATED') {
          console.log(`[Test SUCCESS] Room created with code: ${msg.roomCode}`);
          roomCodeCreated = msg.roomCode;
          resolve();
        }
      });

      client1.on('error', reject);
    });

    if (!roomCodeCreated) {
      throw new Error('Failed to obtain room code from Client 1');
    }

    // 2. Client 2 connects and joins the room
    client2 = new WebSocket(WS_URL);

    let client1NotifiedOfPeer = false;
    let client2Joined = false;

    await new Promise((resolve, reject) => {
      client1.on('message', (data) => {
        const msg = JSON.parse(data.toString());
        if (msg.type === 'PEER_JOINED') {
          console.log('[Test SUCCESS] Client 1 received PEER_JOINED notification');
          client1NotifiedOfPeer = true;
          if (client2Joined && client1NotifiedOfPeer) resolve();
        }
      });

      client2.on('open', () => {
        console.log('[Test] Client 2 connected, joining room:', roomCodeCreated);
        client2.send(JSON.stringify({
          type: 'JOIN_ROOM',
          roomCode: roomCodeCreated
        }));
      });

      client2.on('message', (data) => {
        const msg = JSON.parse(data.toString());
        if (msg.type === 'ROOM_JOINED') {
          console.log('[Test SUCCESS] Client 2 joined room:', msg.roomCode);
          client2Joined = true;
          if (client2Joined && client1NotifiedOfPeer) resolve();
        }
      });

      client2.on('error', reject);
    });

    // 3. Client 1 sends MIDI validation payload to Client 2
    const testMidiPayload = {
      notes: [
        { noteNumber: 60, velocity: 100, timestampSample: 0, durationSamples: 44100 },
        { noteNumber: 64, velocity: 90, timestampSample: 22050, durationSamples: 44100 }
      ],
      tempoBpm: 128.0,
      trackName: 'Lead Synth Private Draft'
    };

    await new Promise((resolve, reject) => {
      client2.on('message', (data) => {
        const msg = JSON.parse(data.toString());
        if (msg.type === 'MIDI_RECEIVED') {
          console.log('[Test SUCCESS] Client 2 received MIDI validation payload from Client 1!');
          console.log('Received track name:', msg.payload.trackName);
          console.log('Received notes count:', msg.payload.notes.length);
          if (msg.payload.notes.length === 2 && msg.payload.notes[0].noteNumber === 60) {
            resolve();
          } else {
            reject(new Error('MIDI payload mismatch'));
          }
        }
      });

      console.log('[Test] Client 1 sending VALIDATE_MIDI push...');
      client1.send(JSON.stringify({
        type: 'VALIDATE_MIDI',
        payload: testMidiPayload
      }));
    });

    console.log('===================================================');
    console.log('🎉 ALL RELAY SERVER INTEGRATION TESTS PASSED CLEANLY!');
    console.log('===================================================');
  } catch (err) {
    console.error('❌ Test failed with error:', err);
    process.exitCode = 1;
  } finally {
    if (client1) client1.close();
    if (client2) client2.close();
    server.close();
  }
}

runTest();
