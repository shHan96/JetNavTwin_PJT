// server.js
import express from 'express';
import { createServer } from 'vite';

const app = express();
const port = 80; // HTTP 포트

// Vite 미들웨어 설정
const vite = await createServer({
  server: { middlewareMode: 'ssr' }
});
app.use(vite.middlewares);

// 기본 라우트 설정
app.get('/', (req, res) => {
  res.send('Hello World!');
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);