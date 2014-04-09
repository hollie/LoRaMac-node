#include "board.h"
#include "spi-board.h"

USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

/*!
 * MCU SPI peripherals enumeration
 */

void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
	GpioInit( &obj->Mosi, mosi, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Miso, miso, PIN_INPUT,  PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, sclk, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );

    if( nss != NC )
    {
		GpioInit( &obj->Nss, nss, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    }

    obj->Spi = USART1;
    if( nss == NC )
    {
        // 8 bits, CPOL = 0, CPHA = 0, MASTER
        SpiFormat( obj, 8, 0, 0, 0 );
    }
    else
    {
        // 8 bits, CPOL = 0, CPHA = 0, SLAVE
        SpiFormat( obj, 8, 0, 0, 1 );
    }
	
	CMU_ClockEnable(cmuClock_USART1, true);

	init.enable			= usartEnable;
	init.baudrate		= 5000000;
	init.databits		= usartDatabits8;
	init.msbf			= 0;
	init.master			= 1;
	init.clockMode		= usartClockMode0;
	init.prsRxEnable	= 0;
	init.autoTx			= 0;
	USART_InitSync(USART1, &init);
	/* Turn on automatic Chip Select control */
	// USART1->CTRL |= USART_CTRL_AUTOCS;
  
	/* Route to location 3 and enable TX, CS, CLK pins */
	USART1->ROUTE = USART_ROUTE_TXPEN |
					USART_ROUTE_RXPEN |
					// USART_ROUTE_CSPEN |
					USART_ROUTE_CLKPEN |
					USART_ROUTE_LOCATION_LOC3;
}

void SpiDeInit( Spi_t *obj )
{
	USART_Reset(USART1);

    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	if (obj->Nss.pin != NC)
		GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
	USART_ClockMode_TypeDef mode;

	USART_Enable(USART1, usartDisable);
    
    if( ( ( ( bits == 8 ) || ( bits == 16 ) ) == false ) ||
        ( ( ( cpol >= 0 ) && ( cpol <= 1 ) ) == false ) ||
        ( ( ( cpha >= 0 ) && ( cpha <= 1 ) ) == false ) )
    {
        // SPI error
        while( 1 );
    }

	if ( cpol == 0x01 )
	{
		if ( cpha == 0x01 )
			mode = usartClockMode2;	/** Clock idle high, sample on falling edge. */
		else
			mode = usartClockMode3; /** Clock idle high, sample on rising edge. */
	}
	else
	{
		if ( cpha == 0x01 )
			mode = usartClockMode1;	/** Clock idle low, sample on falling edge. */
		else
			mode = usartClockMode0; /** Clock idle low, sample on rising edge. */
	}

	init.databits		= ( bits == 8 ) ? usartDatabits8 : usartDatabits16;
	init.msbf			= 0;
	init.master			= ( slave == 0x01 ) ? 0 : 1;
	init.clockMode		= mode;
	init.prsRxEnable	= 0;
	init.autoTx			= 0;
	USART_InitSync(USART1, &init);
	/* Turn on automatic Chip Select control */
	// USART1->CTRL |= USART_CTRL_AUTOCS;
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    if( ( obj == NULL ) || ( obj->Spi ) == NULL )
        while( 1 )
			;
    USART_Tx(USART1, outData);
    return USART_Rx(USART1);
}

