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

    obj->Spi = RADIO_SPI;
    if ( nss == NC )
    {
        SpiFormat( obj, 8, 0, 0, 0 );	// 8 bits, CPOL = 0, CPHA = 0, MASTER
    }
    else
    {
		GpioInit( &obj->Nss, nss, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
        SpiFormat( obj, 8, 0, 0, 1 );	// 8 bits, CPOL = 0, CPHA = 0, SLAVE
    }

	CMU_ClockEnable(RADIO_SPI_CLK, true);

	init.enable			= usartEnable;
	init.baudrate		= 5000000;
	init.databits		= usartDatabits8;
	init.msbf			= 0;
	init.master			= 1;
	init.clockMode		= usartClockMode0;
	init.prsRxEnable	= 0;
	init.autoTx			= 0;
	USART_InitSync(obj->Spi, &init);
	/* Turn on automatic Chip Select control */
	// obj->Spi->CTRL |= USART_CTRL_AUTOCS;
  
	/* Route to location 3 and enable TX, CS, CLK pins */
	obj->Spi->ROUTE =	USART_ROUTE_TXPEN |
						USART_ROUTE_RXPEN |
						// USART_ROUTE_CSPEN |
						USART_ROUTE_CLKPEN |
						RADIO_SPI_LOC;
}

void SpiDeInit( Spi_t *obj )
{
	obj->Spi->ROUTE = 0;
	USART_Reset(obj->Spi);

	CMU_ClockEnable(RADIO_SPI_CLK, true);

    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	if (obj->Nss.pin != NC)
		GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
	USART_ClockMode_TypeDef mode;
	USART_Enable(obj->Spi, usartDisable);

    if ( !(bits == 8 || bits == 16) ||
		!(cpol == 0 || cpol == 1 ) ||
        !(cpha == 0 || cpha == 1 ))
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
	USART_InitSync(obj->Spi, &init);
	/* Turn on automatic Chip Select control */
	// obj->Spi->CTRL |= USART_CTRL_AUTOCS;
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    if (obj == NULL || obj->Spi == NULL)
        while( 1 )
			;
    USART_Tx(obj->Spi, outData);
    return USART_Rx(obj->Spi);
}

