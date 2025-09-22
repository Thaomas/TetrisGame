const WebSocket = require('ws');
const http = require('http');
const url = require('url');
const fs = require('fs');
const path = require('path');

// Store rooms by code
const rooms = {};

// Create HTTP server (serve static + upgrade requests)
const server = http.createServer((req, res) => {
    // Serve static files from this directory
    if (req.method !== 'GET') {
        res.writeHead(405);
        res.end('Method Not Allowed');
        return;
    }

    let requestedPath = url.parse(req.url).pathname || '/';
    if (requestedPath === '/') requestedPath = '/index.html';

    const filePath = path.join(__dirname, requestedPath);

    // Prevent path traversal
    if (!filePath.startsWith(__dirname)) {
        res.writeHead(403);
        res.end('Forbidden');
        return;
    }

    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404);
            res.end('Not Found');
            return;
        }
        const ext = path.extname(filePath).toLowerCase();
        const mime = {
            '.html': 'text/html; charset=utf-8',
            '.js': 'application/javascript; charset=utf-8',
            '.css': 'text/css; charset=utf-8',
            '.json': 'application/json; charset=utf-8'
        }[ext] || 'application/octet-stream';

        res.writeHead(200, { 'Content-Type': mime, 'Cross-Origin-Opener-Policy': 'same-origin', 'Cross-Origin-Embedder-Policy': 'require-corp' });
        res.end(data);
    });
});

const PacketType = {
    GAME_STATE_UPDATE: 0,
    GAME_SCORE_UPDATE: 1,
    GAME_BUFFER_UPDATE: 2,
    GAME_END: 3,
    GAME_START: 4
};


const wss = new WebSocket.Server({ noServer: true });

// Example WebSocket connection string for clients:
// ws://localhost:8080/?code=ROOMCODE
// Replace ROOMCODE with your desired room identifier.
wss.on('connection', function connection(ws, request, roomCode) {
    if (!roomCode) {
        // Generate a random 5-digit numeric room code
        roomCode = Math.floor(10000 + Math.random() * 90000).toString();
        ws.send(JSON.stringify({ type: 'roomCode', code: roomCode }));
    }
        
    if (!rooms[roomCode]) {
        rooms[roomCode] = [];
    }
    rooms[roomCode].push(ws);
    
    ws.on('message', function incoming(data, isBinary) {
        // Print the time (in ISO format) and the number of bytes received since the last message from this client.
        if (!ws._lastMessageTime) {
            ws._lastMessageTime = Date.now();
        }
        const now = Date.now();
        const since = now - ws._lastMessageTime;
        console.log(`[${new Date().toISOString()}] message received from client in room ${roomCode} (${since} ms since last message)`);
        ws._lastMessageTime = now;
        if (!isBinary) {
            return;
        }


        const packetType = data[0];
        const packetData = data.slice(1);
        
        switch (packetType) {
            case PacketType.GAME_STATE_UPDATE:
                try {
                    let out = '+----------+'; // 10 dashes between borders
                    for (let r = 0; r < 22; r++) {
                        out += '\n|';
                        for (let c = 0; c < 10; c++) {
                            out += String(packetData[r * 10 + c]);
                        }
                        out += '|';
                    }
                    out += '\n+----------+';
                    console.log(`[Room ${roomCode}] Grid (${packetData.length} bytes):\n${out}`);
                } catch (e) {
                    console.error('Failed to decode grid:', e);
                }
                break;
            case PacketType.GAME_SCORE_UPDATE:
                console.log(`[Room ${roomCode}] Score update: ${packetData}`);
                break;

            case PacketType.GAME_BUFFER_UPDATE:
                break;
            case PacketType.GAME_END:
                break;
            case PacketType.GAME_START:
                break;
        }

        // Broadcast to all clients in the same room except sender
        rooms[roomCode].forEach(client => {
            if (client !== ws && client.readyState === WebSocket.OPEN) {
                client.send(data, { binary: !!isBinary });
            }
        });
    });

    ws.on('close', function() {
        // Remove from room
        rooms[roomCode] = rooms[roomCode].filter(client => client !== ws);
        if (rooms[roomCode].length === 0) {
            delete rooms[roomCode];
        }
    });
});

server.on('upgrade', function upgrade(request, socket, head) {
    const parsedUrl = url.parse(request.url, true);
    const roomCode = parsedUrl.query.code;
    if (!roomCode) {
        socket.destroy();
        return;
    }
    wss.handleUpgrade(request, socket, head, function done(ws) {
        wss.emit('connection', ws, request, roomCode);
    });
});

const PORT = 8080;
server.listen(PORT, () => {
    console.log(`Server listening on http://localhost:${PORT}`);
});