#include <assert.h>
#include "battle_activity.h"
int main(void) {
 BattleActivity a;
 battle_activity_reset(&a);
 for(int i=0;i<180;i++) assert(!battle_activity_sample(&a,0x8001,0x00FF0000));
 for(int i=0;i<50;i++) assert(!battle_activity_sample(&a,0x1D0+(unsigned)i,0x00FF0000));
 battle_activity_reset(&a);
 assert(!battle_activity_sample(&a,0x8001,0x00FF0000));
 assert(!battle_activity_sample(&a,0x1D0,0x00FE0100));
 for(int i=0;i<10;i++) assert(!battle_activity_sample(&a,0x1D0,0x00FF0000));
 assert(battle_activity_sample(&a,0x1D0,0x00FF0000));
 battle_activity_reset(&a);
 assert(!battle_activity_sample(&a,0x8001,0x00FF0000));
 for(int i=0;i<10;i++) assert(!battle_activity_sample(&a,0x8001,0x00FE0100));
 return 0;
}
