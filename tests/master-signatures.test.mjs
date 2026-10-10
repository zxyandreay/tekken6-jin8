import test from 'node:test';
import assert from 'node:assert/strict';
import { MASTER_SIGNATURES, cwcheatTarget, evaluateSignature } from '../tools/probes/lib/master-signatures.mjs';

test('all sampled direct-write offsets map to PSP user-memory base', () => {
  assert.equal(MASTER_SIGNATURES.length, 11);
  const addresses = new Set();
  for (const signature of MASTER_SIGNATURES) {
    assert.equal(signature.cwcheat >>> 28, 2);
    assert.equal(cwcheatTarget(signature.cwcheat), signature.address);
    assert.equal(signature.address % 4, 0);
    assert.equal(addresses.has(signature.address), false);
    addresses.add(signature.address);
  }
  assert.equal(cwcheatTarget(0x219F0AB0), 0x0A1F0AB0);
});

test('recognizes matching, mismatching and invalid reads', () => {
  const sample = MASTER_SIGNATURES[0];
  assert.equal(evaluateSignature(sample, { value: sample.expected }).status, 'match');
  assert.equal(evaluateSignature(sample, { value: 0 }).status, 'mismatch');
  assert.equal(evaluateSignature(sample, null, 'Invalid address').status, 'read-error');
});
