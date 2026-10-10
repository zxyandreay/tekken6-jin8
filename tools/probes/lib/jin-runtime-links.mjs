// Sampled 32-bit CWCheat type-0x6 pointer writes in the user's (1)jin runtime group.
// Offsets/expected values are independent read-only research metadata, not mod payload.
// Source file: ULUS10466 (3).ini. These entries are CONDITIONAL; nonmatches are not errors.
export const LINK_ANCHORS = Object.freeze([
  { sourceLine: 511781, offset: 0x02C250, expected: 0x0909CE00, description: 'reaction pointer A' },
  { sourceLine: 511791, offset: 0x02C270, expected: 0x0909CE10, description: 'reaction pointer B' },
  { sourceLine: 512751, offset: 0x05F9DC, expected: 0x090A5490, description: 'conditional move linkage A' },
  { sourceLine: 512830, offset: 0x03F974, expected: 0x08E423BC, description: 'conditional move linkage B' },
  { sourceLine: 513124, offset: 0x066AF4, expected: 0x09096200, description: 'conditional move linkage C' },
  { sourceLine: 513567, offset: 0x069B94, expected: 0x0A02F5C4, description: 'conditional move linkage D' },
  { sourceLine: 513991, offset: 0x0624D4, expected: 0x0902E2B0, description: 'late-runtime move linkage A' },
  { sourceLine: 513997, offset: 0x0625DC, expected: 0x0902E2B0, description: 'late-runtime move linkage B' },
]);
export const hex32 = v => '0x' + (v >>> 0).toString(16).toUpperCase().padStart(8, '0');
export function resolveTargets(pointer, anchors = LINK_ANCHORS) {
  if (!Number.isInteger(pointer) || pointer < 0x08000000 || pointer > 0x0C000000
      || (pointer & 3) !== 0) {
    throw new Error('Invalid/unaligned P1 moveset pointer; no child reads attempted.');
  }
  return anchors.map(a => {
    const address = pointer + a.offset;
    if ((address & 3) !== 0 || address < 0x08000000 || address + 4 > 0x0C000000) {
      throw new Error('Derived target out of supported guest range.');
    }
    return { ...a, address: address >>> 0 };
  });
}
