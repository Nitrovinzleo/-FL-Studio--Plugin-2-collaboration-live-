const { WebSocketServer } = require('ws');
const http = require('http');

const PORT = process.env.PORT || 8080;
const server = http.createServer((req, res) => {
  if (req.url === '/health') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok', activeRooms: rooms.size }));
    return;
  }
  res.writeHead(200, { 'Content-Type': 'text/plain' });
  res.end('FL Studio Collaboration Relay Server Running');
});

const wss = new WebSocketServer({ server });

// Room storage: roomCode -> Set of WebSocket clients
const rooms = new Map();
// Client metadata map: ws -> { roomCode, clientId }
const clients = new Map();

function generateRoomCode() {
  const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
  let code = '';
  for (let i = 0; i < 4; i++) {
    code += chars.charAt(Math.floor(Math.random() * chars.length));
  }
  code += '-';
  for (let i = 0; i < 2; i++) {
    code += chars.charAt(Math.floor(Math.random() * chars.length));
  }
  return code; // Format: XXXX-XX (e.g. XK4R-92)
}

function generateClientId() {
  return Math.random().toString(36).substring(2, 9);
}

function broadcastToRoom(roomCode, senderWs, messageData) {
  const roomClients = rooms.get(roomCode);
  if (!roomClients) return;

  for (const client of roomClients) {
    if (client !== senderWs && client.readyState === 1) { // WebSocket.OPEN
      client.send(JSON.stringify(messageData));
    }
  }
}

wss.on('connection', (ws) => {
  const clientId = generateClientId();
  clients.set(ws, { roomCode: null, clientId });
  console.log(`[+] Client connected: ${clientId}`);

  ws.on('message', (rawData) => {
    let msg;
    try {
      msg = JSON.parse(rawData.toString());
    } catch (err) {
      ws.send(JSON.stringify({ type: 'ERROR', message: 'Invalid JSON payload' }));
      return;
    }

    const clientInfo = clients.get(ws);

    switch (msg.type) {
      case 'CREATE_ROOM': {
        let code = generateRoomCode();
        while (rooms.has(code)) {
          code = generateRoomCode();
        }

        const roomSet = new Set([ws]);
        rooms.set(code, roomSet);
        clientInfo.roomCode = code;

        ws.send(JSON.stringify({
          type: 'ROOM_CREATED',
          roomCode: code,
          clientId: clientInfo.clientId,
          peerCount: 1
        }));
        console.log(`[*] Room created: ${code} by client ${clientId}`);
        break;
      }

      case 'JOIN_ROOM': {
        const targetCode = (msg.roomCode || '').toUpperCase().trim();
        if (!rooms.has(targetCode)) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: `Room code '${targetCode}' not found.`
          }));
          return;
        }

        const roomSet = rooms.get(targetCode);
        if (roomSet.size >= 2) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: `Room '${targetCode}' is full (max 2 users).`
          }));
          return;
        }

        roomSet.add(ws);
        clientInfo.roomCode = targetCode;

        // Notify client who joined
        ws.send(JSON.stringify({
          type: 'ROOM_JOINED',
          roomCode: targetCode,
          clientId: clientInfo.clientId,
          peerCount: roomSet.size
        }));

        // Notify the existing peer in room
        broadcastToRoom(targetCode, ws, {
          type: 'PEER_JOINED',
          roomCode: targetCode,
          peerId: clientInfo.clientId,
          peerCount: roomSet.size
        });

        console.log(`[+] Client ${clientId} joined room ${targetCode}. Total peers: ${roomSet.size}`);
        break;
      }

      case 'VALIDATE_MIDI': {
        const currentRoom = clientInfo.roomCode;
        if (!currentRoom || !rooms.has(currentRoom)) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: 'You are not in an active room.'
          }));
          return;
        }

        const payload = msg.payload || {};
        console.log(`[MIDI] Received validation push from ${clientId} in room ${currentRoom}`);

        // Broadcast to peer in room
        broadcastToRoom(currentRoom, ws, {
          type: 'MIDI_RECEIVED',
          senderId: clientId,
          payload,
          timestamp: Date.now()
        });

        // Acknowledge sender
        ws.send(JSON.stringify({
          type: 'VALIDATE_ACK',
          timestamp: Date.now()
        }));
        break;
      }

      case 'PING': {
        ws.send(JSON.stringify({ type: 'PONG', timestamp: Date.now() }));
        break;
      }

      default: {
        ws.send(JSON.stringify({ type: 'ERROR', message: `Unknown message type: ${msg.type}` }));
      }
    }
  });

  ws.on('close', () => {
    const info = clients.get(ws);
    if (info && info.roomCode && rooms.has(info.roomCode)) {
      const roomSet = rooms.get(info.roomCode);
      roomSet.delete(ws);
      console.log(`[-] Client ${info.clientId} left room ${info.roomCode}. Remaining: ${roomSet.size}`);

      if (roomSet.size === 0) {
        rooms.delete(info.roomCode);
        console.log(`[x] Room ${info.roomCode} destroyed (empty).`);
      } else {
        broadcastToRoom(info.roomCode, ws, {
          type: 'PEER_LEFT',
          roomCode: info.roomCode,
          peerId: info.clientId,
          peerCount: roomSet.size
        });
      }
    }
    clients.delete(ws);
    console.log(`[-] Client disconnected: ${clientId}`);
  });
});

server.listen(PORT, () => {
  console.log(`====================================================`);
  console.log(`🚀 FL Studio Collaboration Relay Server running on port ${PORT}`);
  console.log(`====================================================`);
});

module.exports = { server, wss, rooms };
