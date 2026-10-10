#ifndef JIN8_BATTLE_CLOCK_H
#define JIN8_BATTLE_CLOCK_H
#include <stdint.h>
/* Pure-state gate for the established PSP 60-second round timer at 0x090FBE4C.
 * A fight must show 8 plausible monotonically decreasing timer observations
 * and 20 additional valid poll intervals. Values out of range, unexpected
 * jumps, and pauses reset/reject the gate. Any unsupported game mode fails
 * closed: no automatic master writes, rather than risk corrupting loading. */
typedef struct {
    uint32_t previous;
    unsigned int drops;
    unsigned int steady;
    unsigned int settle;
    unsigned int seen;
} BattleClock;
static inline void battle_clock_reset(BattleClock *c) {
    c->previous = 0;
    c->drops = 0;
    c->steady = 0;
    c->settle = 0;
    c->seen = 0;
}
static inline int battle_clock_sample(BattleClock *c, uint32_t now) {
    if (now == 0u || now > 0x3000u) {
        battle_clock_reset(c);
        return 0;
    }
    if (!c->seen) {
        c->seen = 1;
        c->previous = now;
        return 0;
    }
    if (now > c->previous) {
        /* Round restarted or this address is not the round timer. */
        battle_clock_reset(c);
        c->seen = 1;
        c->previous = now;
        return 0;
    }
    uint32_t delta = c->previous - now;
    c->previous = now;
    if (delta > 12u) {
        battle_clock_reset(c);
        c->seen = 1;
        c->previous = now;
        return 0;
    }
    if (delta == 0u) {
        if (++c->steady > 60u) {
            battle_clock_reset(c);
            c->seen = 1;
            c->previous = now;
        }
        return 0;
    }
    c->steady = 0;
    if (c->drops < 8u) c->drops++;
    if (c->drops >= 8u && c->settle < 20u) c->settle++;
    return c->settle >= 20u;
}
#endif
