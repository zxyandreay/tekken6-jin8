#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>
#include <psputils.h>
#include <pspctrl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "battle_clock.h"

PSP_MODULE_INFO("Tekken6JinKiwiYu", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

/*
 * Experimental standalone KiwiYu Jin 1.0 CWCheat interpreter.
 * Keep Tekken6Ultrawide entirely unchanged.
 * Data is generated locally from the user's source INI; never distributed
 * in the public source tree.
 */
#define BASE         0x08800000u
#define GAME_ID_ADDR 0x08C078B4u
#define MOVESET_ADDR 0x08C078E0u
#define MASTER_MARK  0x0B000000u
#define MASTER_FIRST 0x00008001u
#define MASTER_LAST  0x0A200000u
#define MASTER_LAST_VAL 0x08B98190u
#define LEGACY_AUTOGATE_ADDR 0x0A1FFFC0u
#define MAX_GUEST    0x0D000000u  /* memory=80, conservative bound */
#define MASTER_PATH "ms0:/PSP/PLUGINS/Tekken6JinKiwiYu/jin_master.j8c"
#define MASTER2_PATH "ms0:/PSP/PLUGINS/Tekken6JinKiwiYu/tekken8_master.j8c"
#define RUNTIME_PATH "ms0:/PSP/PLUGINS/Tekken6JinKiwiYu/jin_moves.j8c"
#define LOG_PATH "ms0:/PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.log"
#define MASTER_LINES 436692u
#define MASTER2_LINES 437310u
#define RUNTIME_LINES 5113u
#define BLOCK_WORDS 512
#define SAFE_PATH "ms0:/PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.safe"
#define CURRENT_MOVE_ADDR 0x08C07A18u
#define CURRENT_MOVE_POINTER_ADDR 0x08C07964u
/* Tekken 6 PSP CWCheat timer: _L 0x208FBE4C 0x00000E11 (60 sec). */
#define ROUND_TIMER_ADDR 0x090FBE4Cu
/* 80 consecutive samples at 25ms = ~2 seconds of stable fight state. */
#define FIGHT_SETTLE_TICKS 80u
#define MASTER_ONLY_PATH "ms0:/PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.masteronly"
#define PROTECTED_MAX 28
#define MAX_DIRTY_RANGES 128

typedef struct { uint32_t a, b; } Line;
typedef struct {
    SceUID fd;
    uint32_t remaining;
    uint32_t pos, used;
    Line data[BLOCK_WORDS];
} Reader;

static volatile int running = 1;
static volatile int active_thread = 0;
static int safe_mode = 0;
static int master_only_mode = 0;
static int logged_first_runtime = 0;
static int write_rejected = 0;
static Line *cached_runtime = NULL;
static int cached_runtime_ready = 0;
typedef struct { uint32_t start, end; } Protected;
static Protected protected_ranges[PROTECTED_MAX];
static int protected_count = 0;
static SceUID thread_id = -1;
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
static int intersects_protected(uint32_t addr, uint32_t size) {
    uint64_t end = (uint64_t)addr + size;
    for (int i = 0; i < protected_count; ++i)
        if (addr < protected_ranges[i].end && end > protected_ranges[i].start) return 1;
    return 0;
}
static void note_hex(const char *prefix, uint32_t word) {
    char text[96];
    const char *digits = "0123456789ABCDEF";
    unsigned int n = (unsigned int)strlen(prefix);
    if (n > 70) return;
    memcpy(text, prefix, n);
    text[n++] = '0'; text[n++] = 'x';
    for (int i = 7; i >= 0; --i) text[n++] = digits[(word >> (i * 4)) & 15];
    text[n] = 0;
    note(text);
}
static int protect_loaded_modules(void) {
    SceUID ids[64];
    int count = 0, self = 0;
    if (sceKernelGetModuleIdList(ids, 64, &count) < 0 || count > 64) {
        note("cannot enumerate modules: fail closed"); return 0;
    }
    for (int j = 0; j < count; ++j) {
        SceKernelModuleInfo info;
        memset(&info, 0, sizeof(info)); info.size = sizeof(info);
        if (sceKernelQueryModuleInfo(ids[j], &info) < 0) {
            note("module query unavailable: fail closed"); return 0;
        }
        if (strcmp(info.name, "tekken") == 0) continue;
        if (strcmp(info.name, "Tekken6JinKiwiYu") == 0) self = 1;
        if (info.nsegment < 0 || info.nsegment > 4) return 0;
        for (int k = 0; k < info.nsegment; ++k) {
            uint32_t addr = (uint32_t)info.segmentaddr[k];
            uint32_t size = (uint32_t)info.segmentsize[k];
            if (!size) continue;
            if (protected_count >= PROTECTED_MAX || !range_ok(addr, size)) {
                note("invalid protected segment: fail closed"); return 0;
            }
            protected_ranges[protected_count].start = addr;
            protected_ranges[protected_count].end = addr + size;
            protected_count++;
        }
    }
    if (!self || !protected_count) {
        note("own module not identified: fail closed"); return 0;
    }
    note_hex("protected module ranges: ", (uint32_t)protected_count);
    return 1;
}

/* The built-in PPSSPP CWCheat engine invalidates JIT across all guest RAM.
   v0.4.2 invalidated only 0x08800000..0x08A00000, missing executable payload.
   Batch one master invalidation and record changed runtime addresses. */
typedef struct { uint32_t start, end; } DirtyRange;
static DirtyRange dirty_ranges[MAX_DIRTY_RANGES];
static unsigned int dirty_count;
static uint32_t dirty_min = MAX_GUEST, dirty_max;
static int dirty_bulk, dirty_overflow;
static unsigned int changed_writes;
static void begin_dirty(int master) {
    dirty_count = 0;
    dirty_min = MAX_GUEST;
    dirty_max = 0;
    dirty_bulk = master;
    dirty_overflow = 0;
    changed_writes = 0;
}
static void mark_dirty(uint32_t addr, uint32_t bytes) {
    if (!bytes || !range_ok(addr, bytes)) return;
    changed_writes++;
    uint32_t end = addr + bytes;
    if (addr < dirty_min) dirty_min = addr;
    if (end > dirty_max) dirty_max = end;
    if (dirty_bulk || dirty_overflow) return;
    if (dirty_count && addr <= dirty_ranges[dirty_count-1].end + 32u &&
        end >= dirty_ranges[dirty_count-1].start) {
        DirtyRange *last = &dirty_ranges[dirty_count-1];
        if (addr < last->start) last->start = addr;
        if (end > last->end) last->end = end;
    } else if (dirty_count < MAX_DIRTY_RANGES) {
        dirty_ranges[dirty_count].start = addr;
        dirty_ranges[dirty_count].end = end;
        dirty_count++;
    } else dirty_overflow = 1;
}
static void flush_dirty(void) {
    if (dirty_min >= dirty_max) return;
    if (dirty_bulk || dirty_overflow) {
        /* One deferred JIT invalidation over the touched master range. */
        sceKernelIcacheInvalidateRange((const void *)(uintptr_t)dirty_min,
                                      dirty_max - dirty_min);
    } else {
        for (unsigned int j = 0; j < dirty_count; ++j)
            sceKernelIcacheInvalidateRange(
                (const void *)(uintptr_t)dirty_ranges[j].start,
                dirty_ranges[j].end - dirty_ranges[j].start);
    }
}

static void setmem(uint32_t a, int size, uint32_t val) {
    if (!range_ok(a, size) || (a & (size - 1)) != 0) return;
    if (intersects_protected(a, (uint32_t)size)) { write_rejected = 1; return; }
    uint32_t mask = size == 1 ? 0xFFu : (size == 2 ? 0xFFFFu : 0xFFFFFFFFu);
    val &= mask;
    /* Do not constantly repatch code or invalidate every JIT block.
       This is the major fix over the rejected first experimental build. */
    if (getmem(a, size) == val) return;
    if (size == 1) *(volatile uint8_t *)(uintptr_t)a = (uint8_t)val;
    else if (size == 2) *(volatile uint16_t *)(uintptr_t)a = (uint16_t)val;
    else *(volatile uint32_t *)(uintptr_t)a = val;
    /* Emulated game code may be anywhere in expanded RAM; do not limit
       cache coherency to the first 2 MiB as v0.4.2 did. */
    mark_dirty(a, (uint32_t)size);
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
/* Conservatively scan every original direct master write before performing any.
 * This catches collisions with relocated PRX modules without altering ultrawide. */
static int preflight_master(const char *path, uint32_t count) {
    Reader rd;
    rd.fd = -1;
    if (!open_reader(&rd, path, count)) { close_reader(&rd); return 0; }
    Line x;
    unsigned int seen = 0;
    while (read_next(&rd, &x)) {
        ++seen;
        uint32_t type = x.a >> 28;
        if (type == 0 || type == 1 || type == 2) {
            uint32_t a = cwaddr(x.a);
            uint32_t size = type == 2 ? 4 : (type == 1 ? 2 :
                ((x.b & 0xFFFF0000u) ? 4 : ((x.b & 0xFF00u) ? 2 : 1)));
            if (!range_ok(a, size) || intersects_protected(a, size)) {
                note_hex("MASTER MODULE COLLISION at ", a);
                close_reader(&rd); return 0;
            }
        }
    }
    close_reader(&rd);
    if (seen != count) { note("master preflight read was incomplete"); return 0; }
    note("master preflight passed");
    return 1;
}
/* Suspend the game-update thread while writing a large, linked payload.
 * The previous build wrote asynchronously while user_main was executing. */
static SceUID suspend_user_main(void) {
    SceUID ids[64], self = sceKernelGetThreadId();
    int count = 0;
    if (sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, 64, &count) < 0 ||
        count > 64) return -1;
    for (int i = 0; i < count; ++i) {
        if (ids[i] == self) continue;
        SceKernelThreadInfo info;
        memset(&info, 0, sizeof(info)); info.size = sizeof(info);
        if (sceKernelReferThreadStatus(ids[i], &info) >= 0 &&
            strcmp(info.name, "user_main") == 0) {
            return sceKernelSuspendThread(ids[i]) >= 0 ? ids[i] : -1;
        }
    }
    return -1;
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
    write_rejected = 0;
    Line x, y;
    while (running && next(s, &x)) {
        if (write_rejected) { note("attempted module write: stop interpreter"); return 0; }
        uint32_t type = x.a >> 28, a = cwaddr(x.a), b = x.b;
        uint32_t n, addr, val;
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
            /* PPSSPP semantics: odd=addition, even=subtraction. */
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
            if (b && b < 24u * 1024u * 1024u && range_ok(a, b) && range_ok(to, b) && !intersects_protected(to, b)) {
                memmove((void *)(uintptr_t)to, (const void *)(uintptr_t)a, b);
                mark_dirty(to, b);
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
                    uint32_t second = x.b >> 28;
                    if (second == 2 || second == 3) {
                        int32_t follow = (int32_t)(x.b & 0x0FFFFFFFu);
                        if (second == 3) follow = -follow;
                        base = getmem(base + follow, 4);
                    }
                } else if (cmd == 1) {
                    uint32_t src = getmem(a, 4) + off;
                    uint32_t dst = getmem(base_addr, 4) + (x.a & 0x0FFFFFFFu);
                    if (val && val < 24u * 1024u * 1024u &&
                        range_ok(src, val) && range_ok(dst, val) && !intersects_protected(dst, val)) {
                        memmove((void *)(uintptr_t)dst, (const void *)(uintptr_t)src, val);
                        mark_dirty(dst, val);
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
        case 13: {
            /* KiwiYu uses D4-D7 address-to-address conditional comparisons.
               Follow PPSSPP semantics: consume second record, skip y.a records
               when comparison fails. */
            uint32_t op = b >> 28;
            if (op >= 4u && op <= 7u) {
                if (!next(s, &y)) return 0;
                uint32_t sz = 1u << (y.b & 15u);
                if (sz != 1u && sz != 2u && sz != 4u) return 0;
                uint32_t to = cwaddr(b);
                uint32_t left = getmem(a, (int)sz), right = getmem(to, (int)sz);
                if (!test_value(left, right, op - 4u)) skip(s, y.a);
            } else if (op == 0u || op == 2u) {
                uint32_t sz = op == 0u ? 2u : 1u;
                uint32_t val = b & (sz == 1u ? 255u : 65535u);
                if (!test_value(getmem(a, (int)sz), val, (b >> 20) & 15u))
                    skip(s, 1);
            } else {
                note("unsupported D condition; refusing unsafe runtime");
                return 0;
            }
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
    }
    return !write_rejected;
}
/* Load only the tiny 2,458-record runtime once at startup.
 * Never reopen/seek/read the memory-stick file during gameplay. */
static int load_runtime_once(void) {
    Reader rd;
    rd.fd = -1;
    cached_runtime = (Line *)malloc(sizeof(Line) * RUNTIME_LINES);
    if (!cached_runtime) { note("failed to allocate runtime cache"); return 0; }
    if (!open_reader(&rd, RUNTIME_PATH, RUNTIME_LINES)) {
        close_reader(&rd); note("runtime data invalid"); return 0;
    }
    unsigned int count = 0;
    while (count < RUNTIME_LINES && read_next(&rd, &cached_runtime[count])) count++;
    close_reader(&rd);
    if (count != RUNTIME_LINES) { note("runtime data truncated"); return 0; }
    cached_runtime_ready = 1;
    note("runtime cached in RAM; no gameplay I/O");
    return 1;
}
static int execute_runtime(void) {
    if (!cached_runtime_ready) return 0;
    Stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.data = cached_runtime;
    stream.count = RUNTIME_LINES;
    begin_dirty(0);
    int result = execute(&stream);
    flush_dirty();
    return result;
}
/* The rejected v0.3 ZIP enables legacy auto-cheats by default.
 * If the old gate or master payload is present before our own initialization,
 * do not run a second huge writer over already-modified guest RAM.
 * This is diagnostic protection, not a way to safely run both at once. */
static int legacy_master_conflict(void) {
    uint32_t gate = getmem(LEGACY_AUTOGATE_ADDR, 4);
    if (gate == 1u || gate == 2u) {
        note("CONFLICT: legacy Jin8 auto-cheat gate active; master NOT attempted");
        return 1;
    }
    if (getmem(MASTER_MARK, 4) == MASTER_FIRST &&
        getmem(MASTER_LAST, 4) == MASTER_LAST_VAL) {
        note("CONFLICT: pre-existing Jin8 master data; refusing duplicate write");
        return 1;
    }
    return 0;
}
static int load_master(const char *path, uint32_t count) {
    Reader rd;
    rd.fd = -1;
    if (!open_reader(&rd, path, count)) {
        close_reader(&rd); note("master file missing or bad header"); return 0;
    }
    Stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.reader = &rd;
    begin_dirty(1);
    int result = execute(&stream);
    flush_dirty();
    close_reader(&rd);
    if (!result) { note("master execution aborted"); return 0; }
    note("master JIT invalidation completed");
    return 1;
}
static int jin_fight_ready(uint32_t *moveset) {
    if (getmem(GAME_ID_ADDR, 4) != 9u) return 0;
    uint32_t ptr = getmem(MOVESET_ADDR, 4);
    if ((ptr & 0xFFFF0000u) != 0x086A0000u &&
        (ptr & 0xFFFF0000u) != 0x08750000u) return 0;
    if ((ptr & 3u) || !range_ok(ptr, 512)) return 0;
    /* Character-select can already have character ID and moveset pointer;
       wait for a meaningful battle current-move pointer before injecting. */
    uint32_t active = getmem(CURRENT_MOVE_POINTER_ADDR, 4);
    if ((active & 3u) || active < 0x08600000u ||
        active >= 0x09C00000u || !range_ok(active, 128)) return 0;
    if (getmem(CURRENT_MOVE_ADDR, 4) == 0u) return 0;
    if (moveset) *moveset = ptr;
    return 1;
}
static int loader_thread(SceSize args, void *argp) {
    (void)args; (void)argp;
    active_thread = 1;
    log_fd = sceIoOpen(LOG_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0666);
    note("KiwiYu Jin 1.0 standalone v0.2: activation/write telemetry");
    SceUID safefd = sceIoOpen(SAFE_PATH, PSP_O_RDONLY, 0);
    if (safefd >= 0) {
        sceIoClose(safefd);
        safe_mode = 1;
        note("SAFE MODE: no cheat writes; footprint/texture control only");
    }
    SceUID onlyfd = sceIoOpen(MASTER_ONLY_PATH, PSP_O_RDONLY, 0);
    if (onlyfd >= 0) { sceIoClose(onlyfd); master_only_mode = 1; note("MASTER-ONLY diagnostic enabled"); }
    if (!safe_mode && !load_runtime_once()) goto done;
    if (!safe_mode && !protect_loaded_modules()) goto done;
    int installed = 0, idle_cycles = 0;
    uint32_t prior_base = 0;
    uint32_t settle_base = 0;
    unsigned int settle_ticks = 0;
    BattleClock clock;
    battle_clock_reset(&clock);
    int round_clock_logged = 0;
    unsigned int round_wait_polls = 0;
    while (running) {
        uint32_t base = 0;
        if (!jin_fight_ready(&base)) {
            settle_ticks = 0;
            settle_base = 0;
            round_clock_logged = 0;
            round_wait_polls = 0;
            battle_clock_reset(&clock);
            if (++idle_cycles > 40) { installed = 0; prior_base = 0; }
            sceKernelDelayThread(25000); continue;
        }
        idle_cycles = 0;
        if (settle_base != base) {
            settle_base = base;
            settle_ticks = 0;
            round_clock_logged = 0;
            round_wait_polls = 0;
            battle_clock_reset(&clock);
            note("Jin move table candidate found; waiting for battle stability");
        }
        if (settle_ticks < FIGHT_SETTLE_TICKS) {
            ++settle_ticks;
            sceKernelDelayThread(25000);
            continue;
        }
        if (safe_mode) { sceKernelDelayThread(50000); continue; }
        /* A current move ID can exist while the game is still loading.
           Require the actual 60s round timer to count down in repeated
           samples; otherwise NEVER modify the game. No manual cheat toggle. */
        if (!installed || base != prior_base) {
            if (!round_clock_logged) {
                note("stable move table; waiting for active round timer (no writes yet)");
                round_clock_logged = 1;
            }
            uint32_t round_value = getmem(ROUND_TIMER_ADDR, 4);
            if (round_wait_polls == 0)
                note_hex("round timer first sample = ", round_value);
            if (!battle_clock_sample(&clock, round_value)) {
                if (++round_wait_polls == 240u) {
                    note_hex("round timer after ~6 seconds = ", round_value);
                    note("NO MASTER WRITES: waiting for supported countdown");
                    round_wait_polls = 0;
                }
                sceKernelDelayThread(25000);
                continue;
            }
            note_hex("round timer qualified at = ", round_value);
            note("ROUND TIMER ACTIVE: battle confirmed; master may initialize");
        }
        if (!installed || base != prior_base) {
            note("battle timer progressed and fight stable; checking master targets");
            if (legacy_master_conflict()) {
                note("Abort this fight; disable PPSSPP Cheats and remove old v0.3 INI");
                while (running && jin_fight_ready(0)) sceKernelDelayThread(50000);
                continue;
            }
            if (!preflight_master(MASTER_PATH, MASTER_LINES) ||
                !preflight_master(MASTER2_PATH, MASTER2_LINES)) {
                note("KiwiYu two-master preflight refused unsafe write, no initialization attempted");
                while (running && jin_fight_ready(0)) sceKernelDelayThread(50000);
                continue;
            }
            SceUID game = suspend_user_main();
            if (game < 0) {
                note("cannot suspend user_main, refusing unsynchronized master");
                while (running && jin_fight_ready(0)) sceKernelDelayThread(50000);
                continue;
            }
            note("user_main suspended; beginning KiwiYu master A");
            int master_ok = load_master(MASTER_PATH, MASTER_LINES);
            if (master_ok) {
                note("KiwiYu master A completed; applying master B");
                master_ok = load_master(MASTER2_PATH, MASTER2_LINES);
            }
            if (master_ok && (getmem(MASTER_MARK, 4) != MASTER_FIRST ||
                              getmem(MASTER_LAST, 4) != MASTER_LAST_VAL)) {
                note("KiwiYu two-master signature validation failed");
                master_ok = 0;
            }
            if (master_ok) note("both KiwiYu masters initialized (2 signatures)");
            sceKernelResumeThread(game);
            note("user_main resumed");
            if (!master_ok) {
                /* Fail closed and throttle retry; avoid infinite repeated 4MB writes. */
                note("master failed; waiting for next fight");
                installed = 0;
                while (running && jin_fight_ready(0)) sceKernelDelayThread(50000);
                continue;
            }
            installed = 1; prior_base = base;
        }
        if (master_only_mode) { sceKernelDelayThread(50000); continue; }
        /* The massive master ran while user_main was suspended, but the
           small cached runtime runs on the scheduler thread like a cheat
           tick. Avoid per-frame suspend/resume JIT hitches. */
        if (!logged_first_runtime) note("first runtime pass beginning");
        if (!execute_runtime()) {
            note("runtime execution failed; stopping interpreter for safety");
            goto done;
        }
        if (!logged_first_runtime) {
            note("first runtime pass completed");
            note_hex("first runtime changed write operations = ", changed_writes);
            if (!changed_writes)
                note("WARNING: runtime made NO changes on first pass; check state and move mapping");
            logged_first_runtime = 1;
        }
        sceKernelDelayThread(16000);
    }
done:
    note("KiwiYu loader stopped");
    if (cached_runtime) { free(cached_runtime); cached_runtime = NULL; cached_runtime_ready = 0; }
    if (log_fd >= 0) sceIoClose(log_fd);
    active_thread = 0;
    return 0;
}
int module_start(SceSize args, void *argp) {
    (void)args; (void)argp;
    running = 1;
    thread_id = sceKernelCreateThread("KiwiYuLoader", loader_thread, 0x20, 0x4000, 0, NULL);
    if (thread_id >= 0) sceKernelStartThread(thread_id, 0, NULL);
    return 0;
}
int module_stop(SceSize args, void *argp) {
    (void)args; (void)argp;
    running = 0;
    if (thread_id >= 0) sceKernelWaitThreadEnd(thread_id, NULL);
    return 0;
}
