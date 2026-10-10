#!/usr/bin/env node
// Read-only PPSSPP move-state sampler. No memory writes, CPU pause, or breakpoints.
import fs from 'node:fs/promises';
import path from 'node:path';
import process from 'node:process';
import { getEndpointArg, checkTcp, PPSSPPClient, verifyGame } from './lib/ppsspp-client.mjs';
import { LINK_ANCHORS, resolveTargets, hex32 } from './lib/jin-runtime-links.mjs';

const FIELDS = [
  ['characterId', 0x08C078B4],
  ['movesetBase', 0x08C078E0],
  ['currentMoveId', 0x08C07A18],
  ['incomingMoveId', 0x08C07D1C],
  ['inputState', 0x08C07D3C],
  ['speedController', 0x08C52890],
  ['cameraSpeed', 0x08D6F270],
  ['cameraZoom', 0x08D6F224],
];
const WATCH_LINES = new Set([511791, 512751, 512830, 513124, 513567, 513991, 513997]);
const WATCH_LINKS = LINK_ANCHORS.filter(item => WATCH_LINES.has(item.sourceLine));
const pause = milliseconds => new Promise(resolve => setTimeout(resolve, milliseconds));
function option(args, name, fallback) {
  const idx = args.indexOf(name);
  return idx < 0 ? fallback : args[idx + 1];
}
async function readU32(client, address) {
  try {
    const reply = await client.request('memory.read_u32', { address });
    const value = reply.value ?? reply.uintValue;
    return Number.isInteger(value) ? { value: hex32(value) } : { error: 'No integer value returned' };
  } catch (error) { return { error: error.message }; }
}
async function readCameraInstruction(client) {
  try {
    const reply = await client.request('memory.read', { address: 0x0895350C, size: 4, replacements: false });
    const bytes = Buffer.from(reply.base64, 'base64');
    return bytes.length === 4 ? { value: hex32(bytes.readUInt32LE(0)) } : { error: 'Guest code read returned wrong size' };
  } catch (error) { return { error: error.message }; }
}
async function snapshot(client, started) {
  const record = { elapsedMs: Date.now() - started, fields: {}, conditionalLinks: {} };
  for (const [name, address] of FIELDS) record.fields[name] = await readU32(client, address);
  record.fields.cameraInstruction = await readCameraInstruction(client);
  const raw = record.fields.movesetBase.value;
  const ptr = raw ? Number.parseInt(raw, 16) : NaN;
  try {
    const targets = resolveTargets(ptr, WATCH_LINKS);
    for (const target of targets) {
      const observed = await readU32(client, target.address);
      record.conditionalLinks[String(target.sourceLine)] = {
        address: hex32(target.address), expected: hex32(target.expected),
        ...observed,
        matchesExpected: observed.value === hex32(target.expected),
      };
    }
  } catch (error) { record.pointerWarning = error.message; }
  // Detect whether the table base changed *within* this sequential (non-atomic) pass.
  record.movesetBaseAfter = await readU32(client, 0x08C078E0);
  record.pointerStableDuringSample = raw !== undefined
    && record.movesetBaseAfter.value === raw;
  return record;
}
export function summarize(records) {
  const counts = {};
  const pointerBases = new Set();
  const currentMoveIds = new Set();
  let pointerUnstable = 0;
  for (const r of records) {
    if (r.fields.movesetBase.value) pointerBases.add(r.fields.movesetBase.value);
    if (r.fields.currentMoveId.value) currentMoveIds.add(r.fields.currentMoveId.value);
    if (!r.pointerStableDuringSample) pointerUnstable++;
    for (const [line, link] of Object.entries(r.conditionalLinks)) {
      if (!counts[line]) counts[line] = { matches: 0, samples: 0, observed: {} };
      if (link.value) {
        counts[line].samples++;
        if (link.matchesExpected) counts[line].matches++;
        counts[line].observed[link.value] = (counts[line].observed[link.value] ?? 0) + 1;
      }
    }
  }
  return {
    pointerBases: [...pointerBases],
    currentMoveIds: [...currentMoveIds],
    pointerUnstableSamples: pointerUnstable,
    conditionalLinks: counts,
  };
}
async function main() {
  const args = process.argv.slice(2);
  if (args.includes('--help')) {
    console.log('Usage: node tools/probes/watch-jin-move-conditions.mjs --endpoint HOST:PORT --label jin-one-move --samples 40 --interval-ms 200');
    return;
  }
  const endpoint = getEndpointArg(args);
  const label = option(args, '--label');
  if (!label || !/^[a-zA-Z0-9][a-zA-Z0-9_-]{0,63}$/.test(label)) {
    throw new Error('Use --label with 1-64 letters, digits, underscore or hyphen.');
  }
  const samples = Number(option(args, '--samples', '40'));
  const intervalMs = Number(option(args, '--interval-ms', '200'));
  if (!Number.isInteger(samples) || samples < 1 || samples > 80) throw new Error('--samples must be 1..80');
  if (!Number.isInteger(intervalMs) || intervalMs < 100 || intervalMs > 1500) throw new Error('--interval-ms must be 100..1500');
  const client = new PPSSPPClient(endpoint);
  const filename = path.join('captures', label + '.json');
  try {
    await checkTcp(endpoint);
    await client.connect();
    const identity = await verifyGame(client);
    console.log('Connected: ' + identity.gameStatus.game.id + '; no writes, breakpoints or pause');
    console.log('Start performing one reproducible move when sampling begins.');
    const started = Date.now();
    const events = [];
    let previous = null;
    for (let i = 0; i < samples; i++) {
      const record = await snapshot(client, started);
      events.push(record);
      const move = record.fields.currentMoveId.value || 'unreadable';
      const base = record.fields.movesetBase.value || 'unreadable';
      const matched = Object.entries(record.conditionalLinks)
        .filter(([, v]) => v.matchesExpected).map(([line]) => line);
      const key = [base, move, record.fields.incomingMoveId.value, matched.join(',')].join('|');
      if (i === 0 || key !== previous) {
        console.log('sample=' + (i + 1) + ' t=' + record.elapsedMs + 'ms base=' + base
          + ' move=' + move + ' matching-source-lines=' + (matched.join(',') || 'none'));
      }
      previous = key;
      if (i + 1 < samples) await pause(intervalMs);
    }
    const result = {
      schema: 'jin8-move-condition-watch-v1', label, capturedAt: new Date().toISOString(),
      game: identity.gameStatus.game, version: identity.version,
      sourceMoveHints: { '0x0266': 'source section CD', '0x043D': 'source comment 1+2+3+4', '0x02E3': 'source condition with no confirmed input mapping' },
      note: 'Read-only, sequential, and non-atomic. Source pointer writes are conditional. Do not equate observed values with all moves working.',
      sampleCount: samples, intervalMs, records: events, summary: summarize(events),
    };
    await fs.mkdir(path.dirname(filename), { recursive: true });
    await fs.writeFile(filename, JSON.stringify(result, null, 2) + '\n', { flag: 'wx' });
    console.log('Summary: ' + JSON.stringify(result.summary, null, 2));
    console.log('Saved: ' + filename);
  } finally { await client.close(); }
}
if (process.argv[1]?.endsWith('watch-jin-move-conditions.mjs')) {
  main().catch(e => { console.error('MOVE CONDITION WATCH FAILED: ' + e.message); process.exitCode = 1; });
}
