#ifndef JIN_KIWIYU_BATTLE_ACTIVITY_H
#define JIN_KIWIYU_BATTLE_ACTIVITY_H
#include <stdint.h>
/* Stock ULUS10466 snapshots captured during real P1 fights:
 * neutral input 0x00FF0000; button activity 0x00FE0100/0x00FD0200,
 * current move IDs include 0x01A6/0x01D0. Require *both* player
 * input onset and a real move change, followed by ~300ms of stability.
 * Current-move pointers ALONE exist during stage loading: insufficient. */
typedef struct {
    uint32_t prior_move;
    uint32_t prior_input;
    unsigned int initialized;
    unsigned int input_window;
    unsigned int move_event;
    unsigned int after_activity;
} BattleActivity;
static inline void battle_activity_reset(BattleActivity *s) {
    s->prior_move=0;
    s->prior_input=0;
    s->initialized=0;
    s->input_window=0;
    s->move_event=0;
    s->after_activity=0;
}
static inline int battle_activity_sample(BattleActivity *s, uint32_t move, uint32_t input) {
    const int move_plausible = (move > 0u && move < 0x8000u);
    const int button_active = ((input & 0xFF000000u) == 0 &&
        ((input >> 16) & 0xFFu) != 0xFFu &&
        (input & 0x0000FF00u) != 0);
    if (!s->initialized) {
        s->initialized=1;
        s->prior_move=move;
        s->prior_input=input;
        return 0;
    }
    if (button_active && s->prior_input == 0x00FF0000u)
        s->input_window=80;
    if (move_plausible && s->input_window && move != s->prior_move)
        s->move_event=1;
    s->prior_move=move;
    s->prior_input=input;
    if (s->input_window) s->input_window--;
    if (s->move_event && s->input_window) {
        if (++s->after_activity >= 12) return 1;
    } else if (!s->input_window) {
        s->move_event=0;
        s->after_activity=0;
    }
    return 0;
}
#endif
