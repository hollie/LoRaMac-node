#include "board.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "dfu_mal.h"

#include "usb-dfu-board.h"

typedef  void ( *pFunction )( void );

uint8_t DeviceState;
uint8_t DeviceStatus[6];

/*
 * Board peripherals objects
 */
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

Gpio_t DcDcEnable;
Gpio_t UartRx;

void DelayLoop( __IO uint32_t nCount )
{
	__IO uint32_t index = 0;
	for( index = ( 5000 * nCount ); index != 0; index-- )
	{
	}
}

int main(void)
{
	SystemCoreClockUpdate();

    GpioInit( &UartRx,		UART_RX,	PIN_INPUT,	PIN_PUSH_PULL, PIN_PULL_UP, 1 );
	GpioInit( &DcDcEnable,	DC_DC_EN,	PIN_OUTPUT,	PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led1,		LED_1,		PIN_OUTPUT,	PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led2,		LED_2,		PIN_OUTPUT,	PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led3,		LED_3,		PIN_OUTPUT,	PIN_PUSH_PULL, PIN_NO_PULL, 0 );

	if( GpioRead( &UartRx ) == 1 )
	{	/* Test if user code is programmed starting from address 0x8003000 */
		if ((( *( __IO uint32_t* )ApplicationAddress ) & 0x2FFE0000 ) == 0x20000000 )
		{
			/* Initialize user application's Stack Pointer */
			__set_MSP( *( __IO uint32_t* ) ApplicationAddress );
			/* Jump to user application */
			pFunction Jump_To_Application = ( pFunction )(*( __IO uint32_t* )( ApplicationAddress + 4 ));
			Jump_To_Application( );
		}
	} /* Otherwise enters DFU mode to allow user to program his application */

	/* Enter DFU mode */
	DeviceState = STATE_dfuERROR;
	DeviceStatus[0] = STATUS_ERRFIRMWARE;
	DeviceStatus[4] = DeviceState;

	UsbMcuInit( );

	/* Main loop */
	while (1)
	{
		GpioWrite( &Led2, 0 );
		DelayLoop( 500 );
		GpioWrite( &Led2, 1 );
		DelayLoop( 500 );
	}
}
