#include <math.h>
#include <time.h>

#include "board.h"
#include "rtc-board.h"

/*!
 * RTC Time base in us
 */
#define RTC_ALARM_TIME_BASE		122.07

/*!
 * MCU Wake Up Time
 */
#define MCU_WAKE_UP_TIME		3400

/*!
 * \brief Configure the Rtc hardware
 */
void RtcSetConfig( void );

/*!
 * \brief Start the Rtc Alarm (time base 1s)
 */
void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Read the MCU internal Calendar value
 *
 * \retval Calendar value
 */
uint64_t RtcGetCalendarValue( void );

/*!
 * \brief Clear the RTC flags and Stop all IRQs
 */
void RtcClearStatus( void );

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
bool RtcInitalized = false;

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough 
 * to set the MCU into low power mode
 */
bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode 
 */
bool LowPowerDisableDuringTask = false;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 */
uint64_t RtcTimerContext = 0;

/*!
 * Number of seconds in a minute
 */
const uint8_t SecondsInMinute = 60;

/*!
 * Number of seconds in an hour
 */
const uint16_t SecondsInHour = (60 * 60);

/*!
 * Number of seconds in a day
 */
const uint32_t SecondsInDay = (60 * 60 * 24);

/*!
 * Number of hours in a day
 */
const uint8_t HoursInDay = 24;

/*!
 * Number of days in a standard year
 */
const uint16_t DaysInYear = 365;

/*!
 * Number of days in a leap year
 */
const uint16_t DaysInLeapYear = 366;

/*!
 * Number of days in a century
 */
const uint16_t DaysInCentury = 36524;

/*!
 * Number of days in each month on a normal year
 */
const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Hold the previous year value to detect the turn of a century
 */
uint8_t PreviousYear = 0;

/*!
 * Century counter
 */
uint8_t Century = 0;

void RtcInit( void )
{
	if ( RtcInitalized == false )
	{
		RtcSetConfig( );
		RtcInitalized = true;
	}
}

void RtcSetConfig( void )
{
	RTC_Init_TypeDef rtcInit = RTC_INIT_DEFAULT;

	/* Starting LFRCO and waiting until it is stable */
	CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

	/* Routing the LFRCO clock to the RTC */
	CMU_ClockSelectSet(cmuClock_LFA,cmuSelect_LFRCO);
	CMU_ClockEnable(cmuClock_RTC, true);

	/* Prescaler of 4 = 122.07 us of resolution */
	CMU_ClockDivSet(cmuClock_RTC,cmuClkDiv_4);

	/* Enabling clock to the interface of the low energy modules */
	CMU_ClockEnable(cmuClock_CORELE, true);

	rtcInit.enable   = true;	/* Enable RTC after init has run */
	rtcInit.comp0Top = true;	/* Clear counter on compare match */
	rtcInit.debugRun = false;	/* Counter shall keep running during debug halt. */

	RTC_Init(&rtcInit);			/* Initialize the RTC */
}

uint32_t RtcGetMinimumTimeout( void )
{
	return( ceil( 3 * RTC_ALARM_TIME_BASE ) );
}
/*
void RtcSetTimeout( uint32_t timeout )
{
	if ( timeout < ( 3 * RTC_ALARM_TIME_BASE ) )
		timeout = 3 * RTC_ALARM_TIME_BASE;
	
	// we don't go in Low Power mode for delay below 50ms (needed for LEDs)
	RtcTimerEventAllowsLowPower = !( timeout < 55000 );
	RtcStartWakeUpAlarm( timeout );
}
*/

uint32_t RtcGetTimerElapsedTime( void )
{
	uint64_t CalendarValue = 0;
	CalendarValue = RtcGetCalendarValue( );
	return( ( uint32_t )( ceil ((CalendarValue - RtcTimerContext + 2) * RTC_ALARM_TIME_BASE)) );
}

uint64_t RtcGetTimerValue( void )
{
	uint64_t CalendarValue = 0;
	CalendarValue = RtcGetCalendarValue( );
	return( ( CalendarValue + 2 ) * RTC_ALARM_TIME_BASE );
}

void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
#if 0
	uint8_t rtcSeconds = 0;
	uint8_t rtcMinutes = 0;
	uint8_t rtcHours = 0;

	uint8_t rtcAlarmSeconds = 0;
	uint8_t rtcAlarmMinutes = 0;
	uint8_t rtcAlarmHours = 0;
	uint16_t rtcAlarmDays = 0;

	RTC_AlarmTypeDef RTC_AlarmStructure;
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	RtcClearStatus( );

	RTC_GetTime( RTC_Format_BIN, &RTC_TimeStruct ); 
	RTC_GetDate( RTC_Format_BIN, &RTC_DateStruct ); 

	if ( ( PreviousYear == 99 ) && ( RTC_DateStruct.RTC_Year == 0 ) )
	{
		Century++;
	}
	PreviousYear = RTC_DateStruct.RTC_Year;

	RtcTimerContext = RtcGetCalendarValue( );
	   
	timeoutValue = timeoutValue / RTC_ALARM_TIME_BASE;

	rtcSeconds = timeoutValue % SecondsInMinute;
	rtcMinutes = ( timeoutValue / SecondsInMinute ) % SecondsInMinute;
	rtcHours = ( timeoutValue / SecondsInHour ) % HoursInDay;

	rtcAlarmSeconds = ( rtcSeconds + RTC_TimeStruct.RTC_Seconds ) % 60;
	rtcAlarmMinutes = ( ( ( rtcSeconds + RTC_TimeStruct.RTC_Seconds ) / 60 ) + ( rtcMinutes + RTC_TimeStruct.RTC_Minutes % 60 ) ) % 60;
	rtcAlarmHours = ( ( ( ( rtcSeconds + RTC_TimeStruct.RTC_Seconds ) / 60 ) + ( rtcMinutes + RTC_TimeStruct.RTC_Minutes % 60 ) ) / 60 ); 
	rtcAlarmHours = ( rtcAlarmHours + ( rtcHours + RTC_TimeStruct.RTC_Hours ) ) % 24;
	rtcAlarmDays = ( ( ( ( rtcSeconds + RTC_TimeStruct.RTC_Seconds ) / 60 ) + ( rtcMinutes + RTC_TimeStruct.RTC_Minutes % 60 ) ) / 60 );
	rtcAlarmDays = ( rtcAlarmDays + ( rtcHours + RTC_TimeStruct.RTC_Hours ) ) / 24;	  
	rtcAlarmDays = ( rtcAlarmDays + ( timeoutValue / SecondsInDay ) + RTC_DateStruct.RTC_Date );		

	if ( ( RTC_DateStruct.RTC_Year != 0 ) && ( RTC_DateStruct.RTC_Year % 4 == 0 ) )
	{
		if ( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.RTC_Month - 1 ] )
		{   
			rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.RTC_Month - 1 ];
		}
	}
	else
	{
		if ( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.RTC_Month - 1 ] )
		{   
			rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.RTC_Month - 1 ];
		}
	}
	
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = rtcAlarmSeconds;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = rtcAlarmMinutes;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = rtcAlarmHours;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay	  = rtcAlarmDays;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12	 = RTC_TimeStruct.RTC_H12;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel   = RTC_AlarmDateWeekDaySel_Date;
	RTC_AlarmStructure.RTC_AlarmMask			 = RTC_AlarmMask_None;
	RTC_SetAlarm( RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure );

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro( );
	
	/* Enable RTC Alarm A Interrupt */
	RTC_ITConfig( RTC_IT_ALRA, ENABLE );
  
	/* Enable the Alarm A */
	RTC_AlarmCmd( RTC_Alarm_A, ENABLE );
#endif
}

void RtcEnterLowPowerStopMode( void )
{   
#ifdef LOW_POWER_MODE_ENABLE
	if ( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
	{
		EMU_EnterEM2(false);
	}
#else
	if ( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
	{
		EMU_EnterEM1();
	}
#endif
}

void RtcRecoverMcuStatus( void )
{
}

/*!
 * \brief RTC IRQ Handler on the RTC Alarm
 */
void RTC_IRQHandler( void )
{
	if ( RTC_IntGet() & RTC_IF_COMP0 )
	{
		RtcRecoverMcuStatus( );
		TimerIrqHandler( );
		RTC_IntClear(RTC_IFC_COMP0);
	}
}

void BlockLowPowerDuringTask ( bool status )
{
	if ( status == true )
	{
		RtcRecoverMcuStatus( );
	}
	LowPowerDisableDuringTask = status;
}

void RtcDelayMs ( uint32_t delay )
{
	uint64_t delayValue = 0;
	uint64_t timeout = 0;

	delayValue = ( uint64_t )( delay * 1000 );

	// Wait delay ms
	timeout = RtcGetTimerValue( );
	while( ( ( RtcGetTimerValue( ) - timeout ) ) < delayValue )
	{
		__NOP( );
	}
}

uint64_t RtcGetCalendarValue( void )
{
	uint64_t calendarValue = 0;
#if 0
	uint8_t i = 0;

	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	RTC_GetTime( RTC_Format_BIN, &RTC_TimeStruct ); 
	RTC_GetDate( RTC_Format_BIN, &RTC_DateStruct ); 

	RTC_WaitForSynchro( );

	if ( ( PreviousYear == 99 ) && ( RTC_DateStruct.RTC_Year == 0 ) )
	{
		Century++;
	}
	PreviousYear = RTC_DateStruct.RTC_Year;

	// century
	for( i = 0; i < Century; i++ )
	{
		calendarValue += DaysInCentury * SecondsInDay;
	}

	// years
	for( i = 0; i < RTC_DateStruct.RTC_Year; i++ )
	{
		if ( ( i != 0 ) && ( i % 4 == 0 ) )
		{
			calendarValue += DaysInLeapYear * SecondsInDay * RTC_DateStruct.RTC_Year;
		}
		else
		{
			calendarValue += DaysInYear * SecondsInDay * RTC_DateStruct.RTC_Year;
		}
	}

	// months
	if ( ( RTC_DateStruct.RTC_Year != 0 ) && ( RTC_DateStruct.RTC_Year % 4 == 0 ) )
	{
		for( i = 1; i < RTC_DateStruct.RTC_Month; i++ )
		{
			calendarValue += DaysInMonthLeapYear[ i - 1] * SecondsInDay;
		}
	}
	else
	{
		for( i = 1;  i < RTC_DateStruct.RTC_Month; i++ )
		{
			calendarValue += DaysInMonth[ i ] * SecondsInDay;
		}
	}	   

	// days
	calendarValue += ( ( uint32_t )RTC_TimeStruct.RTC_Seconds + 
					  ( ( uint32_t )RTC_TimeStruct.RTC_Minutes * 60 ) +
					  ( ( uint32_t )RTC_TimeStruct.RTC_Hours * SecondsInHour ) + 
					  ( ( uint32_t )RTC_DateStruct.RTC_Date * SecondsInDay ) ); 

#endif
	return( calendarValue );
}
