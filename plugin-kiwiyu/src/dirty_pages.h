#ifndef KIWIYU_DIRTY_PAGES_H
#define KIWIYU_DIRTY_PAGES_H
#include <stdint.h>
/* 80 MiB guest window 0x08000000..0x0D000000 (memory=80).
   Tracking code/data changes with 4 KiB pages avoids the legacy 128-entry
   overflow -> invalidate-everything fallback during 16ms runtime updates. */
#define DIRTY_BASE 0x08000000u
#define DIRTY_END 0x0D000000u
#define DIRTY_PAGE_SHIFT 12u
#define DIRTY_PAGE_COUNT ((DIRTY_END - DIRTY_BASE) >> DIRTY_PAGE_SHIFT)
#define DIRTY_BITMAP_BYTES ((DIRTY_PAGE_COUNT + 7u) / 8u)
static inline void dirty_pages_add(uint8_t *bits, uint32_t addr, uint32_t bytes) {
    if (!bytes || addr < DIRTY_BASE || (uint64_t)addr + bytes > DIRTY_END) return;
    uint32_t first = (addr - DIRTY_BASE) >> DIRTY_PAGE_SHIFT;
    uint32_t last = (addr + bytes - 1u - DIRTY_BASE) >> DIRTY_PAGE_SHIFT;
    for (uint32_t p = first; p <= last; ++p)
        bits[p >> 3] |= (uint8_t)(1u << (p & 7u));
}
static inline int dirty_pages_has(const uint8_t *bits, uint32_t page) {
    return page < DIRTY_PAGE_COUNT && !!(bits[page >> 3] & (uint8_t)(1u << (page & 7u)));
}
#endif
