const { WebSocket } = require('ws');
const http = require('http');
const { server } = require('../src/server.js');

function httpGet(url) {
  return new Promise((resolve, reject) => {
    http.get(url, (res) => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => resolve({ statusCode: res.statusCode, data }));
    }).on('error', reject);
  });
}

async function runTest() {
  console.log('--- STARTING RELAY SERVER INTEGRATION TEST ---');
  const WS_URL = 'ws://localhost:8080';
  const HTTP_URL = 'http://localhost:8080';

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
          console.log(`[Test SUCCESS] Join URL: ${msg.joinUrl}`);
          roomCodeCreated = msg.roomCode;
          resolve();
        }
      });

      client1.on('error', reject);
    });

    if (!roomCodeCreated) {
      throw new Error('Failed to obtain room code from Client 1');
    }

    // 2. Test HTTP Web Invitation Page & Room Status API
    console.log('[Test] Testing HTTP Web Invitation page GET /join/' + roomCodeCreated);
    const webRes = await httpGet(`${HTTP_URL}/join/${roomCodeCreated}`);
    if (webRes.statusCode === 200 && webRes.data.includes(roomCodeCreated)) {
      console.log('[Test SUCCESS] Web Invitation Page rendered correctly with room code!');
    } else {
      throw new Error('Web Invitation Page test failed');
    }

    // 3. Client 2 connects and joins the room
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

    // 4. Test Draft Activity Indicator (PEER_TYPING)
    await new Promise((resolve, reject) => {
      client2.on('message', (data) => {
        const msg = JSON.parse(data.toString());
        if (msg.type === 'PEER_TYPING' && msg.isComposing) {
          console.log('[Test SUCCESS] Client 2 received PEER_TYPING activity indicator from Client 1!');
          resolve();
        }
      });

      console.log('[Test] Client 1 sending DRAFT_ACTIVITY signal...');
      client1.send(JSON.stringify({
        type: 'DRAFT_ACTIVITY',
        isComposing: true
      }));
    });

    // 5. Client 1 sends MIDI validation payload to Client 2
    const testMidiPayload = {
      notes: [
        { noteNumber: 60, velocity: 0.8, sampleOffset: 0, lengthSamples: 44100 },
        { noteNumber: 64, velocity: 0.9, sampleOffset: 22050, lengthSamples: 44100 }
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

    // 6. Test Audio Render Validation Push (VALIDATE_AUDIO)
    const testAudioPayload = {
      trackName: 'Piste 1: Drums Render',
      sampleRate: 44100,
      numChannels: 2,
      samplesLength: 512,
      pcmDataBase64: 'AAAAAEAAAAA='
    };

    await new Promise((resolve, reject) => {
      client2.on('message', (data) => {
        const msg = JSON.parse(data.toString());
        if (msg.type === 'AUDIO_RECEIVED') {
          console.log('[Test SUCCESS] Client 2 received AUDIO validation payload from Client 1!');
          console.log('Received audio track name:', msg.payload.trackName);
          resolve();
        }
      });

      console.log('[Test] Client 1 sending VALIDATE_AUDIO push...');
      client1.send(JSON.stringify({
        type: 'VALIDATE_AUDIO',
        payload: testAudioPayload
      }));
    });

    console.log('===================================================');
    console.log('🎉 ALL RELAY SERVER INTEGRATION, MIDI & AUDIO RENDER TESTS PASSED CLEANLY!');
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
