import net from 'node:net';

const SUBPROTOCOL = 'debugger.ppsspp.org';
const TIMEOUT_MS = 7000;

export function debuggerUrl(endpoint) {
  if (!endpoint || !String(endpoint).trim()) {
    throw new Error('Missing --endpoint HOST:PORT (get the port from PPSSPP remote debugger).');
  }
  const value = String(endpoint).trim();
  const url = new URL(/^wss?:\/\//i.test(value) ? value : `ws://${value}`);
  if (!['ws:', 'wss:'].includes(url.protocol) || !url.hostname || !url.port) {
    throw new Error('Endpoint must be HOST:PORT or ws://HOST:PORT/debugger.');
  }
  const path = url.pathname.replace(/\/+$/, '');
  if (path && path !== '/debugger') throw new Error('Expected the WebSocket endpoint path /debugger.');
  url.pathname = '/debugger';
  url.search = '';
  url.hash = '';
  return url;
}

export function getEndpointArg(argv) {
  const index = argv.indexOf('--endpoint');
  if (index < 0 || !argv[index + 1]) {
    throw new Error('Usage: --endpoint HOST:PORT  (example: --endpoint 127.0.0.1:39100)');
  }
  return debuggerUrl(argv[index + 1]);
}

export async function checkTcp(url) {
  const port = Number(url.port);
  await new Promise((resolve, reject) => {
    const socket = net.createConnection({ host: url.hostname, port });
    let complete = false;
    const finish = (err) => {
      if (complete) return;
      complete = true;
      socket.destroy();
      err ? reject(err) : resolve();
    };
    socket.setTimeout(TIMEOUT_MS, () => finish(new Error('TCP connect timeout: check PPSSPP debug port and firewall')));
    socket.once('connect', () => finish());
    socket.once('error', (err) => finish(err));
  });
}

export class PPSSPPClient {
  constructor(url) {
    this.url = url;
    this.ws = null;
    this.ticket = 1;
    this.pending = new Map();
  }

  async connect() {
    if (typeof WebSocket === 'undefined') throw new Error('Node.js 22+ required (global WebSocket missing).');
    await new Promise((resolve, reject) => {
      const ws = new WebSocket(this.url.href, SUBPROTOCOL);
      let settled = false;
      const done = (error) => {
        if (settled) return;
        settled = true;
        clearTimeout(timer);
        if (error) { try { ws.close(); } catch {} reject(error); }
        else { this.ws = ws; resolve(); }
      };
      const timer = setTimeout(() => done(new Error('WebSocket handshake timeout')), TIMEOUT_MS);
      ws.addEventListener('open', () => {
        if (ws.protocol !== SUBPROTOCOL) {
          done(new Error(`Wrong debugger subprotocol: ${ws.protocol || '<empty>'}`));
        } else {
          done();
        }
      }, { once: true });
      ws.addEventListener('error', () => done(new Error('WebSocket upgrade failed. Verify /debugger and PPSSPP port.')));
      ws.addEventListener('close', () => done(new Error('WebSocket closed before handshake.')));
      ws.addEventListener('message', event => { void this.onMessage(event.data); });
      ws.addEventListener('close', () => {
        for (const p of this.pending.values()) {
          clearTimeout(p.timer);
          p.reject(new Error(`Connection closed while waiting for ${p.event}`));
        }
        this.pending.clear();
      });
    });
  }

  async onMessage(data) {
    let raw;
    try {
      if (typeof data === 'string') raw = data;
      else if (data instanceof ArrayBuffer) raw = Buffer.from(data).toString('utf8');
      else if (ArrayBuffer.isView(data)) raw = Buffer.from(data.buffer, data.byteOffset, data.byteLength).toString('utf8');
      else if (typeof data?.text === 'function') raw = await data.text();
      else raw = String(data);
      const result = JSON.parse(raw);
      if (result.ticket === undefined) return; // Ignore unsolicited debugger broadcasts.
      const key = Number(result.ticket);
      const p = this.pending.get(key);
      if (!p) return;
      this.pending.delete(key);
      clearTimeout(p.timer);
      if (result.event === 'error') p.reject(new Error(`${p.event}: ${result.message || 'PPSSPP error'}`));
      else p.resolve(result);
    } catch (error) {
      console.error('PPSSPP message decode warning:', error.message);
    }
  }

  request(event, params = {}, timeout = TIMEOUT_MS) {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) {
      return Promise.reject(new Error('Debugger WebSocket is not connected'));
    }
    const ticket = this.ticket++;
    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => {
        this.pending.delete(ticket);
        reject(new Error(`Timed out waiting for ${event} (ticket ${ticket})`));
      }, timeout);
      this.pending.set(ticket, { timer, event, resolve, reject });
      try { this.ws.send(JSON.stringify({ event, ...params, ticket })); }
      catch (err) {
        clearTimeout(timer);
        this.pending.delete(ticket);
        reject(err);
      }
    });
  }

  async close() {
    if (!this.ws || this.ws.readyState === WebSocket.CLOSED) return;
    await new Promise(resolve => {
      const timer = setTimeout(resolve, 500);
      this.ws.addEventListener('close', () => { clearTimeout(timer); resolve(); }, { once: true });
      try { this.ws.close(1000, 'Jin8 read-only probe complete'); }
      catch { clearTimeout(timer); resolve(); }
    });
  }
}

export async function verifyGame(client) {
  const version = await client.request('version', { name: 'tekken6-jin8-research', version: '0.1' });
  const gameStatus = await client.request('game.status');
  if (!gameStatus.game?.id) throw new Error('PPSSPP responded but no game is running.');
  if (gameStatus.game.id !== 'ULUS10466') {
    throw new Error(`Wrong game ID: ${gameStatus.game.id}; expected ULUS10466.`);
  }
  const cpu = await client.request('cpu.status');
  return { version, gameStatus, cpu };
}
