#!/usr/bin/env node
// READ-ONLY sampled integrity probe for the Jin8 master; not a full 511K-line verification.
import fs from 'node:fs/promises';
import path from 'node:path';
import process from 'node:process';
import { PPSSPPClient, getEndpointArg, checkTcp, verifyGame } from './lib/ppsspp-client.mjs';
import { MASTER_SIGNATURES, hex32, evaluateSignature } from './lib/master-signatures.mjs';

async function main() {
  const args = process.argv.slice(2);
  if (args.includes('--help')) {
    console.log('Usage: node tools/probes/verify-master-payload.mjs --endpoint HOST:PORT --label master-stock');
    return;
  }
  const endpoint = getEndpointArg(args);
  const labelIndex = args.indexOf('--label');
  const label = labelIndex >= 0 ? args[labelIndex + 1] : null;
  if (!label || !/^[a-zA-Z0-9][a-zA-Z0-9_-]{0,63}$/.test(label)) {
    throw new Error('Specify --label with 1–64 alphanumeric, underscore or hyphen characters.');
  }
  const filename = path.join('captures', label + '.json');
  const client = new PPSSPPClient(endpoint);
  try {
    await checkTcp(endpoint);
    await client.connect();
    const info = await verifyGame(client);
    let modules = [];
    try {
      const list = await client.request('hle.module.list');
      modules = (list.modules ?? []).map(module => ({ name: module.name ?? null, address: module.address ?? null }));
    } catch (e) { modules = [{ error: String(e.message) }]; }
    const checks = [];
    for (const signature of MASTER_SIGNATURES) {
      try {
        const read = await client.request('memory.read_u32', { address: signature.address });
        checks.push(evaluateSignature(signature, read));
      } catch (error) {
        checks.push(evaluateSignature(signature, null, error.message));
      }
    }
    const counts = { match: 0, mismatch: 0, 'read-error': 0 };
    for (const check of checks) counts[check.status]++;
    const output = {
      schema: 'jin8-master-signature-v1', label, capturedAt: new Date().toISOString(),
      game: info.gameStatus.game, version: info.version, cpu: info.cpu, modules,
      methodology: '11 static CWCheat direct-write signatures; read-only, not proof of full initialization or runtime behavior',
      checks, counts,
    };
    await fs.mkdir(path.dirname(filename), { recursive: true });
    await fs.writeFile(filename, JSON.stringify(output, null, 2) + '\n', { flag: 'wx' });
    console.log('Game:', info.gameStatus.game.title, info.gameStatus.game.id);
    console.log('Modules:', modules.map(m => m.name || 'unknown').join(', '));
    console.log('CPU paused:', info.cpu.paused);
    for (const c of checks) {
      console.log([c.status.padEnd(10), hex32(c.address), 'expected=' + hex32(c.expected),
        'observed=' + (c.actual === undefined ? c.error : hex32(c.actual)),
        '(source line ' + c.line + ')'].join(' '));
    }
    console.log('Summary:', JSON.stringify(counts));
    console.log('Saved:', filename);
    console.log('This cannot prove every master write occurred, nor prove the moveset is working.');
  } finally {
    await client.close();
  }
}

main().catch(e => { console.error('MASTER VERIFY FAILED:', e.message); process.exitCode = 1; });
