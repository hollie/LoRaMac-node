/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board SPI driver implementation

Comments: When compiling this file with KEIL compiler the --gnu option must be
          added in order to support the __builtin_ffs function.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __SPI_MCU_H__
#define __SPI_MCU_H__

#include "em_device.h"
#include "gpio-board.h"

/*!
 * SPI driver structure definition
 */
struct Spi_s
{
    USART_TypeDef *Spi;
    Gpio_t Mosi;
    Gpio_t Miso;
    Gpio_t Sclk;
    Gpio_t Nss;
};

#endif  // __SPI_MCU_H__
