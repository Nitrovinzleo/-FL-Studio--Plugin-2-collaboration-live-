const { WebSocketServer } = require('ws');
const http = require('http');

const PORT = process.env.PORT || 8080;
const ROOM_MAX_INACTIVE_MS = 24 * 60 * 60 * 1000; // 24 Hours

// Room storage: roomCode -> { clients: Set<WebSocket>, lastActivity: number }
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
  const roomObj = rooms.get(roomCode);
  if (!roomObj) return;

  roomObj.lastActivity = Date.now();
  for (const client of roomObj.clients) {
    if (client !== senderWs && client.readyState === 1) { // WebSocket.OPEN
      client.send(JSON.stringify(messageData));
    }
  }
}

// Automatic background cleanup of inactive rooms (> 24h)
setInterval(() => {
  const now = Date.now();
  for (const [code, roomObj] of rooms.entries()) {
    if (now - roomObj.lastActivity > ROOM_MAX_INACTIVE_MS) {
      console.log(`[x] Expiration 24h: Cleaning up inactive room ${code}`);
      for (const ws of roomObj.clients) {
        ws.send(JSON.stringify({ type: 'ROOM_EXPIRED', message: 'Le salon a expiré (24h d\'inactivité).' }));
        ws.close();
      }
      rooms.delete(code);
    }
  }
}, 60 * 60 * 1000); // Check hourly

// HTTP Server handling web invitation page and API routes
const server = http.createServer((req, res) => {
  const urlParts = req.url.split('?')[0].split('/');

  // Health check
  if (req.url === '/health') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok', activeRooms: rooms.size }));
    return;
  }

  // Room status API: /api/room/:roomCode
  if (urlParts[1] === 'api' && urlParts[2] === 'room' && urlParts[3]) {
    const code = urlParts[3].toUpperCase();
    const exists = rooms.has(code);
    const roomObj = exists ? rooms.get(code) : null;
    const peerCount = roomObj ? roomObj.clients.size : 0;
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ roomCode: code, exists, peerCount, isFull: peerCount >= 2 }));
    return;
  }

  // Web invitation page: /join/:roomCode
  if (urlParts[1] === 'join' && urlParts[2]) {
    const roomCode = urlParts[2].toUpperCase();
    const html = `<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Rejoindre la Session FL Studio - ${roomCode}</title>
  <style>
    :root {
      --bg-gradient: linear-gradient(135deg, #0f1117 0%, #1a1d28 100%);
      --card-bg: rgba(25, 29, 41, 0.85);
      --accent: #8b5cf6;
      --accent-hover: #7c3aed;
      --text: #f3f4f6;
      --text-muted: #9ca3af;
      --border: #374151;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      background: var(--bg-gradient);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    .card {
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 36px;
      max-width: 460px;
      width: 100%;
      text-align: center;
      box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.5);
    }
    .logo {
      font-size: 28px;
      font-weight: 800;
      background: linear-gradient(90deg, #a78bfa, #f472b6);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      margin-bottom: 8px;
    }
    .subtitle {
      color: var(--text-muted);
      font-size: 14px;
      margin-bottom: 28px;
    }
    .code-box {
      background: #11131b;
      border: 2px dashed var(--accent);
      border-radius: 12px;
      padding: 20px;
      font-size: 36px;
      font-weight: 800;
      letter-spacing: 4px;
      color: #a78bfa;
      margin-bottom: 24px;
      user-select: all;
    }
    .btn-copy {
      background: var(--accent);
      color: #fff;
      border: none;
      padding: 14px 28px;
      border-radius: 10px;
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      width: 100%;
      transition: background 0.2s, transform 0.1s;
    }
    .btn-copy:hover { background: var(--accent-hover); }
    .btn-copy:active { transform: scale(0.98); }
    .instructions {
      margin-top: 28px;
      text-align: left;
      border-top: 1px solid var(--border);
      padding-top: 20px;
    }
    .instructions h4 {
      font-size: 14px;
      margin-bottom: 12px;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    .instructions ol {
      padding-left: 20px;
      font-size: 14px;
      color: #d1d5db;
      line-height: 1.6;
    }
    .toast {
      margin-top: 12px;
      font-size: 13px;
      color: #34d399;
      display: none;
    }
  </style>
</head>
<body>
  <div class="card">
    <div class="logo">FL COLLAB LIVE</div>
    <div class="subtitle">Invitation à rejoindre une session de collaboration MIDI / Audio</div>
    
    <div class="code-box" id="roomCode">${roomCode}</div>
    
    <button class="btn-copy" onclick="copyCode()">Copier le code de salon</button>
    <div class="toast" id="toast">✓ Code copié dans le presse-papier !</div>
    
    <div class="instructions">
      <h4>Comment rejoindre :</h4>
      <ol>
        <li>Copiez le code ci-dessus.</li>
        <li>Ouvrez votre projet dans <strong>FL Studio</strong>.</li>
        <li>Chargez le plugin <strong>FL Studio Collab</strong> sur votre piste.</li>
        <li>Collez le code dans le champ du plugin et cliquez sur <strong>Rejoindre</strong>.</li>
      </ol>
    </div>
  </div>

  <script>
    function copyCode() {
      const code = document.getElementById('roomCode').innerText.trim();
      navigator.clipboard.writeText(code).then(() => {
        const toast = document.getElementById('toast');
        toast.style.display = 'block';
        setTimeout(() => toast.style.display = 'none', 3000);
      });
    }
  </script>
</body>
</html>`;
    res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
    res.end(html);
    return;
  }

  res.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8' });
  res.end('Serveur relais FL Studio Collaboration Live actif.');
});

const wss = new WebSocketServer({ server });

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

        rooms.set(code, { clients: new Set([ws]), lastActivity: Date.now() });
        clientInfo.roomCode = code;

        ws.send(JSON.stringify({
          type: 'ROOM_CREATED',
          roomCode: code,
          clientId: clientInfo.clientId,
          peerCount: 1,
          joinUrl: `http://localhost:${PORT}/join/${code}`
        }));
        console.log(`[*] Room created: ${code} by client ${clientId}`);
        break;
      }

      case 'JOIN_ROOM': {
        const targetCode = (msg.roomCode || '').toUpperCase().trim();
        if (!rooms.has(targetCode)) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: `Salon '${targetCode}' introuvable.`
          }));
          return;
        }

        const roomObj = rooms.get(targetCode);
        if (roomObj.clients.size >= 2) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: `Le salon '${targetCode}' est complet (2 utilisateurs max).`
          }));
          return;
        }

        roomObj.clients.add(ws);
        roomObj.lastActivity = Date.now();
        clientInfo.roomCode = targetCode;

        // Notify client who joined
        ws.send(JSON.stringify({
          type: 'ROOM_JOINED',
          roomCode: targetCode,
          clientId: clientInfo.clientId,
          peerCount: roomObj.clients.size
        }));

        // Notify the existing peer in room
        broadcastToRoom(targetCode, ws, {
          type: 'PEER_JOINED',
          roomCode: targetCode,
          peerId: clientInfo.clientId,
          peerCount: roomObj.clients.size
        });

        console.log(`[+] Client ${clientId} joined room ${targetCode}. Total peers: ${roomObj.clients.size}`);
        break;
      }

      case 'DRAFT_ACTIVITY': {
        const currentRoom = clientInfo.roomCode;
        if (currentRoom && rooms.has(currentRoom)) {
          broadcastToRoom(currentRoom, ws, {
            type: 'PEER_TYPING',
            peerId: clientId,
            isComposing: !!msg.isComposing
          });
        }
        break;
      }

      case 'VALIDATE_MIDI': {
        const currentRoom = clientInfo.roomCode;
        if (!currentRoom || !rooms.has(currentRoom)) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: 'Vous n\'êtes pas dans un salon actif.'
          }));
          return;
        }

        const payload = msg.payload || {};
        console.log(`[MIDI] Validation push from ${clientId} in room ${currentRoom} (${(payload.notes || []).length} notes)`);

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

      case 'VALIDATE_AUDIO': {
        const currentRoom = clientInfo.roomCode;
        if (!currentRoom || !rooms.has(currentRoom)) {
          ws.send(JSON.stringify({
            type: 'ERROR',
            message: 'Vous n\'êtes pas dans un salon actif.'
          }));
          return;
        }

        const payload = msg.payload || {};
        console.log(`[AUDIO] Validation audio render push from ${clientId} in room ${currentRoom} (${payload.trackName || 'Piste Audio'})`);

        // Broadcast audio render payload to peer
        broadcastToRoom(currentRoom, ws, {
          type: 'AUDIO_RECEIVED',
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
      const roomObj = rooms.get(info.roomCode);
      roomObj.clients.delete(ws);
      roomObj.lastActivity = Date.now();
      console.log(`[-] Client ${info.clientId} left room ${info.roomCode}. Remaining: ${roomObj.clients.size}`);

      if (roomObj.clients.size === 0) {
        rooms.delete(info.roomCode);
        console.log(`[x] Room ${info.roomCode} destroyed (empty).`);
      } else {
        broadcastToRoom(info.roomCode, ws, {
          type: 'PEER_LEFT',
          roomCode: info.roomCode,
          peerId: info.clientId,
          peerCount: roomObj.clients.size
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
  console.log(`🌐 Invitation page available at: http://localhost:${PORT}/join/CODE`);
  console.log(`====================================================`);
});

module.exports = { server, wss, rooms };
