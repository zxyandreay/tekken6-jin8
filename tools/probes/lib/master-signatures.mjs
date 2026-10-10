// Immutable, independently sampled 32-bit CWCheat master write signatures.
// PSP guest address = 0x08800000 + (first opcode word & 0x0FFFFFFF).
// From user-supplied ULUS10466 (3).ini. No copyrighted binary data embedded.
export const MASTER_SIGNATURES = Object.freeze([
  { line: 4,       cwcheat: 0x20910000, address: 0x09110000, expected: 0x08B98190 },
  { line: 51184,   cwcheat: 0x2094CF60, address: 0x0914CF60, expected: 0x3D9354E1 },
  { line: 127913,  cwcheat: 0x21821E24, address: 0x0A021E24, expected: 0x3F9E7B67 },
  { line: 204671,  cwcheat: 0x2186CB38, address: 0x0A06CB38, expected: 0xBDA1F606 },
  { line: 255851,  cwcheat: 0x2189E918, address: 0x0A09E918, expected: 0xBFC91687 },
  { line: 307023,  cwcheat: 0x218D071C, address: 0x0A0D071C, expected: 0xBEE7F9FB },
  { line: 383808,  cwcheat: 0x2191B3C4, address: 0x0A11B3C4, expected: 0xBF097DAD },
  { line: 460545,  cwcheat: 0x21966118, address: 0x0A166118, expected: 0x3F4D887D },
  { line: 501468,  cwcheat: 0x219E7470, address: 0x0A1E7470, expected: 0xBEA09EC9 },
  { line: 510675,  cwcheat: 0x219F0424, address: 0x0A1F0424, expected: 0x67406AF3 },
  { line: 511094,  cwcheat: 0x219F0AB0, address: 0x0A1F0AB0, expected: 0x00DF7C0E },
]);
export const hex32 = value => '0x' + (value >>> 0).toString(16).toUpperCase().padStart(8, '0');
export const cwcheatTarget = code => (0x08800000 + (code & 0x0FFFFFFF)) >>> 0;

export function evaluateSignature(signature, response, error = null) {
  if (error) return { ...signature, status: 'read-error', error: String(error) };
  const raw = response?.value ?? response?.uintValue;
  if (!Number.isInteger(raw)) return { ...signature, status: 'read-error', error: 'No integer PPSSPP response' };
  const actual = raw >>> 0;
  return { ...signature, actual, status: actual === (signature.expected >>> 0) ? 'match' : 'mismatch' };
}
