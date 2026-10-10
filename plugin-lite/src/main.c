#include <pspkernel.h>
#include <pspthreadman.h>
#include <stdint.h>

PSP_MODULE_INFO("Tekken6Jin8Gate", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

/* v0.3: a minimal game-state gate. All actual Jin8 CWCheat operations
 * run inside PPSSPP's own cheat scheduler, never from this PSP thread.
 * The official Tekken6Ultrawide PRX is not referenced or modified. */
#define CHARACTER   0x08C078B4u
#define MOVESET     0x08C078E0u
#define MOVEPTR     0x08C07964u
#define MOVEID      0x08C07A18u
#define GATE        0x0A1FFFC0u   /* outside source master's final 0x0A1F0AB0 */
#define INIT        1u
#define INSTALLED   2u

static volatile int running = 1;
static SceUID watcher = -1;

static uint32_t word(uint32_t address) {
    return *(volatile const uint32_t *)(uintptr_t)address;
}
static void store(uint32_t address, uint32_t value) {
    *(volatile uint32_t *)(uintptr_t)address = value;
}
static int ready(void) {
    uint32_t base = word(MOVESET);
    uint32_t ptr = word(MOVEPTR);
    return word(CHARACTER) == 9u
        && ((base & 0xFFFF0000u) == 0x086A0000u
            || (base & 0xFFFF0000u) == 0x08750000u)
        && (ptr >= 0x08600000u && ptr < 0x09C00000u)
        && word(MOVEID) != 0u;
}
static int gate_thread(SceSize args, void *argp) {
    int armed = 0, missing = 0, install_wait = 0;
    uint32_t previous_base = 0;
    (void)args; (void)argp;
    while (running) {
        int valid = ready();
        uint32_t base = valid ? word(MOVESET) : 0u;
        if (valid) {
            missing = 0;
            if (!armed || base != previous_base) {
                store(GATE, INIT);
                armed = 1;
                install_wait = 0;
                previous_base = base;
            } else if (word(GATE) == INSTALLED) {
                install_wait = 0;
            } else if (word(GATE) == INIT && ++install_wait > 140) {
                /* Missing/broken master: stop reapplying it this match. */
                store(GATE, 0);
                install_wait = -20000;
            }
        } else if (++missing >= 12) {
            if (armed) store(GATE, 0);
            armed = 0;
            previous_base = 0;
            missing = 12;
        }
        sceKernelDelayThread(50000);
    }
    store(GATE, 0);
    return 0;
}
int module_start(SceSize args, void *argp) {
    (void)args; (void)argp;
    running = 1;
    watcher = sceKernelCreateThread("Jin8Gate", gate_thread, 0x20, 0x1000, 0, 0);
    if (watcher >= 0) sceKernelStartThread(watcher, 0, 0);
    return 0;
}
int module_stop(SceSize args, void *argp) {
    (void)args; (void)argp;
    running = 0;
    if (watcher >= 0) sceKernelWaitThreadEnd(watcher, 0);
    return 0;
}
