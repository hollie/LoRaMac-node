#include "board.h"
#include "gpio-board.h"
#include "stm32l1xx_gpio.h"

GpioIrqHandler *GpioIrq[16];

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if( pin == NC )
		return;
	
	obj->portIndex = ( uint32_t ) pin >> 4;

	obj->pin = pin;
	obj->pinIndex = ( 0x01 << ( obj->pin & 0x0F ) );

	if( obj->portIndex < 6 )
	{
		obj->port = ( GPIO_TypeDef * )( GPIOA_BASE + ( obj->portIndex << 10 ) );
		RCC_AHBPeriphClockCmd( ( 0x01 << obj->portIndex ), ENABLE );
	}
	else if( obj->portIndex == 6 )
	{   /* GPIO base address not in alphabetical order after GPIOE (cf stm32l1xx.h, line 926 ) */
		/* Access to GPIOF and GPIOG not implemented */
		obj->port = ( GPIO_TypeDef * )( GPIOH_BASE );
		RCC_AHBPeriphClockCmd( ( 0x01 << ( obj->portIndex - 1 ) ), ENABLE );
	}

	// Sets initial output value
	if( mode == PIN_OUTPUT )
	{
		GpioMcuWrite( obj, value );
	}

	GPIO_InitStructure.GPIO_Mode = ( GPIOMode_TypeDef )mode;
	GPIO_InitStructure.GPIO_OType = ( GPIOOType_TypeDef )config;
	GPIO_InitStructure.GPIO_PuPd = ( GPIOPuPd_TypeDef )type;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Pin = obj->pinIndex;
	GPIO_Init( obj->port, &GPIO_InitStructure );
}

void GpioMcuSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	if( irqHandler == NULL )
		return;

	GpioIrq[obj->pin & 0x0F] = irqHandler;

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

	/* Connect EXTI Line to GPIO pin */
	SYSCFG_EXTILineConfig( obj->portIndex, ( ( obj->pin ) & 0x0F ) );

	/* Configure EXTI line */
	EXTI_InitStructure.EXTI_Line = ( 0x01 << ( obj->pin & 0x0F ) );
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;

	if( irqMode == IRQ_RISING_EDGE )
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	else if( irqMode == IRQ_FALLING_EDGE )
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	else if( irqMode == IRQ_RISING_FALLING_EDGE )
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	else
		while( 1 );

	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	if( ( ( obj->pin ) & 0x0F ) < 5 )
		NVIC_InitStructure.NVIC_IRQChannel = ( ( obj->pin ) & 0x0F ) + 6;
	else if( ( ( obj->pin ) & 0x0F ) < 10 )
		NVIC_InitStructure.NVIC_IRQChannel = 23;
	else if( ( ( obj->pin ) & 0x0F ) < 16 )
		NVIC_InitStructure.NVIC_IRQChannel = 40;
	else
		while( 1 );

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
		while( 1 );

	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{
	EXTI_InitTypeDef EXTI_InitStructure;

	GpioIrq[obj->pin & 0x0F] = NULL;

	EXTI_InitStructure.EXTI_Line = ( 0x01 << ( obj->pin & 0x0F ) );
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void GpioMcuWrite( Gpio_t *obj, uint32_t value )
{
	if( ( obj == NULL ) || ( obj->port == NULL ) )
		while( 1 );
	// Check if pin is not connected
	if( obj->pin == NC )
		return;
	if( value == 0 )
		GPIO_ResetBits( obj->port, obj->pinIndex );
	else
		GPIO_SetBits( obj->port, obj->pinIndex );
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
	if( obj == NULL )
		while( 1 );
	// Check if pin is not connected
	if( obj->pin == NC )
		return 0;
	return GPIO_ReadInputDataBit( obj->port, obj->pinIndex );
}

void EXTIx_Handler(int idx)
{
	uint32_t irq = (1 << idx);
	if( EXTI_GetITStatus( irq ) != RESET )
	{
		if( GpioIrq[idx] != NULL )
			GpioIrq[idx]( );
		EXTI_ClearITPendingBit( irq );
	}
}

void EXTI0_IRQHandler( void )
{
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	EXTIx_Handler(0);
}

void EXTI1_IRQHandler( void )
{
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	EXTIx_Handler(1);
}

void EXTI2_IRQHandler( void )
{
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	EXTIx_Handler(2);
}

void EXTI3_IRQHandler( void )
{
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	EXTIx_Handler(3);
}

void EXTI4_IRQHandler( void )
{
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	EXTIx_Handler(4);
}

void EXTI9_5_IRQHandler( void )
{
	int idx;
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	for (idx = 5; idx <= 9; idx++)
		EXTIx_Handler(idx);
}

void EXTI15_10_IRQHandler( void )
{
	int idx;
#ifdef LOW_POWER_MODE_ENABLE
	RtcRecoverMcuStatus( );
#endif
	for (idx = 10; idx <= 15; idx++)
		EXTIx_Handler(idx);
}
