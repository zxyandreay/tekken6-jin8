#!/usr/bin/env node
// READ-ONLY and bounded: never performs memory writes, CPU pause/resume, or breakpoints.
import fs from 'node:fs/promises';
import { createHash } from 'node:crypto';
import path from 'node:path';
import process from 'node:process';
import { getEndpointArg, checkTcp, PPSSPPClient, verifyGame } from './lib/ppsspp-client.mjs';

const CANDIDATES = [
  [0x08C078B4, 'p1_character_id_candidate'],
  [0x08C078E0, 'p1_moveset_pointer_candidate'],
  [0x08C07964, 'current_move_pointer_candidate'],
  [0x08C07A18, 'current_move_id_candidate'],
  [0x08C07D1C, 'incoming_move_id_candidate'],
  [0x08C07D24, 'incoming_move_pointer_candidate'],
  [0x08C07D3C, 'input_button_candidate'],
  [0x0895350C, 'shared_camera_word'],
];
const ULTRAWIDE_WORDS = [
  0x08945F10, 0x08945F14, 0x08946794, 0x08946798,
  0x08946BC8, 0x08946BCC, 0x08947D90, 0x08947D94,
];
const PAYLOAD_SAMPLES = [
  [0x09110000, 'master_first_target'],
  [0x08B98190, 'master_operand_reference_a'],
  [0x08E423B0, 'master_operand_reference_b'],
  [0x0A1F0AB0, 'master_high_target_candidate'],
];
const hex = x => `0x${(x >>> 0).toString(16).toUpperCase().padStart(8, '0')}`;

function option(argv, key) {
  const idx = argv.indexOf(key);
  return idx >= 0 ? argv[idx + 1] : undefined;
}

async function captureWord(client, address, code = false) {
  try {
    if (code) {
      // Code words: ignore JIT/emuhack replacement when requesting raw guest instruction bytes.
      const r = await client.request('memory.read', { address, size: 4, replacements: false });
      const b = Buffer.from(r.base64, 'base64');
      if (b.length !== 4) throw new Error(`Expected 4 bytes, got ${b.length}`);
      return { address: hex(address), value: hex(b.readUInt32LE(0)) };
    }
    const r = await client.request('memory.read_u32', { address });
    if (!Number.isInteger(r.value) && !Number.isInteger(r.uintValue)) {
      throw new Error('PPSSPP response contains no integer value');
    }
    return { address: hex(address), value: hex((r.value ?? r.uintValue) >>> 0) };
  } catch (e) { return { address: hex(address), error: e.message }; }
}

async function captureBytes(client, address, size = 16) {
  try {
    const r = await client.request('memory.read', { address, size, replacements: false });
    const b = Buffer.from(r.base64, 'base64');
    if (b.length !== size) throw new Error(`Expected ${size} bytes, got ${b.length}`);
    return { address: hex(address), byteCount: size, bytesHex: b.toString('hex') };
  } catch (e) { return { address: hex(address), error: e.message }; }
}

async function main() {
  const argv = process.argv.slice(2);
  if (argv.includes('--help')) {
    console.log('Usage: node tools/probes/capture-jin-state.mjs --endpoint HOST:PORT --label stock-jin-battle [--out captures/custom.json] [--inspect-pointers]');
    return;
  }
  const url = getEndpointArg(argv);
  const label = option(argv, '--label');
  if (!label || !/^[a-zA-Z0-9][a-zA-Z0-9_-]{0,63}$/.test(label)) {
    throw new Error('Provide --label of 1-64 letters, numbers, underscores, or hyphens.');
  }
  const output = option(argv, '--out') || path.join('captures', `${label}.json`);
  const inspectPointers = argv.includes('--inspect-pointers');
  const client = new PPSSPPClient(url);
  try {
    await checkTcp(url);
    await client.connect();
    const base = await verifyGame(client);
    console.log(`Connected: ${base.gameStatus.game.title} (${base.gameStatus.game.id})`);
    const data = {
      schema: 'jin8-snapshot-v1',
      label,
      capturedAt: new Date().toISOString(),
      endpoint: url.href,
      note: 'Read-only sequential snapshot: game may advance between reads unless manually paused.',
      game: base.gameStatus.game,
      version: base.version,
      cpu: base.cpu,
      modules: null,
      candidateWords: {},
      ultrawideInstructionWords: {},
      payloadSamples: {},
    };
    try {
      const result = await client.request('hle.module.list');
      data.modules = (result.modules || []).map(m => ({ name: m.name ?? null, address: m.address ?? null }));
    } catch (e) { data.modules = { error: e.message }; }

    for (const [addr, name] of CANDIDATES) data.candidateWords[name] = await captureWord(client, addr);
    for (const addr of ULTRAWIDE_WORDS) data.ultrawideInstructionWords[hex(addr)] = await captureWord(client, addr, true);
    for (const [addr, name] of PAYLOAD_SAMPLES) data.payloadSamples[name] = await captureBytes(client, addr);

    if (inspectPointers) {
      data.pointerWindows = {};
      // Only two selected pointer candidates; never recursively chase guest pointers.
      for (const [name, size] of [
        ['p1_moveset_pointer_candidate', 256],
        ['current_move_pointer_candidate', 128],
      ]) {
        const candidate = data.candidateWords[name];
        const value = candidate?.value ? Number.parseInt(candidate.value, 16) : NaN;
        if (!Number.isInteger(value) || value === 0 || (value & 3) !== 0
            || value < 0x08000000 || value > 0x0C000000 - size) {
          data.pointerWindows[name] = { pointer: candidate?.value ?? null, skipped: 'null, unaligned or outside bounded guest range' };
          continue;
        }
        const window = await captureBytes(client, value, size);
        if (window.bytesHex) {
          window.sha256 = createHash('sha256').update(Buffer.from(window.bytesHex, 'hex')).digest('hex');
        }
        data.pointerWindows[name] = { pointer: hex(value), ...window };
      }
    }

    await fs.mkdir(path.dirname(output), { recursive: true });
    await fs.writeFile(output, JSON.stringify(data, null, 2) + '\n', { flag: 'wx' });
    console.log(`SUCCESS: saved ${output}`);
    console.log('Candidate words:', JSON.stringify(data.candidateWords, null, 2));
    if (inspectPointers) console.log('Pointer windows:', JSON.stringify(Object.fromEntries(Object.entries(data.pointerWindows).map(([k, v]) => [k, { pointer: v.pointer, sha256: v.sha256, error: v.error, skipped: v.skipped }])), null, 2));
    console.log('Read errors are recorded in JSON; do not treat them as proof of a bad game build.');
    console.log('No memory writes, breakpoints, or automatic pausing were performed.');
  } finally {
    await client.close();
  }
}

main().catch(e => { console.error('CAPTURE FAILED:', e.message); process.exitCode = 1; });
