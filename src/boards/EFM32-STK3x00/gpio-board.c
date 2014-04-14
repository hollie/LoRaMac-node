#include "board.h"
#include "gpio-board.h"
#include "em_gpio.h"
#include "em_int.h"

GpioIrqHandler *GpioIrq[16];

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint16_t value )
{
	GPIO_Mode_TypeDef pinMode;

	obj->pin = pin;
	if( pin == NC ) 
		return;

	obj->portIndex = ( uint16_t ) pin >> 4;
	obj->pinIndex = 1 << (obj->pin & 0x0F);

	if( obj->portIndex > (uint16_t)gpioPortF )
	{
		obj->pin = NC;
		return;
	}

	switch (mode)
	{
		case PIN_INPUT:
			switch (type)
			{
				case PIN_NO_PULL:
					pinMode = gpioModeInput;
					break;
				case PIN_PULL_UP:
					pinMode = gpioModeInputPull;
					value = 1;
					break;
				case PIN_PULL_DOWN:
					pinMode = gpioModeInputPull;
					value = 0;
					break;
				default:
					obj->pin = NC;
					return;
			}
			break;
		case PIN_OUTPUT:
			switch (config)
			{
				case PIN_PUSH_PULL:
					pinMode = gpioModePushPull;
					break;
				case PIN_OPEN_DRAIN:
					switch(type)
					{
						case PIN_NO_PULL:
							pinMode = gpioModeWiredAnd;
							break;
						case PIN_PULL_UP:
							pinMode = gpioModeWiredAndPullUp;
							value = 1;
							break;
						case PIN_PULL_DOWN:
							pinMode = gpioModeWiredAndPullUp;
							value = 0;
							break;
						default:
							obj->pin = NC;
							return;
					}
					break;
				default:
					obj->pin = NC;
					return;
			}
			break;
		case PIN_ALTERNATE_FCT:
			break;
		case PIN_ANALOGIC:
			pinMode = gpioModeDisabled;
			value = 0;
			break;
		default:
			obj->pin = NC;
			return;
	}
	
	GPIO_PinModeSet(
		(GPIO_Port_TypeDef)(obj->portIndex),
		(obj->pin & 0x0F),
		pinMode,
		value
	);
}

void GpioMcuSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
	unsigned int pin = obj->pin & 0x0F;
	bool rising = false;
	bool falling = false;
	
	if (obj->pin == NC)
		return;

	if( irqHandler == NULL )
	{
		GPIO_IntConfig(
			(GPIO_Port_TypeDef)(obj->portIndex),
			pin, false, false, false
		);
		return;
	}

	GpioIrq[pin] = irqHandler;

	if( irqMode == IRQ_RISING_EDGE )
		rising = true;
	else if( irqMode == IRQ_FALLING_EDGE )
		falling = true;
	else if( irqMode == IRQ_RISING_FALLING_EDGE )
		falling = rising = true;
	else
	{
		while( 1 );
	}
		
	GPIO_IntConfig((GPIO_Port_TypeDef)(obj->portIndex), pin, rising, falling, true);
 
	if (obj->pin & 0x01)
		NVIC_EnableIRQ(GPIO_ODD_IRQn);
	else
		NVIC_EnableIRQ(GPIO_EVEN_IRQn);
	/*
	if( irqPriority == IRQ_VERY_LOW_PRIORITY )
	{
	}
	else if( irqPriority == IRQ_LOW_PRIORITY )
	{
	}
	else if( irqPriority == IRQ_MEDIUM_PRIORITY )
	{
	}
	else if( irqPriority == IRQ_HIGH_PRIORITY )
	{
	}
	else if( irqPriority == IRQ_VERY_HIGH_PRIORITY )
	{
	}
	else
	{
		while( 1 );
	}
	*/
}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{
	int pin = obj->pin & 0x0F;

	if (obj->pin == NC)
		return;

	GpioIrq[pin] = NULL;
	GPIO_IntConfig((GPIO_Port_TypeDef)(obj->portIndex), pin, false, false, false);
}

void GpioMcuWrite( Gpio_t *obj, uint16_t value )
{
	if (obj == NULL || obj->portIndex == NULL || obj->pin == NC)
		return;

	if (value == 0)
		GPIO_PortOutClear((GPIO_Port_TypeDef)(obj->portIndex), obj->pinIndex);
	else
		GPIO_PortOutSet((GPIO_Port_TypeDef)(obj->portIndex), obj->pinIndex);
}

uint16_t GpioMcuRead( Gpio_t *obj )
{
	if (obj == NULL || obj->portIndex == NULL || obj->pin == NC)
		return 0;
	return (GPIO->P[obj->portIndex].DIN & obj->portIndex ? 1 : 0);
}

void EXTIx_IRQHandler(void)
{
	register uint32_t mask;
	register GpioIrqHandler ** handler;
	register uint32_t flags;

	RtcRecoverMcuStatus( );

	mask = 0x8000;
	handler = &GpioIrq[0];
	flags = GPIO_IntGetEnabled();

	do
	{
		if (flags & mask)
		{
			if (*handler != NULL)
				(*handler)();
			handler++;
		}
		mask >>= 1;
	} while (mask != 0);

	GPIO_IntClear(flags);
}

void GPIO_EVEN_IRQHandler(void)
{
	EXTIx_IRQHandler();
}

void GPIO_ODD_IRQHandler(void)
{
	EXTIx_IRQHandler();
}
