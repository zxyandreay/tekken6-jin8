#!/usr/bin/env node
// Passive focused cinematic monitor: zero memory writes, breakpoints or CPU manipulation.
import fs from 'node:fs/promises';
import path from 'node:path';
import process from 'node:process';
import { PPSSPPClient, getEndpointArg, checkTcp, verifyGame } from './lib/ppsspp-client.mjs';
import { classifyCinematic } from './lib/cinematic-events.mjs';

const FIELDS = [
  ['currentMoveId', 0x08C07A18],
  ['incomingMoveId', 0x08C07D1C],
  ['inputState', 0x08C07D3C],
  ['speedMode', 0x08C52890],
  ['cameraRate', 0x08D6F270],
  ['cameraZoom', 0x08D6F224],
  ['slowmoTimer', 0x08C07CA0],
  ['slowmoState', 0x08C07C78],
  ['effectGate', 0x08C140B8],
];
const hex = x => '0x' + (x >>> 0).toString(16).toUpperCase().padStart(8, '0');
const sleep = n => new Promise(resolve => setTimeout(resolve, n));
function arg(args, name, fallback) { const i = args.indexOf(name); return i < 0 ? fallback : args[i + 1]; }
async function readWord(client, address) {
  try {
    const r = await client.request('memory.read_u32', { address });
    const value = r.value ?? r.uintValue;
    return Number.isInteger(value) ? { value: hex(value) } : { error: 'No integer value in PPSSPP response' };
  } catch (error) { return { error: error.message }; }
}
async function main() {
  const args = process.argv.slice(2);
  if (args.includes('--help')) {
    console.log('Usage: node tools/probes/watch-jin-cinematics.mjs --endpoint HOST:PORT --label practice-jin-cinematic [--samples 80] [--interval-ms 100] [--after-detection 8]');
    return;
  }
  const endpoint = getEndpointArg(args);
  const label = arg(args, '--label');
  if (!label || !/^[a-zA-Z0-9][a-zA-Z0-9_-]{0,63}$/.test(label)) throw Error('Set a new --label (1-64 letters/numbers/_/-).');
  const samples = Number(arg(args, '--samples', '80'));
  const interval = Number(arg(args, '--interval-ms', '100'));
  const afterDetection = Number(arg(args, '--after-detection', '8'));
  if (!Number.isInteger(samples) || samples < 1 || samples > 120) throw Error('--samples must be 1..120');
  if (!Number.isInteger(interval) || interval < 50 || interval > 1000) throw Error('--interval-ms must be 50..1000');
  if (!Number.isInteger(afterDetection) || afterDetection < 0 || afterDetection > 50) throw Error('--after-detection must be 0..50');

  const client = new PPSSPPClient(endpoint);
  try {
    await checkTcp(endpoint);
    await client.connect();
    const base = await verifyGame(client);
    console.log('Connected:', base.gameStatus.game.id, '| read-only | starting focused capture');
    const started = Date.now(), records = [];
    let firstEvent = null, prevMove = null;
    for (let i = 0; i < samples; i++) {
      const r = { index: i, elapsedMs: Date.now() - started, fields: {} };
      for (const [key,address] of FIELDS) r.fields[key] = await readWord(client, address);
      r.cinematic = classifyCinematic(r.fields);
      records.push(r);
      const move = r.fields.currentMoveId.value;
      if (r.cinematic.trigger && firstEvent === null) firstEvent = i;
      if (r.cinematic.trigger || i === 0 || move !== prevMove) {
        console.log('t=' + r.elapsedMs + 'ms move=' + (move ?? '?')
           + ' speed=' + (r.cinematic.speedMode ?? '?')
           + ' zoom=' + (r.cinematic.zoom === null ? '?' : r.cinematic.zoom.toFixed(2))
           + (r.cinematic.trigger ? ' EVENT ' + r.cinematic.flags.join(',') : ''));
      }
      prevMove = move;
      if (firstEvent !== null && i >= firstEvent + afterDetection) break;
      if (i + 1 < samples) await sleep(interval);
    }
    const summary = {
      samplesTaken: records.length, elapsedMs: Date.now() - started,
      detectedEvents: records.filter(r => r.cinematic.trigger).length,
      firstDetectedSample: firstEvent, detectionCriteria: 'speed mode 3 OR camera zoom between 0 and 40',
    };
    const output = {
      schema: 'jin8-cinematic-monitor-v1', label, capturedAt: new Date().toISOString(),
      game: base.gameStatus.game, version: base.version, cpuAtStart: base.cpu,
      note: 'Read-only, sequential; matching values are not proof of visually perceptible slowdown or its game trigger.',
      summary, records,
    };
    const filename = path.join('captures', label + '.json');
    await fs.mkdir(path.dirname(filename), { recursive: true });
    await fs.writeFile(filename, JSON.stringify(output, null, 2) + '\n', { flag: 'wx' });
    console.log('Summary:', JSON.stringify(summary));
    console.log('Saved:', filename);
  } finally { await client.close(); }
}
main().catch(e => { console.error('CINEMATIC MONITOR FAILED:', e.message); process.exitCode = 1; });
