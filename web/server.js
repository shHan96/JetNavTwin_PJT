// server.js
import express from 'express';
import { createServer } from 'http';
import { WebSocket, WebSocketServer } from 'ws';
import { fileURLToPath } from 'url';
import { dirname } from 'path';
import path from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const server = createServer(app);
const port = 3000;

// Add upgrade handling
server.on('upgrade', (request, socket, head) => {
  console.log('Upgrade request received:', request.url);
});

// Create WebSocket server
const wss = new WebSocketServer({ 
  server,
  handleProtocols: () => 'websocket' // Force websocket protocol
});

// WebSocket handlers with debug logging
wss.on('connection', (ws, req) => {
  console.log('Client connected from:', req.socket.remoteAddress);
  console.log('Connection headers:', req.headers);

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message);
      // console.log('Received:', data);
      
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(JSON.stringify(data));
        }
      });
    } catch (error) {
      console.error('Message processing error:', error);
    }
  });

  ws.on('close', () => {
    console.log('Client disconnected');
  });
});

// Static files
app.use(express.static(path.join(__dirname, 'dist')));

app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

server.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
  console.log('WebSocket server ready');
});