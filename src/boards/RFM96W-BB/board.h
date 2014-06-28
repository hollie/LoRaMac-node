#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "stm32l1xx.h"

#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "uart.h"
#include "radio.h"
#include "rtc-board.h"
#include "timer-board.h"
#include "sx1276-board.h"
#include "uart-board.h"

#if defined( USE_USB_CDC )
	#include "usb-cdc-board.h"
#endif

/*!
 * NULL definition
 */
#ifndef NULL
    #define NULL	( ( void * )0 )
#endif

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS		1
#endif

#ifndef FAIL
#define FAIL		0  
#endif

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define ID1			( 0x1FF80050 )
#define ID2			( 0x1FF80054 )
#define ID3			( 0x1FF80064 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED	(( *( uint32_t* )ID1 ) ^ \
					 ( *( uint32_t* )ID2 ) ^ \
					 ( *( uint32_t* )ID3 ))

/*!
 * Board IO Extender pins definitions
 */
#define LED_1				PB_11
#define LED_2				PB_10
#define LED_3				PA_3

#define LED_1_ON()			GpioWrite( &Led1, 1 )
#define LED_1_OFF()			GpioWrite( &Led1, 0 )
#define LED_2_ON()			GpioWrite( &Led2, 1 )
#define LED_2_OFF()			GpioWrite( &Led2, 0 )
#define LED_3_ON()			GpioWrite( &Led3, 1 )
#define LED_3_OFF()			GpioWrite( &Led3, 0 )

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET			PB_15

#define RADIO_MOSI			PB_5
#define RADIO_MISO			PB_4
#define RADIO_SCLK			PB_3
#define RADIO_NSS			PA_15

#define RADIO_DIO_0			PB_6
#define RADIO_DIO_1			PB_7
#define RADIO_DIO_2			PB_8
#define RADIO_DIO_3			PA_2
#define RADIO_DIO_4			PA_1
#define RADIO_DIO_5			PB_12

#define USB_DM				PA_11
#define USB_DP				PA_12

#define UART_TX				PA_9
#define UART_RX				PA_10    

#define DC_DC_EN			PB_9
#define USB_ON				PA_8

#define SWDIO				PA_13
#define SWCLK				PA_14

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;
extern Gpio_t Led3;

/*!
 * MCU objects
 */
extern Uart_t Uart1;
#if defined( USE_USB_CDC )
extern Uart_t UartUsb;
#endif

extern Gpio_t DcDcEnable;

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardMeasureBatterieLevel( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Gets the board 64 bits unique ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#endif // __BOARD_H__
