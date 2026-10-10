#!/usr/bin/env node
// READ-ONLY: inspect selected conditional CWCheat pointer-write destination addresses.
// Unlike the first 256B moveset-header probe, targets live at 0x0D78C..0x69B94 offsets.
import fs from 'node:fs/promises';
import path from 'node:path';
import process from 'node:process';
import { getEndpointArg, checkTcp, PPSSPPClient, verifyGame } from './lib/ppsspp-client.mjs';
import { LINK_ANCHORS, hex32, resolveTargets } from './lib/jin-runtime-links.mjs';

function arg(args, name) {
  const i = args.indexOf(name);
  return i < 0 ? undefined : args[i + 1];
}

async function main() {
  const args = process.argv.slice(2);
  if (args.includes('--help')) {
    console.log('Usage: node tools/probes/trace-jin-runtime-links.mjs --endpoint HOST:PORT --label mod-idle [--samples 1..20] [--interval-ms 100..1000]');
    return;
  }
  const endpoint = getEndpointArg(args);
  const label = arg(args, '--label');
  if (!label || !/^[a-zA-Z0-9][a-zA-Z0-9_-]{0,63}$/.test(label)) {
    throw new Error('Please provide --label with 1-64 alphanumeric, underscore or hyphen characters.');
  }
  const samples = Number(arg(args, '--samples') ?? '1');
  const intervalMs = Number(arg(args, '--interval-ms') ?? '200');
  if (!Number.isInteger(samples) || samples < 1 || samples > 20) throw new Error('--samples must be 1..20');
  if (!Number.isInteger(intervalMs) || intervalMs < 100 || intervalMs > 1000) throw new Error('--interval-ms must be 100..1000');
  const client = new PPSSPPClient(endpoint);
  try {
    await checkTcp(endpoint);
    await client.connect();
    const info = await verifyGame(client);
    const pointerResult = await client.request('memory.read_u32', { address: 0x08C078E0 });
    const pointer = (pointerResult.value ?? pointerResult.uintValue);
    const targets = resolveTargets(pointer, LINK_ANCHORS);
    const observed = Object.fromEntries(targets.map(t => [hex32(t.address), {
      sourceLine: t.sourceLine,
      address: hex32(t.address),
      offset: hex32(t.offset),
      expected: hex32(t.expected),
      description: t.description,
      samples: 0,
      matches: 0,
      observed: {},
      errors: [],
    }]));
    for (let s = 0; s < samples; s++) {
      for (const t of targets) {
        const entry = observed[hex32(t.address)];
        try {
          const read = await client.request('memory.read_u32', { address: t.address });
          const raw = read.value ?? read.uintValue;
          if (!Number.isInteger(raw)) throw new Error('No integer value returned');
          const value = hex32(raw);
          entry.samples++;
          entry.observed[value] = (entry.observed[value] ?? 0) + 1;
          if ((raw >>> 0) === (t.expected >>> 0)) entry.matches++;
        } catch (error) {
          entry.errors.push(error.message);
        }
      }
      if (s + 1 < samples) await new Promise(resolve => setTimeout(resolve, intervalMs));
    }
    const outFile = path.join('captures', label + '.json');
    const result = {
      schema: 'jin8-runtime-links-v1', label, capturedAt: new Date().toISOString(),
      game: info.gameStatus.game, version: info.version, cpu: info.cpu,
      movesetPointer: hex32(pointer), sampleCount: samples, intervalMs,
      note: 'Conditional write targets. A nonmatch is not evidence of failure. Memory not modified.',
      anchors: Object.values(observed),
    };
    await fs.mkdir(path.dirname(outFile), { recursive: true });
    await fs.writeFile(outFile, JSON.stringify(result, null, 2) + '\n', { flag: 'wx' });
    console.log('Game:', info.gameStatus.game.id, 'moveset base:', hex32(pointer), 'paused:', info.cpu.paused);
    for (const a of result.anchors) console.log(a.address, 'expected', a.expected, 'matched', a.matches + '/' + a.samples, 'observed', JSON.stringify(a.observed), 'errors', a.errors.length);
    console.log('Saved:', outFile);
    console.log('All probes were read-only. Only source-conditional 32-bit pointer destinations were sampled.');
  } finally {
    await client.close();
  }
}

main().catch(e => { console.error('RUNTIME LINK PROBE FAILED:', e.message); process.exitCode = 1; });
