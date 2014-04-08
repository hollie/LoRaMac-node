#include "board.h"
#include "gpio-board.h"

static GpioIrqHandler *GpioIrq[16];

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
	GPIO_Mode_TypeDef pinMode;

	if( pin == NC ) 
		return;

	obj->portIndex = ( uint32_t ) pin >> 4;
	obj->pin = pin;
	obj->pinIndex = (obj->pin & 0x0F);

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
		obj->pinIndex,
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
		
		GPIO_PinOutClear((GPIO_Port_TypeDef)(obj->portIndex), obj->pinIndex);
	}
	else
	{
		GPIO_PinOutSet((GPIO_Port_TypeDef)(obj->portIndex), obj->pinIndex);
	}
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
	if ( obj == NULL || obj->pin == NC )
		return 0;

	return GPIO_PinInGet((GPIO_Port_TypeDef)(obj->portIndex), obj->pinIndex);
}

#if 0
void EXTI0_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line0 ) != RESET )
	{
		if( GpioIrq[0] != NULL )
		{
			GpioIrq[0]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line0 );
	}
}

void EXTI1_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line1 ) != RESET )
	{
		if( GpioIrq[1] != NULL )
		{
			GpioIrq[1]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line1 );
	}
}

void EXTI2_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line2 ) != RESET )
	{
		if( GpioIrq[2] != NULL )
		{
			GpioIrq[2]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line2 );
	}
}

void EXTI3_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line3 ) != RESET )
	{
		if( GpioIrq[3] != NULL )
		{
			GpioIrq[3]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line3 );
	}
}

void EXTI4_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line4 ) != RESET )
	{
		if( GpioIrq[4] != NULL )
		{
			GpioIrq[4]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line4 );
	}
}

void EXTI9_5_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line5 ) != RESET )
	{
		if( GpioIrq[5] != NULL )
		{
			GpioIrq[5]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line5 );
	}

	if( EXTI_GetITStatus( EXTI_Line6 ) != RESET )
	{   
		if( GpioIrq[6] != NULL )
		{
			GpioIrq[6]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line6 );
	}

	if( EXTI_GetITStatus( EXTI_Line7 ) != RESET )
	{
		if( GpioIrq[7] != NULL )
		{
			GpioIrq[7]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line7 );
	}

	if( EXTI_GetITStatus( EXTI_Line8 ) != RESET )
	{
		if( GpioIrq[8] != NULL )
		{
			GpioIrq[8]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line8 );
	}

	if( EXTI_GetITStatus( EXTI_Line9 ) != RESET )
	{   
		if( GpioIrq[9] != NULL )
		{
			GpioIrq[9]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line9 );
	}
}

void EXTI15_10_IRQHandler( void )
{
	RtcRecoverMcuStatus( );
	if( EXTI_GetITStatus( EXTI_Line10 ) != RESET )
	{	
		if( GpioIrq[10] != NULL )
		{
			GpioIrq[10]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line10 );
	}

	if( EXTI_GetITStatus( EXTI_Line11 ) != RESET )
	{	
		if( GpioIrq[11] != NULL )
		{
			GpioIrq[11]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line11 );
	}

	if( EXTI_GetITStatus( EXTI_Line12 ) != RESET )
	{	
		if( GpioIrq[12] != NULL )
		{
			GpioIrq[12]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line12 );
	}

	if( EXTI_GetITStatus( EXTI_Line13 ) != RESET )
	{	
		if( GpioIrq[13] != NULL )
		{
			GpioIrq[13]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line13 );
	}

	if( EXTI_GetITStatus( EXTI_Line14 ) != RESET )
	{	
		if( GpioIrq[14] != NULL )
		{
			GpioIrq[14]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line14 );
	}

	if( EXTI_GetITStatus( EXTI_Line15 ) != RESET )
	{	
		if( GpioIrq[15] != NULL )
		{
			GpioIrq[15]( ); 
		}
		EXTI_ClearITPendingBit( EXTI_Line15 );
	}
}
#endif
