// Diagnostic only: source (1)jin "SlowMo v2.3 Master" writes mode 3, zoom 16.0,
// and camera-rate 0.5. These memory values don't by themselves prove perceived slow motion.
export function u32(value) {
  if (typeof value !== 'string' || !/^0x[0-9A-Fa-f]{1,8}$/.test(value)) return null;
  return Number.parseInt(value, 16) >>> 0;
}
export function floatBits(value) {
  const bits = u32(value);
  if (bits === null) return null;
  const buffer = Buffer.allocUnsafe(4);
  buffer.writeUInt32LE(bits, 0);
  const number = buffer.readFloatLE(0);
  return Number.isFinite(number) ? number : null;
}
export function classifyCinematic(fields) {
  const speed = u32(fields.speedMode?.value);
  const zoom = floatBits(fields.cameraZoom?.value);
  const rate = floatBits(fields.cameraRate?.value);
  const mode3 = speed === 3;
  const deepZoom = zoom !== null && zoom <= 40 && zoom > 0;
  const flags = [
    ...(mode3 ? ['speed-mode-3'] : []),
    ...(deepZoom ? ['camera-zoom-below-40'] : []),
  ];
  return { trigger: flags.length > 0, flags, speedMode: speed, zoom, cameraRate: rate };
}
