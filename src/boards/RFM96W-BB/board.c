#include "board.h"

Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;


/*
 * MCU objects
 */
Gpio_t DcDcEnable;

Uart_t Uart1;
#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

uint8_t BoardMeasureBatterieLevel( void ) 
{
	return 254;
}

void BoardInitPeriph( void )
{
	GpioInit( &DcDcEnable, DC_DC_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
	GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led2, LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &Led3, LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void BoardInitMcu( void )
{
	if( McuInitialized == false )
	{
#if defined( USE_BOOTLOADER )
		// Set the Vector Table base location at 0x3000
		NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x3000 );
#endif
		// We use IRQ priority group 4 for the entire project
		// When setting the IRQ, only the preemption priority is used
		NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

		// Disable Systick
		SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;	// Systick IRQ off 
		SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;			// Clear SysTick Exception pending flag
			
		SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
		SX1276IoInit( );

		BoardUnusedIoInit( );

#if defined( USE_USB_CDC )
		UsbMcuInit( );
		UartInit( &UartUsb, UART_USB_CDC, NC, NC );
		UartConfig( &UartUsb, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );
#endif

#ifdef LOW_POWER_MODE_ENABLE
		RtcInit( );
#else
		TimerHwInit( );
#endif
		McuInitialized = true;
	}
}

void BoardDeInitMcu( void )
{
	SpiDeInit( &SX1276.Spi );
	SX1276IoDeInit( );

	McuInitialized = false;
}

void BoardGetUniqueId( uint8_t *id )
{
	id[0] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
	id[1] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
	id[2] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
	id[3] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
	id[4] = ( ( *( uint32_t* )ID2 ) ) >> 24;
	id[5] = ( ( *( uint32_t* )ID2 ) ) >> 16;
	id[6] = ( ( *( uint32_t* )ID2 ) ) >> 8;
	id[7] = ( ( *( uint32_t* )ID2 ) );
}

static void BoardUnusedIoInit( void )
{
#if (!defined( USE_USB_CDC ) || !defined( USE_DEBUGGER ))
	Gpio_t ioPin;
#endif

	/* USB */
#if !defined( USE_USB_CDC )
	GpioInit( &ioPin, USB_DM, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, USB_DP, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

#if defined( USE_DEBUGGER )
	DBGMCU_Config( DBGMCU_SLEEP | DBGMCU_STOP | DBGMCU_STANDBY, ENABLE);
#else
	DBGMCU_Config( DBGMCU_SLEEP | DBGMCU_STOP | DBGMCU_STANDBY, DISABLE );
	GpioInit( &ioPin, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	GpioInit( &ioPin, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif	
}
