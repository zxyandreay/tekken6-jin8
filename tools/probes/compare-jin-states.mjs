#!/usr/bin/env node
// Offline-only structured comparison. Does not contact PPSSPP.
import fs from 'node:fs/promises';
import process from 'node:process';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

export function diffSnapshots(a, b) {
  if (a.schema !== 'jin8-snapshot-v1' || b.schema !== 'jin8-snapshot-v1') {
    throw new Error('Unrecognized snapshot schema. Expected jin8-snapshot-v1.');
  }
  const sections = ['candidateWords', 'ultrawideInstructionWords', 'payloadSamples'];
  const differences = [];
  for (const section of sections) {
    const first = a[section] ?? {};
    const second = b[section] ?? {};
    for (const key of new Set([...Object.keys(first), ...Object.keys(second)])) {
      const before = first[key] ?? null;
      const after = second[key] ?? null;
      const lhs = JSON.stringify(before);
      const rhs = JSON.stringify(after);
      if (lhs !== rhs) differences.push({ section, key, before, after });
    }
  }
  return differences;
}

async function main() {
  const args = process.argv.slice(2);
  if (args.includes('--help') || args.length !== 2) {
    console.log('Usage: node tools/probes/compare-jin-states.mjs captures/stock.json captures/mod.json');
    if (!args.includes('--help')) process.exitCode = 2;
    return;
  }
  const [a, b] = await Promise.all(args.map(async p => JSON.parse(await fs.readFile(p, 'utf8'))));
  if (a.game?.id !== 'ULUS10466' || b.game?.id !== 'ULUS10466') throw new Error('Both captures must be ULUS10466.');
  console.log(`Comparing: ${a.label} (${a.capturedAt}) → ${b.label} (${b.capturedAt})`);
  const changes = diffSnapshots(a, b);
  for (const c of changes) {
    const short = item => item?.value ?? item?.bytesHex ?? (`error: ${item?.error ?? 'missing'}`);
    console.log(`${c.section}/${c.key}: ${short(c.before)} -> ${short(c.after)}`);
  }
  console.log(`Total changed probes: ${changes.length}. Unchanged does not imply equal gameplay behavior.`);
}
if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  main().catch(e => { console.error('COMPARE FAILED:', e.message); process.exitCode = 1; });
}
