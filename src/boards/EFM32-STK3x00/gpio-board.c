#include "board.h"
#include "gpio-board.h"

GpioIrqHandler *GpioIrq[16];

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
	GPIO_Mode_TypeDef pinMode;

	obj->pin = pin;
	if( pin == NC ) 
		return;

	obj->portIndex = ( uint32_t ) pin >> 4;
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
#if 0
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	if( irqHandler == NULL )
	{
		return;
	}

	GpioIrq[obj->pin & 0x0F] = irqHandler;

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

	/* Connect EXTI Line to GPIO pin */
	SYSCFG_EXTILineConfig( obj->portIndex, ( ( obj->pin ) & 0x0F ) );

	/* Configure EXTI line */
	 EXTI_InitStructure.EXTI_Line = ( 0x01 << ( obj->pin & 0x0F ) );
	 EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

	if( irqMode == IRQ_RISING_EDGE )
	{
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
	}
	else if( irqMode == IRQ_FALLING_EDGE )
	{
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
	}
	else if( irqMode == IRQ_RISING_FALLING_EDGE )
	{
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; 
	}
	else
	{
		while( 1 );
	}
 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	if( ( ( obj->pin ) & 0x0F ) < 5 )
	{
		NVIC_InitStructure.NVIC_IRQChannel = ( ( obj->pin ) & 0x0F ) + 6;
	}
	else if( ( ( obj->pin ) & 0x0F ) < 10 )
	{
		NVIC_InitStructure.NVIC_IRQChannel = 23;
	}
	else if( ( ( obj->pin ) & 0x0F ) < 16 )
	{
		NVIC_InitStructure.NVIC_IRQChannel = 40;
	}
	else
	{
		while( 1 );
	}

	if( irqPriority == IRQ_VERY_LOW_PRIORITY )
	{
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	}
	else if( irqPriority == IRQ_LOW_PRIORITY )
	{
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 12;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	}
	else if( irqPriority == IRQ_MEDIUM_PRIORITY )
	{
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	}
	else if( irqPriority == IRQ_HIGH_PRIORITY )
	{
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	}
	else if( irqPriority == IRQ_VERY_HIGH_PRIORITY )
	{
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	}
	else
	{
		while( 1 );
	}
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
#endif
}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{
	/*
	EXTI_InitTypeDef EXTI_InitStructure;

	GpioIrq[obj->pin & 0x0F] = NULL;

	EXTI_InitStructure.EXTI_Line = ( 0x01 << ( obj->pin & 0x0F ) );
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
	*/
}

void GpioMcuWrite( Gpio_t *obj, uint32_t value )
{
	if (obj == NULL || obj->portIndex == NULL || obj->pin == NC)
		return;

	if ( value == 0 )
	{
		GPIO->P[obj->portIndex].DOUTCLR = obj->pinIndex;
	}
	else
	{
		GPIO->P[obj->portIndex].DOUTSET = obj->pinIndex;
	}
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
	if ( obj == NULL || obj->pin == NC )
		return 0;
	return (GPIO->P[obj->portIndex].DIN & obj->portIndex ? 1 : 0);
}

#if 0
void EXTIx_IRQHandler( uint6_t mask, int index )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( mask ) != RESET )
	{
		if( GpioIrq[index] != NULL )
		{
			GpioIrq[index]( ); 
		}
		EXTI_ClearITPendingBit( mask );
	}
}

void EXTI0_IRQHandler( void )
{
	EXTIx_IRQHandler( EXTI_Line0, 0 );
}

#endif
