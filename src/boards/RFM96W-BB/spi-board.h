#ifndef __SPI_MCU_H__
#define __SPI_MCU_H__

#include "stm32l1xx.h"
#include "gpio.h"

/*!
 * SPI driver structure definition
 */
struct Spi_s
{
    SPI_TypeDef *Spi;
    Gpio_t Mosi;
    Gpio_t Miso;
    Gpio_t Sclk;
    Gpio_t Nss;
};

#endif  // __SPI_MCU_H__
