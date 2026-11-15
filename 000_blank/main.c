#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"

// Добавлен для настройки питания ядра
//#include "stm32f4xx_ll_pwr.h" 

/**
  *   System Clock Configuration
  *   The system Clock is configured as follow :
  *   System Clock source            = PLL (HSE)
  *   SYSCLK(Hz)                     = 168000000
  *   HCLK(Hz)                       = 168000000
  *   AHB Prescaler                  = 1
  *   APB1 Prescaler                 = 4
  *   APB2 Prescaler                 = 2
  *   HSE Frequency(Hz)              = 8000000
  *   PLL_M                          = 8
  *   PLL_N                          = 336
  *   PLL_P                          = 2
  *   VDD(V)                         = 3.3
  *   Main regulator output voltage  = Scale1 mode
  *   Flash Latency(WS)              = 5
  */
 /**
 * @brief  Настройка тактирования микроконтроллера на 168 МГц от HSE (8 МГц)
 */ 
static void rcc_config() {
/* 1. Включаем тактирование модуля управления питанием (PWR) */
        //LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
        
        /* КРИТИЧЕСКИЙ ИСПРАВЛЕННЫЙ ШАГ: Включаем Scale 1 Mode для работы на 168 МГц.
         * Без этого ядро микроконтроллера будет испытывать "голодание" по питанию */
        //LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
		
        /* Enable HSE oscillator */
		 /* 2. Включаем внешний кварцевый резонатор (HSE) */
		//SystemCoreClockUpdate();
        LL_RCC_HSE_Enable();
        while (LL_RCC_HSE_IsReady() == 0);

        /* Set FLASH latency */
		/* 3. Настраиваем задержку Flash памяти (5 ворлд-стейтов для 168 МГц) */
        LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
		//SystemCoreClockUpdate();
        /* Main PLL configuration and activation */
		/* 4. Конфигурация основного умножителя PLL: 
         * Вход HSE (8МГц) / M(8) * N(336) / P(2) = 168 МГц */
        LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8,
                                    336, LL_RCC_PLLP_DIV_2);
        LL_RCC_PLL_Enable();
        while (LL_RCC_PLL_IsReady() == 0);

		/* 5. Настраиваем делители шин периферии:
         * AHB  = SYSCLK / 1 = 168 МГц
         * APB1 = HCLK / 4   = 42 МГц  (Максимум для APB1 таймеров - 84 МГц)
         * APB2 = HCLK / 2   = 84 МГц  (Максимум для APB2 таймеров - 168 МГц) */
        /* Sysclk activation on the main PLL */
        LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
		 /* Set APB1 & APB2 prescaler */
        LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
        LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
		
		/* 6. Переключаем источник системного тактирования на выход PLL */
        LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
		
		while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);
	   //while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

       /* ИСПРАВЛЕНО: Вместо ручного хардкода частоты вызываем стандартную 
         * функцию CMSIS. Она сама рассчитает частоту на основе регистров RCC 
         * и запишет её в глобальную переменную SystemCoreClock */

        //SystemCoreClock = 168000000;
		//SystemCoreClockUpdate();
		SystemCoreClockUpdate();
        SysTick_Config(SystemCoreClock/1000);

        /* Update CMSIS variable (which can be updated also
         * through SystemCoreClockUpdate function) */
      
}

static void gpio_config(void) {
		// Включаем тактирование порта B		
}

void SysTick_Handler(void) {
	
}

int main(void) {
		// Инициализируем систему тактирования
        rcc_config();
		 // Инициализируем ножку светодиода
        gpio_config();
		
        while (1) {
			
		}
		return 0;
}
