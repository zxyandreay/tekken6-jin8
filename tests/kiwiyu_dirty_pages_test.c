#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "dirty_pages.h"
int main(void) {
 static uint8_t bits[DIRTY_BITMAP_BYTES];
 assert(DIRTY_PAGE_COUNT == 20480u && DIRTY_BITMAP_BYTES == 2560u);
 assert(!dirty_pages_has(bits, 0));
 dirty_pages_add(bits, 0x08C07974u, 4u);
 dirty_pages_add(bits, 0x08C07974u, 4u);
 uint32_t a = (0x08C07974u - DIRTY_BASE) >> DIRTY_PAGE_SHIFT;
 assert(dirty_pages_has(bits, a));
 assert(!dirty_pages_has(bits, a+1u));
 dirty_pages_add(bits, 0x08C07FF0u, 64u);
 assert(dirty_pages_has(bits, a+1u));
 dirty_pages_add(bits, 0x0CFFFFFEu, 2u);
 assert(dirty_pages_has(bits, DIRTY_PAGE_COUNT-1u));
 dirty_pages_add(bits, 0x07FFFFFFu, 4u);
 assert(!dirty_pages_has(bits, 0));
 memset(bits, 0, sizeof(bits));
 assert(!dirty_pages_has(bits, a));
 return 0;
}
