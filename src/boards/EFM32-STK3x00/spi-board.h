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
