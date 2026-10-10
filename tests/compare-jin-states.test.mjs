import test from 'node:test';
import assert from 'node:assert/strict';
import { diffSnapshots } from '../tools/probes/compare-jin-states.mjs';

test('detects word changes while ignoring timestamps', () => {
  const a = { schema: 'jin8-snapshot-v1', capturedAt: 'A', candidateWords: { x: { value: '0x00000000' } } };
  const b = { schema: 'jin8-snapshot-v1', capturedAt: 'B', candidateWords: { x: { value: '0x00000001' } } };
  const diff = diffSnapshots(a, b);
  assert.equal(diff.length, 1);
  assert.equal(diff[0].key, 'x');
});

test('rejects incompatible data', () => {
  assert.throws(() => diffSnapshots({ schema: 'wrong' }, { schema: 'jin8-snapshot-v1' }));
});

test('detects pointer-window changes without contacting PPSSPP', () => {
  const a = { schema: 'jin8-snapshot-v1', pointerWindows: { move: { pointer: '0x087AD474', bytesHex: '0102', sha256: 'aaa' } } };
  const b = { schema: 'jin8-snapshot-v1', pointerWindows: { move: { pointer: '0x087AD474', bytesHex: '0103', sha256: 'bbb' } } };
  const diff = diffSnapshots(a, b);
  assert.equal(diff.length, 1);
  assert.equal(diff[0].section, 'pointerWindows');
});
