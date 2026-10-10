import test from 'node:test';
import assert from 'node:assert/strict';
import { LINK_ANCHORS, resolveTargets } from '../tools/probes/lib/jin-runtime-links.mjs';

test('8 samples map to expected guest addresses from user Jin pointer', () => {
  const list = resolveTargets(0x08754400);
  assert.equal(list.length, 8);
  assert.equal(list[0].address, 0x08780650);
  assert.equal(list[6].address, 0x087B68D4);
  assert.equal(list[7].address, 0x087B69DC);
  assert.deepEqual(list.map(x => x.sourceLine), LINK_ANCHORS.map(x => x.sourceLine));
});

test('invalid guest pointers are rejected instead of dereferenced', () => {
  for (const p of [0, 0x12345678, 0x08754401, 0x0FFFFFFC]) {
    assert.throws(() => resolveTargets(p));
  }
});
