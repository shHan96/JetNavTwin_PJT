import express from 'express';
import { fileURLToPath } from 'url';
import { dirname } from 'path';
import path from 'path';
import { WebSocketServer } from 'ws';

const wss = new WebSocketServer({ 
  port: 8080,
  host: '0.0.0.0'  // 모든 인터페이스에서 접근 허용
 }); // Move this up

wss.on('connection', (ws) => {
  console.log('New client connected');

  ws.on('message', (message) => {
    try {
      const data = JSON.parse(message)
      console.log('Received data:', data);
      
      // Broadcast the message to all clients
      wss.clients.forEach((client) => {
        if (client.readyState === ws.OPEN) {
          client.send(JSON.stringify(data));
        }
      });
    } catch (error) {
      console.error('Error processing message:', error);
    }
  });

  ws.on('close', () => {
    console.log('Client disconnected');
  });
});

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const port = 80;

// Serve static files from the dist directory
app.use(express.static(path.join(__dirname, 'dist')));

app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});