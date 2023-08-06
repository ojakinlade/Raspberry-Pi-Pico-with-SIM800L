#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/structs/systick.h"
#include "sysTimer.h"

static volatile uint32_t numOfTicks = 0;

static void SysTick_Init(void)
{
    systick_hw->csr = 0;        // Disable
    systick_hw->rvr = 124999UL; // standard system clock (125MHz) / (rvr value + 1) = 1ms
    systick_hw->csr = 0x7;       // Enable Systick, enable exceptions
}

extern "C" void isr_systick()
{
    numOfTicks++;
}

static uint32_t SysTick_GetTick(void)
{
    return numOfTicks;
}

void SysTimer_Init(sysTimer_t* pSysTimer,uint32_t timerRepTime)
{
    SysTick_Init();
    pSysTimer->startTime = 0;
    pSysTimer->ticksToWait = timerRepTime;
    pSysTimer->isCounting = false;
}

bool SysTimer_DoneCounting(sysTimer_t* pSysTimer)
{
    bool countingComplete = false;

    if(!pSysTimer->isCounting)
    {
        pSysTimer->startTime = SysTick_GetTick();
        pSysTimer->isCounting = true;
    }
    else
    {
        if((SysTick_GetTick() - pSysTimer->startTime) >= pSysTimer->ticksToWait)
        {
            countingComplete = true;
            pSysTimer->startTime = 0;
            pSysTimer->isCounting = false;
        }
    }
    return countingComplete;
}

