#include <assert.h>
#include "battle_clock.h"
int main(void) {
    BattleClock c;
    battle_clock_reset(&c);
    assert(!battle_clock_sample(&c, 0));
    assert(!battle_clock_sample(&c, 0xFFFFFFFFu));
    assert(!battle_clock_sample(&c, 3601u));
    for (int n = 0; n < 26; ++n)
        assert(battle_clock_sample(&c, 3600u-(uint32_t)n) == (n>=26));
    assert(battle_clock_sample(&c, 3573u));
    /* Merely stable, non-moving values NEVER authorize master writes. */
    battle_clock_reset(&c);
    for(int i=0;i<90;i++) assert(!battle_clock_sample(&c, 3600u));
    battle_clock_reset(&c);
    for(int i=0;i<8;i++) assert(!battle_clock_sample(&c, 200000u));
    battle_clock_reset(&c);
    for(int i=0;i<6;i++) assert(!battle_clock_sample(&c, 3600u-(uint32_t)i));
    assert(!battle_clock_sample(&c, 3900u)); /* Round reset. */
    return 0;
}
