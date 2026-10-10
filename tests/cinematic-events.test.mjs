import test from 'node:test';
import assert from 'node:assert/strict';
import { u32, floatBits, classifyCinematic } from '../tools/probes/lib/cinematic-events.mjs';

const fields = (mode, zoom, rate) => ({
  speedMode: { value: mode }, cameraZoom: { value: zoom }, cameraRate: { value: rate },
});
test('decodes source SlowMo constants', () => {
  assert.equal(floatBits('0x41800000'), 16);
  assert.equal(floatBits('0x3F000000'), 0.5);
  assert.equal(u32('0x00000003'), 3);
});
test('detects speed mode 3 even when zoom has not yet reached 16', () => {
  const value = classifyCinematic(fields('0x00000003','0x4203BA5E','0x3EFEF9DB'));
  assert.equal(value.trigger, true);
  assert.equal(value.speedMode, 3);
  assert.ok(value.zoom > 30 && value.zoom < 40);
});
test('normal values do not trigger', () => {
  assert.equal(classifyCinematic(fields('0x00000001','0x42480000','0x3DCCCCCD')).trigger, false);
});
test('invalid read cannot be mistaken for event', () => {
  assert.equal(classifyCinematic({ speedMode: { error:'invalid' }, cameraZoom:{ error:'invalid'} }).trigger, false);
});
