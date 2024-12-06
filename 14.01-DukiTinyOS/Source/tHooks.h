#ifndef __THOOKS_H
#define __THOOKS_H

void tHooksCpuIdle(void);
void tHooksSysTick(void); 
void tHooksTaskSwitch(tTask * from, tTask * to);
void tHooksTaskInit(tTask * task);

#endif
