const WebSocket = require('ws');
const http = require('http');
const url = require('url');

// Store rooms by code
const rooms = {};

// Create HTTP server (for upgrade requests)
const server = http.createServer();

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

    ws.on('message', function incoming(message) {
        // Broadcast to all clients in the same room except sender
        rooms[roomCode].forEach(client => {
            if (client !== ws && client.readyState === WebSocket.OPEN) {
                client.send(message);
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
    console.log(`WebSocket server listening on port ${PORT}`);
});