#ifndef SYSTIMER_H
#define SYSTIMER_H

#include <stdint.h>

typedef struct sysTimer
{
    bool isCounting;
    uint32_t startTime;
    uint32_t ticksToWait;
}sysTimer_t;

extern void SysTimer_Init(sysTimer_t* pSysTimer,uint32_t timerRepTime);
extern bool SysTimer_DoneCounting(sysTimer_t* pSysTimer);

#endif /* TIMER_H */