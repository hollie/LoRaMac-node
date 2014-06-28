/**
  ******************************************************************************
  * @file    system_stm32l1xx.c
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    27-August-2013
  *=============================================================================
  *                         System Clock Configuration
  *=============================================================================
  *        System Clock source          | PLL(HSI)
  *----------------------------------------------------------------------------- 
  *        SYSCLK                       | 32000000 Hz
  *----------------------------------------------------------------------------- 
  *        HCLK                         | 32000000 Hz
  *----------------------------------------------------------------------------- 
  *        AHB Prescaler                | 1
  *----------------------------------------------------------------------------- 
  *        APB1 Prescaler               | 1
  *----------------------------------------------------------------------------- 
  *        APB2 Prescaler               | 1
  *----------------------------------------------------------------------------- 
  *        HSE Frequency                | 12000000 Hz
  *----------------------------------------------------------------------------- 
  *        PLL DIV                      | 3
  *----------------------------------------------------------------------------- 
  *        PLL MUL                      | 8
  *----------------------------------------------------------------------------- 
  *        VDD                          | 3.3 V
  *----------------------------------------------------------------------------- 
  *        Vcore                        | 1.8 V (Range 1)
  *----------------------------------------------------------------------------- 
  *        Flash Latency                | 1 WS
  *----------------------------------------------------------------------------- 
  *        Require 48MHz for USB clock  | Enabled
  *----------------------------------------------------------------------------- 
  *=============================================================================
  ******************************************************************************
  */

#include "stm32l1xx.h"

/*!< Uncomment the following line if you need to relocate your vector Table in Internal SRAM. */ 
/* #define VECT_TAB_SRAM */
/*!< Vector Table base offset field. This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET			0x0
uint32_t SystemCoreClock		= 32000000UL;
const uint8_t PLLMulTable[9]	= {3, 4, 6, 8, 12, 16, 24, 32, 48};
const uint8_t AHBPrescTable[16]	= {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

void SetSysClock(void);

/**
  * @brief  Setup the microcontroller system.
  *         Initialize the Embedded Flash Interface, the PLL and update the 
  *         SystemCoreClock variable.
  * @param  None
  * @retval None
  */
void SystemInit (void)
{
	RCC->CR   |= RCC_CR_MSION;
	RCC->CFGR &= ~(RCC_CFGR_MCOPRE | RCC_CFGR_MCOSEL | RCC_CFGR_PPRE2 | RCC_CFGR_PPRE1 | RCC_CFGR_HPRE | RCC_CFGR_SW);
	RCC->CR   &= ~(RCC_CR_CSSON | RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_HSION);
	RCC->CR   &= ~RCC_CR_HSEBYP;
	RCC->CFGR &= ~(RCC_CFGR_PLLDIV | RCC_CFGR_PLLMUL | RCC_CFGR_PLLSRC);
	RCC->CIR = 0;	// Disable all interrupts

	SetSysClock();

#ifdef VECT_TAB_SRAM
	SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM. */
#else
	SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
#endif
}

/**
  * @brief  Update SystemCoreClock according to Clock Register Values
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *           
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.         
  *     
  * @note   - The system frequency computed by this function is not the real 
  *           frequency in the chip. It is calculated based on the predefined 
  *           constant and the selected clock source:
  *             
  *           - If SYSCLK source is MSI, SystemCoreClock will contain the MSI 
  *             value as defined by the MSI range.
  *                                   
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(*)
  *                                              
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(**)
  *                          
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(**) 
  *             or HSI_VALUE(*) multiplied/divided by the PLL factors.
  *         
  *         (*) HSI_VALUE is a constant defined in stm32l1xx.h file (default value
  *             16 MHz) but the real value may vary depending on the variations
  *             in voltage and temperature.   
  *    
  *         (**) HSE_VALUE is a constant defined in stm32l1xx.h file (default value
  *              8 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *                
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.  
  * @param  None
  * @retval None
  */
void SystemCoreClockUpdate (void)
{
	/* Get SYSCLK source -------------------------------------------------------*/
	switch (RCC->CFGR & RCC_CFGR_SWS)
	{
		case RCC_CFGR_SWS_HSI:  // HSI used as system clock
			SystemCoreClock = HSI_VALUE;
			break;
		case RCC_CFGR_SWS_HSE:  // HSE used as system clock
			SystemCoreClock = HSE_VALUE;
			break;
		case RCC_CFGR_SWS_PLL:  // PLL used as system clock
			// Get PLL clock source and multiplication factor
			SystemCoreClock =
				((RCC->CFGR & RCC_CFGR_PLLSRC) == RCC_CFGR_PLLSRC_HSI)
				? HSI_VALUE
				: HSE_VALUE;
			SystemCoreClock *= PLLMulTable[((RCC->CFGR & RCC_CFGR_PLLMUL) >> 18)];
			SystemCoreClock /= (((RCC->CFGR & RCC_CFGR_PLLDIV) >> 22) + 1);
			break;
		default: /* MSI used as system clock */
			SystemCoreClock = (32768 * (1 << (((RCC->ICSCR & RCC_ICSCR_MSIRANGE) >> 13) + 1)));
			break;
	}
	/* Compute HCLK clock frequency --------------------------------------------*/
	SystemCoreClock >>= AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
}

/**
  * @brief  Configures the System clock frequency, AHB/APBx prescalers and Flash 
  *         settings.
  * @note   This function should be called only once the RCC clock configuration  
  *         is reset to the default reset state (done in SystemInit() function).             
  * @param  None
  * @retval None
  */
void SetSysClock(void)
{
	__IO uint32_t StartUpCounter = 0, Status;
  
	RCC->CR |= RCC_CR_HSION;
	while (((RCC->CR & RCC_CR_HSIRDY) != RESET) && (StartUpCounter++ != HSI_STARTUP_TIMEOUT))
	{ }

	if ((RCC->CR & RCC_CR_HSIRDY) != RESET)
	{
		FLASH->ACR |= FLASH_ACR_ACC64;		/* Enable 64-bit access */
		FLASH->ACR |= FLASH_ACR_PRFTEN;		/* Enable Prefetch Buffer */
		FLASH->ACR |= FLASH_ACR_LATENCY;	/* Flash 1 wait state */
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;	/* Power enable */
		PWR->CR = PWR_CR_VOS_0;				/* Select the Voltage Range 1 (1.8 V) */
		while((PWR->CSR & PWR_CSR_VOSF) != RESET)
		{ }									/* Wait Until the Voltage Regulator is ready */

		RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;	/* HCLK = SYSCLK /1*/
		RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;	/* PCLK2 = HCLK /1*/
		RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;	/* PCLK1 = HCLK /1*/
		/*  PLL configuration */
		RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
		RCC->CFGR |= (RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL6 | RCC_CFGR_PLLDIV3);
		RCC->CR |= RCC_CR_PLLON;				/* Enable PLL */
		while((RCC->CR & RCC_CR_PLLRDY) == 0)	/* Wait till PLL is ready */
		{ }

		/* Select PLL as system clock source */
		RCC->CFGR &= ~(RCC_CFGR_SW);
		RCC->CFGR |= RCC_CFGR_SW_PLL;

		/* Wait till PLL is used as system clock source */
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		{ }
	}
	else
	{
		/* If HSE fails to start-up, the application will have wrong clock
       configuration. User can add here some code to deal with this error */
	}
}
