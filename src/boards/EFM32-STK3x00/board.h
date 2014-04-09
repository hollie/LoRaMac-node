#ifndef __BOARD_H__
#define __BOARD_H__
/*
             ---- STK3700 ------
              3V3  20  19  GND
               5V  18  17  PD7
       DIO1   PD6  16  15  PC6  DIO0
      LE_RX   PD5  14  13  PB12
      LE_TX   PD4  12  11  PB11 RF_RST
        NSS   PD3  10  9   PC5  I2C1_SCL
        SCK   PD2   8  7   PC4  I2C1_SDA
       MISO   PD1   6  5   PC3  SW_RX
       MOSI   PD0   4  3   PC0  SW_TX
             VMCU   2  1   GND

            ----- STK3200  -----
              3V3  20  19  EBID_SDA
               5V  18  17  EBID_SCL
   I2C0_SDA  PE12  16  15  PE13 I2C0_SCL
      LE_RX   PD5  14  13  PA1  DIO1
      LE_TX   PD4  12  11  PB11 RF_RST
        NSS  PC14  10  9   PA0  DIO0
        SCK  PC15   8  7   PC2  
       MISO   PD6   6  5   PC1  SW_TX
       MOSI   PD7   4  3   PC0  SW_RX
             VMCU   2  1   GND
*/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "em_device.h"
#include "em_chip.h"

#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_rtc.h"
#include "em_letimer.h"

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
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED	( DEVINFO->UNIQUEH ^ DEVINFO->UNIQUEL )

#ifdef EFM32ZG222F32
	/*!
	 * EFM32-STK3200
	 */
	#define LED_1			PC_10
	#define LED_1_OFF_STATE	0
	#define LED_1_ON_STATE	1
	#define LED_2			PC_11
	#define LED_2_OFF_STATE	0
	#define LED_2_ON_STATE	1

	#define RADIO_RESET		PB_11

	#define RADIO_MOSI		PD_7
	#define RADIO_MISO		PD_6
	#define RADIO_SCLK		PC_15
	#define RADIO_NSS		PC_14

	#define RADIO_DIO_0		PA_0
	#define RADIO_DIO_1		PA_1

	#define RADIO_ANT_SWITCH_RX		PC_0
	#define RADIO_ANT_SWITCH_TX		PC_1

	#define I2C_SCL			PE_13
	#define I2C_SDA			PE_12
#endif

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

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
