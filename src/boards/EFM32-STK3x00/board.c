#include "board.h"

/*!
 * LED GPIO pins objects
 */
Gpio_t Led1;
Gpio_t Led2;

#ifdef USE_I2C
	I2c_t I2c;
#endif
/*!
 * Flag to indicate if the MCU is Initialized
 */
bool McuInitialized = false;

/*!
 * Initializes the unused GPIO to a know status
 */
void BoardUnusedIoInit( void );

void Led_1_On(void)
{
	GpioWrite(&Led1, LED_1_ON_STATE);
}

void Led_1_Off(void)
{
	GpioWrite(&Led1, LED_1_OFF_STATE);
}

void Led_2_On(void)
{
	GpioWrite(&Led2, LED_2_ON_STATE);
}

void Led_2_Off(void)
{
	GpioWrite(&Led2, LED_2_OFF_STATE);
}

void BoardInitMcu( void )
{
	if( McuInitialized == false )
	{
		/* Chip errata */
		CHIP_Init();

		CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
		CMU_ClockSelectSet(cmuClock_HF,cmuSelect_HFXO);

		/* Enable clock for HF peripherals */
		CMU_ClockEnable(cmuClock_HFPER, true);
		CMU_ClockEnable(cmuClock_GPIO, true);

		GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, LED_1_ON_STATE );
		GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, LED_2_ON_STATE );

#ifdef USE_I2C
		I2cInit( &I2c, I2C_SCL, I2C_SDA );
#endif
		SX1272IoInit( );
		SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );

		BoardUnusedIoInit( );

#ifdef LOW_POWER_MODE_ENABLE
		#error LOW_POWER_MODE_ENABLE not supported
		RtcInit( );
#else
		TimerHwInit( );
#endif
		Led_1_Off();
		Led_2_Off();

		McuInitialized = true;
	}
}

void BoardDeInitMcu( void )
{
	/* Switching the CPU clock source to HFRCO */
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

	/* Disabling every oscillator except hfrco and lfxo */
	CMU_OscillatorEnable(cmuOsc_AUXHFRCO, false, true);
	CMU_OscillatorEnable(cmuOsc_HFXO, false, true);
	CMU_OscillatorEnable(cmuOsc_LFRCO, false, true);

#ifdef USE_I2C
	I2cDeInit( &I2c );
#endif

	SpiDeInit( &SX1272.Spi );
	SX1272IoDeInit( );

	GpioInit( &Led1, LED_1, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led2, LED_2, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	McuInitialized = false;
}

void BoardInitPeriph( void )
{
}

void BoardGetUniqueId( uint8_t *id )
{
	*((uint64_t *)id) = SYSTEM_GetUnique();
}

uint8_t BoardMeasureBatterieLevel( void ) 
{
	uint8_t batteryLevel = 0;
/*
	uint8_t i, result = 0;
	uint32_t tmpreg = 0;

	RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );

	PWR_PVDCmd( ENABLE ); // activate the voltage reference
	DelayMs( 1 );
	
	tmpreg = PWR->CR; // get the CR register for a read-modify-write
	
	for( i = 0; i <= 6; i++ ) 
	{
		PWR->CR = ( ( tmpreg & 0xFFFFFF1F ) | ( i << 5 ) ); // set PVD level from 0 to 6
		DelayMs( 1 );
		if( PWR_GetFlagStatus( PWR_FLAG_PVDO ) == 1 )
		{
			result |= 0x01 << i; // use 'result' as a bit array
		}
	}
	PWR_PVDCmd( DISABLE ); // shut down voltage reference
	
	switch( result ) { // transcribe result and detect invalid codes
		case 0x7F: 
			batteryLevel = 0x1F;	// Vbat < 1.9V
			break;
		case 0x7E: 
			batteryLevel = 0x3F;	// 1.9 < Vbat < 2.1
			break;
		case 0x7C: 
			batteryLevel = 0x5F;	// 2.1 < Vbat < 2.3
			break;
		case 0x78: 
			batteryLevel = 0x7F;	// 2.3 < Vbat < 2.5
			break;
		case 0x70: 
			batteryLevel = 0x9F;	// 2.5 < Vbat < 2.7
			break;
		case 0x60: 
			batteryLevel = 0xBF;	// 2.7 < Vbat < 2.9
			break;
		case 0x40: 
			batteryLevel = 0xDF;	// 2.9 < Vbat < 3.1
			break;
		case 0x00: 
			batteryLevel = 0xFE;	// Vbat > 3.1V
			break;
		default:   
			batteryLevel = 0xFF;	// Fail
			break;
	}
*/
	return batteryLevel;
}

void BoardUnusedIoInit( void )
{
}
