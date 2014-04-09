#include <math.h>
#include "board.h"
#include "timer-board.h"

/*!
 * Hardware Time base in us
 */
#define HW_TIMER_TIME_BASE		1000 //us

/*!
 * Hardware Timer tick counter
 */
volatile uint64_t TimerTickCounter = 1;

/*!
 * Saved value of the Tick counter at the start of the next event
 */
uint64_t TimerTickCounterContext = 0;

/*!
 * Value trigging the IRQ
 */
volatile uint64_t TimeoutCntValue = 0;

void TimerHwInit( void )
{
	if (SysTick_Config(SystemCoreClockGet() / 1000))
		while (1)
			;
}

void TimerHwDeInit( void )
{
}

uint32_t TimerHwGetMinimumTimeout( void )
{
    return( ceil( 2 * HW_TIMER_TIME_BASE ) );
}

void TimerHwStart( uint32_t val )
{
    TimerTickCounterContext = TimerHwGetTimerValue( );

    if( val <= HW_TIMER_TIME_BASE + 1 )
        TimeoutCntValue = TimerTickCounterContext + 1;
    else
        TimeoutCntValue = TimerTickCounterContext + ( ( val - 1 ) / HW_TIMER_TIME_BASE );
}

void TimerHwStop( void )
{
	/* Disable Systick */
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;	// Systick IRQ off
	SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;			// Clear SysTick Exception pending flag
}

volatile uint32_t TimerDownCount;

void TimerHwDelayMs( uint32_t delay_ms )
{
	TimerDownCount = delay_ms;
	while (TimerDownCount != 0)
	{
		EMU_EnterEM1();
	}
}

uint64_t TimerHwGetElapsedTime( void )
{
     return( ( ( TimerHwGetTimerValue( ) - TimerTickCounterContext ) + 1 )  * HW_TIMER_TIME_BASE );
}

uint64_t TimerHwGetTimerValue( void )
{
    uint64_t val;
	int irq = __disable_irq( );
    val = TimerTickCounter;
    if (irq)
		__enable_irq( );
    return( val );
}

/*!
 * Timer IRQ handler
 */
void SysTick_Handler(void)
{
	TimerTickCounter++;
	if (TimerDownCount != 0)
		--TimerDownCount;

	if( TimerTickCounter == TimeoutCntValue )
		TimerIrqHandler( );
}

void TimerHwEnterLowPowerStopMode( void )
{
	EMU_EnterEM1();
}
