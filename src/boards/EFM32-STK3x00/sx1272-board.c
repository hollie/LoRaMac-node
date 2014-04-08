#include "board.h"
#include "radio.h"

#include "sx1272/sx1272.h"
#include "sx1272-board.h"

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
	SX1272Init,
	SX1272GetStatus,
	
	SX1272SetChannel,
	SX1272IsChannelFree,
	SX1272SetRxConfig,
	SX1272SetTxConfig,
	SX1272GetTimeOnAir,
	SX1272Send,

	SX1272SetSleep,
	SX1272SetStby, 
	SX1272SetRx,
	
	SX1272ReadRssi,
	
	SX1272Write,
	SX1272Read,
	SX1272WriteBuffer,
	SX1272ReadBuffer
};

/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t AntRx;
Gpio_t AntTx;

void SX1272IoInit( void )
{
	GpioInit( &SX1272.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

	GpioInit( &SX1272.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &SX1272.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );

	SX1272AntSwInit( );
}

void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{
	GpioSetInterrupt( &SX1272.DIO0, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[0] );
	GpioSetInterrupt( &SX1272.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, irqHandlers[1] );
}

void SX1272IoDeInit( void )
{
	GpioInit( &SX1272.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

	GpioInit( &SX1272.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &SX1272.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX1272SetAntSwLowPower( bool status )
{
	if( RadioIsActive != status )
	{
		RadioIsActive = status;
	
		if( status == false )
		{
			SX1272AntSwInit( );
		}
		else
		{
			SX1272AntSwDeInit( );
		}
	}
}

void SX1272AntSwInit( void )
{
	GpioInit( &AntTx, RADIO_ANT_SWITCH_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
	GpioInit( &AntRx, RADIO_ANT_SWITCH_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SX1272AntSwDeInit( void )
{
	GpioInit( &AntTx, RADIO_ANT_SWITCH_TX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &AntRx, RADIO_ANT_SWITCH_RX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX1272SetAntSw( uint8_t rxTx )
{
	if( SX1272.RxTx == rxTx )
	{
		return;
	}

	SX1272.RxTx = rxTx;

	if( rxTx != 0 )
	{
		GpioWrite( &AntRx, 0 );
		GpioWrite( &AntTx, 1 );
	}
	else
	{
		GpioWrite( &AntRx, 1 );
		GpioWrite( &AntTx, 0 );
	}
}
