#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "em_device.h"

#include "gpio.h"
#include "i2c.h"
#include "timer.h"
#include "spi.h"
#include "radio.h"

#include "utilities.h"
#include "delay.h"

#include "sx1272/sx1272.h"
#include "rtc-board.h"
#include "timer-board.h"
#include "sx1272-board.h"

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
	#define SUCCESS	1
#endif

#ifndef FAIL
	#define FAIL	0
#endif

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define ID1	( 0x1FF80050 )
#define ID2	( 0x1FF80054 )
#define ID3	( 0x1FF80064 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED	(	(*( uint32_t*)ID1 ) ^ \
						(*( uint32_t*)ID2 ) ^ \
						(*( uint32_t* )ID3) )

/*!
 * Board MCU pins definitions
 */
#define LED_1			PB_9
#define LED_2			PB_8

#define RADIO_RESET		PC_2

#define RADIO_MOSI		PA_7
#define RADIO_MISO		PA_6
#define RADIO_SCLK		PA_5
#define RADIO_NSS		PA_4

#define RADIO_DIO_0		PA_0
#define RADIO_DIO_1		PA_1

#define RADIO_ANT_SWITCH_RX		PC_0
#define RADIO_ANT_SWITCH_TX		PC_1

#define OSC_LSE_IN		PC_14
#define OSC_LSE_OUT		PC_15

#define OSC_HSE_IN		PH_0
#define OSC_HSE_OUT		PH_1

#define USB_DM			PA_11
#define USB_DP			PA_12

#define I2C_SCL			PB_10
#define I2C_SDA			PB_11

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;
extern Gpio_t Led3;

/*!
 * MCU objects
 */
extern I2c_t I2c;

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

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
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardMeasureBatterieLevel( void );

/*!
 * \brief Gets the board 64 bits unique ID 
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#endif // __BOARD_H__
