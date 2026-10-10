#!/usr/bin/env node
// Read-only: transport, version, game identity, CPU status.
import process from 'node:process';
import { getEndpointArg, checkTcp, PPSSPPClient, verifyGame } from './lib/ppsspp-client.mjs';

async function main() {
  if (process.argv.includes('--help')) {
    console.log('Usage: node tools/probes/ppsspp-smoke.mjs --endpoint HOST:PORT');
    return;
  }
  const url = getEndpointArg(process.argv.slice(2));
  const client = new PPSSPPClient(url);
  console.log(`Connecting to ${url.href} ...`);
  try {
    await checkTcp(url);
    console.log('TCP: OK');
    await client.connect();
    console.log('WebSocket: OK');
    const info = await verifyGame(client);
    console.log('PPSSPP version:', JSON.stringify(info.version, null, 2));
    console.log('Game:', JSON.stringify(info.gameStatus.game, null, 2));
    console.log('CPU:', JSON.stringify(info.cpu, null, 2));
    console.log('SUCCESS: ULUS10466 debugger reachable; no memory was modified.');
  } finally {
    await client.close();
  }
}

main().catch(error => { console.error('SMOKE FAILED:', error.message); process.exitCode = 1; });
