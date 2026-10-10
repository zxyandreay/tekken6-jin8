#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspiofilemgr.h>
#include <psputils.h>
#include <pspctrl.h>
#include <stdint.h>
#include <string.h>

PSP_MODULE_INFO("Tekken6Jin8", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

/*
 * Experimental, standalone, game-specific CWCheat interpreter.
 * Keep Tekken6Ultrawide entirely unchanged.
 * Data is generated locally from the user's source INI; never distributed
 * in the public source tree.
 */
#define BASE         0x08800000u
#define GAME_ID_ADDR 0x08C078B4u
#define MOVESET_ADDR 0x08C078E0u
#define MASTER_MARK  0x09110000u
#define MASTER_FIRST 0x08B98190u
#define MASTER_LAST  0x0A1F0AB0u
#define MASTER_LAST_VAL 0x00DF7C0Eu
#define MAX_GUEST    0x0D000000u  /* memory=80, conservative bound */
#define MASTER_PATH "ms0:/PSP/PLUGINS/Tekken6Jin8/master.j8c"
#define RUNTIME_PATH "ms0:/PSP/PLUGINS/Tekken6Jin8/runtime.j8c"
#define LOG_PATH "ms0:/PSP/PLUGINS/Tekken6Jin8/Jin8.log"
#define MASTER_LINES 511607u
#define RUNTIME_LINES 2458u
#define BLOCK_WORDS 512

typedef struct { uint32_t a, b; } Line;
typedef struct {
    SceUID fd;
    uint32_t remaining;
    uint32_t pos, used;
    Line data[BLOCK_WORDS];
} Reader;

static volatile int running = 1;
static volatile int active_thread = 0;
static SceUID thread_id = -1;
static Line runtime[RUNTIME_LINES];
static unsigned runtime_count = 0;
static SceUID log_fd = -1;

static void note(const char *message) {
    if (log_fd >= 0) {
        sceIoWrite(log_fd, message, (int)strlen(message));
        sceIoWrite(log_fd, "\n", 1);
    }
}
static int range_ok(uint32_t addr, uint32_t len) {
    uint64_t end = (uint64_t)addr + len;
    return len != 0 && addr >= 0x08000000u && end <= MAX_GUEST;
}
static uint32_t cwaddr(uint32_t offset) {
    return (BASE + (offset & 0x0FFFFFFFu)) & 0x3FFFFFFFu;
}
static uint32_t getmem(uint32_t a, int size) {
    if (!range_ok(a, size) || (a & (size - 1)) != 0) return 0;
    if (size == 1) return *(volatile uint8_t *)(uintptr_t)a;
    if (size == 2) return *(volatile uint16_t *)(uintptr_t)a;
    return *(volatile uint32_t *)(uintptr_t)a;
}
static void setmem(uint32_t a, int size, uint32_t val) {
    if (!range_ok(a, size) || (a & (size - 1)) != 0) return;
    if (size == 1) *(volatile uint8_t *)(uintptr_t)a = (uint8_t)val;
    else if (size == 2) *(volatile uint16_t *)(uintptr_t)a = (uint16_t)val;
    else *(volatile uint32_t *)(uintptr_t)a = val;
}
static int open_reader(Reader *r, const char *path, uint32_t expected) {
    char magic[8];
    memset(r, 0, sizeof(*r));
    r->fd = sceIoOpen(path, PSP_O_RDONLY, 0);
    if (r->fd < 0) return 0;
    if (sceIoRead(r->fd, magic, 8) != 8 ||
        memcmp(magic, "J8C1", 4) != 0) return 0;
    uint32_t n;
    memcpy(&n, magic + 4, 4);
    if (n != expected) return 0;
    r->remaining = n;
    return 1;
}
static void close_reader(Reader *r) {
    if (r->fd >= 0) { sceIoClose(r->fd); r->fd = -1; }
}
static int read_next(Reader *r, Line *out) {
    if (!r->remaining) return 0;
    if (r->pos == r->used) {
        int lines = (r->remaining > BLOCK_WORDS) ? BLOCK_WORDS : (int)r->remaining;
        int bytes = sceIoRead(r->fd, r->data, lines * sizeof(Line));
        if (bytes != lines * (int)sizeof(Line)) return 0;
        r->used = lines; r->pos = 0;
    }
    *out = r->data[r->pos++];
    r->remaining--;
    return 1;
}
typedef struct {
    const Line *data;
    uint32_t count, index;
    Reader *reader;
} Stream;
static int next(Stream *s, Line *out) {
    if (s->reader) return read_next(s->reader, out);
    if (s->index >= s->count) return 0;
    *out = s->data[s->index++];
    return 1;
}
static void skip(Stream *s, uint32_t n) {
    Line unused;
    while (n-- && next(s, &unused)) {}
}
static int test_value(uint32_t left, uint32_t right, uint32_t op) {
    switch (op) {
        case 0: return (int32_t)left == (int32_t)right;
        case 1: return (int32_t)left != (int32_t)right;
        case 2: return (int32_t)left < (int32_t)right;
        case 3: return (int32_t)left > (int32_t)right;
    }
    return 0;
}
static int execute(Stream *s) {
    Line x, y;
    int changes_code = 0;
    while (running && next(s, &x)) {
        uint32_t type = x.a >> 28, a = cwaddr(x.a), b = x.b;
        uint32_t n, addr, val, increment;
        switch (type) {
        case 0: {
            int size = (b & 0xFFFF0000u) ? 4 : ((b & 0xFF00u) ? 2 : 1);
            setmem(a, size, b);
            break;
        }
        case 1: setmem(a, 2, b); break;
        case 2: setmem(a, 4, b); break;
        case 3: {
            uint32_t subtype = (x.a >> 20) & 15u;
            int size = (subtype <= 2) ? 1 : ((subtype <= 4) ? 2 : 4);
            int sub = (subtype & 1u) == 0;
            uint32_t delta = (size == 1) ? x.a & 255u : (size == 2) ? x.a & 65535u : 0;
            if (subtype == 5 || subtype == 6) {
                if (!next(s, &y)) return 0;
                delta = y.a;
            } else if (subtype < 1 || subtype > 4) return 0;
            addr = cwaddr(b);
            setmem(addr, size, sub ? getmem(addr, size) - delta : getmem(addr, size) + delta);
            break;
        }
        case 4:
        case 8: {
            if (!next(s, &y)) return 0;
            int size = type == 4 ? 4 : ((y.a & 0xFFFF0000u) ? 2 : 1);
            n = b >> 16;
            uint32_t step = (b & 0xFFFFu) * size;
            uint64_t span = (uint64_t)n * step + size;
            if (n < 65536 && span < 24u * 1024u * 1024u && range_ok(a, (uint32_t)span)) {
                val = size == 4 ? y.a : y.a & (size == 1 ? 255u : 65535u);
                for (uint32_t j = 0; j < n; ++j) {
                    setmem(a + j * step, size, val);
                    val += y.b;
                }
            }
            break;
        }
        case 5: {
            if (!next(s, &y)) return 0;
            uint32_t to = cwaddr(y.a);
            if (b && b < 24u * 1024u * 1024u && range_ok(a, b) && range_ok(to, b)) {
                memmove((void *)(uintptr_t)to, (const void *)(uintptr_t)a, b);
            }
            break;
        }
        case 6: {
            if (!next(s, &y)) return 0;
            int32_t base_offset = ((int32_t)y.a >> 20) * 4;
            uint32_t count = (y.a & 65535u);
            if (!count || count > 128u) return 0;
            count--;
            uint32_t base_addr = a + base_offset;
            uint32_t base = getmem(base_addr, 4);
            int mode = (y.a >> 16) & 15;
            int32_t off = (int32_t)y.b;
            val = b;
            for (uint32_t j = 0; j < count; ++j) {
                if (!next(s, &x)) return 0;
                uint32_t cmd = x.a >> 28;
                if (cmd == 9) { base += x.a & 0x0FFFFFFFu; val += x.b; }
                else if (cmd == 2 || cmd == 3) {
                    int32_t step = (int32_t)(x.a & 0x0FFFFFFFu);
                    if (cmd == 3) step = -step;
                    base = getmem(base + step, 4);
                } else if (cmd == 1) {
                    uint32_t src = getmem(a, 4) + off;
                    uint32_t dst = getmem(base_addr, 4) + (x.a & 0x0FFFFFFFu);
                    if (val && val < 24u * 1024u * 1024u &&
                        range_ok(src, val) && range_ok(dst, val)) {
                        memmove((void *)(uintptr_t)dst, (const void *)(uintptr_t)src, val);
                    }
                    mode = -1;
                }
            }
            addr = (mode >= 3) ? base - off : base + off;
            if (mode == 0 || mode == 3) setmem(addr, 1, val);
            else if (mode == 1 || mode == 4) setmem(addr, 2, val);
            else if (mode == 2 || mode == 5) setmem(addr, 4, val);
            break;
        }
        case 14: {
            /* E0 / E1: skip next N *physical* _L lines if condition false. */
            int size = ((x.a >> 24) == 0xE1) ? 1 : 2;
            uint32_t mask = size == 1 ? 255u : 65535u;
            uint32_t limit = size == 1 ? 255u : 4095u;
            uint32_t op = b >> 28;
            n = (x.a >> 16) & limit;
            if (!test_value(getmem(cwaddr(b), size) & mask, x.a & mask, op)) skip(s, n);
            break;
        }
        default:
            /* The supplied payload contains no type-7/D/A/etc.
               Unknown commands fail closed rather than corrupt later writes. */
            return 0;
        }
        if (a >= 0x08800000u && a < 0x08A00000u &&
            (type == 0 || type == 1 || type == 2)) changes_code = 1;
    }
    /* Only the engine's game-code writes require instruction synchronization. */
    sceKernelDcacheWritebackInvalidateAll();
    if (changes_code) sceKernelIcacheInvalidateAll();
    return 1;
}
static int read_runtime(void) {
    Reader rd;
    rd.fd = -1;
    if (!open_reader(&rd, RUNTIME_PATH, RUNTIME_LINES)) {
        close_reader(&rd); note("runtime file missing or bad header"); return 0;
    }
    while (runtime_count < RUNTIME_LINES &&
           read_next(&rd, &runtime[runtime_count])) runtime_count++;
    close_reader(&rd);
    if (runtime_count != RUNTIME_LINES) {
        note("runtime truncated"); return 0;
    }
    note("runtime loaded"); return 1;
}
static int load_master(void) {
    Reader rd;
    rd.fd = -1;
    if (!open_reader(&rd, MASTER_PATH, MASTER_LINES)) {
        close_reader(&rd); note("master file missing or bad header"); return 0;
    }
    Stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.reader = &rd;
    int result = execute(&stream);
    close_reader(&rd);
    if (!result) { note("master execution aborted"); return 0; }
    if (getmem(MASTER_MARK, 4) != MASTER_FIRST ||
        getmem(MASTER_LAST, 4) != MASTER_LAST_VAL) {
        note("master signatures not present - skip runtime"); return 0;
    }
    note("master initialized (2 signatures)");
    return 1;
}
static int jin_fight_ready(uint32_t *moveset) {
    if (getmem(GAME_ID_ADDR, 4) != 9u) return 0;
    uint32_t ptr = getmem(MOVESET_ADDR, 4);
    if ((ptr & 0xFFFF0000u) != 0x086A0000u &&
        (ptr & 0xFFFF0000u) != 0x08750000u) return 0;
    if ((ptr & 3u) || !range_ok(ptr, 512)) return 0;
    if (moveset) *moveset = ptr;
    return 1;
}
static int loader_thread(SceSize args, void *argp) {
    (void)args; (void)argp;
    active_thread = 1;
    log_fd = sceIoOpen(LOG_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0666);
    note("Tekken6Jin8 experimental PRX initialized");
    if (!read_runtime()) goto done;
    int installed = 0, idle_cycles = 0;
    uint32_t prior_base = 0;
    while (running) {
        uint32_t base = 0;
        if (!jin_fight_ready(&base)) {
            if (++idle_cycles > 40) { installed = 0; prior_base = 0; }
            sceKernelDelayThread(25000); continue;
        }
        idle_cycles = 0;
        if (!installed || base != prior_base) {
            note("Jin fight detected; initializing master");
            if (!load_master()) {
                /* Fail closed and throttle retry; avoid infinite repeated 4MB writes. */
                note("master failed; waiting for next fight");
                installed = 0;
                while (running && jin_fight_ready(0)) sceKernelDelayThread(50000);
                continue;
            }
            installed = 1; prior_base = base;
        }
        Stream stream;
        memset(&stream, 0, sizeof(stream));
        stream.data = runtime;
        stream.count = runtime_count;
        (void)execute(&stream);
        sceKernelDelayThread(50000);
    }
done:
    note("Jin8 loader stopped");
    if (log_fd >= 0) sceIoClose(log_fd);
    active_thread = 0;
    return 0;
}
int module_start(SceSize args, void *argp) {
    (void)args; (void)argp;
    running = 1;
    thread_id = sceKernelCreateThread("Jin8Loader", loader_thread, 0x18, 0x8000, 0, NULL);
    if (thread_id >= 0) sceKernelStartThread(thread_id, 0, NULL);
    return 0;
}
int module_stop(SceSize args, void *argp) {
    (void)args; (void)argp;
    running = 0;
    if (thread_id >= 0) sceKernelWaitThreadEnd(thread_id, NULL);
    return 0;
}
