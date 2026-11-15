#include "uart_drv.h"
#include "debug.h" 

// Выделяем память под ваш кольцевой буфер
volatile RingBuffer_t rx_ring = {0};

/**
  * @brief  Инициализация аппаратного модуля USART1 на пинах PA9 (TX) и PA10 (RX)
  * @param  baudrate: желаемая скорость передачи (например, 115200)
  */
void usart1_config(uint32_t baudrate) {
        /* 1. Включаем тактирование порта GPIOA и самого модуля USART1 */
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

        /* 2. Конфигурируем пин PA9 как USART1_TX (Alternate Function 7) */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_7);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);

        /* 3. Конфигурируем пин PA10 как USART1_RX (Alternate Function 7) */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_7);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_VERY_HIGH);
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_UP);

        /* 4. Настройка параметров кадра передачи */
        LL_USART_Disable(USART1); // Отключаем модуль перед конфигурированием

        LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);
        LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
        LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
        LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
        LL_USART_SetHWFlowCtrl(USART1, LL_USART_HWCONTROL_NONE);
        LL_USART_SetOverSampling(USART1, LL_USART_OVERSAMPLING_16);

        /* 5. Автоматический расчет битрейта. 
         * USART1 висит на быстрой шине APB2. Так как мы вызвали SystemCoreClockUpdate(), 
         * частота шины APB2 гарантированно равна SystemCoreClock / 2 (то есть 84 МГц) */
        uint32_t apb2_frequency = SystemCoreClock / 2; 
        LL_USART_SetBaudRate(USART1, apb2_frequency, LL_USART_OVERSAMPLING_16, baudrate);
		
		 /*  6. Включаем прерывание на аппаратном уровне модуля USART1 по флагу приема байта */
		LL_USART_EnableIT_RXNE(USART1);
    
		/* 7. Разрешаем обработку прерывания USART1 в глобальном контроллере NVIC микроконтроллера */
		NVIC_SetPriority(USART1_IRQn, 6); // Выставляем средний приоритет
		NVIC_EnableIRQ(USART1_IRQn);

        /* 8. Финальное включение модуля */
        LL_USART_Enable(USART1);
		// Отключаем буферизацию Си для мгновенного вывода в UART
		setvbuf(stdout, NULL, _IONBF, 0);
}

/**
  * @brief  Обработчик прерывания USART1. Вызывается аппаратно, когда прилетает байт.
  */
void USART1_IRQHandler(void) {
    /* Проверяем, что прерывание вызвано именно флагом RXNE (приемный регистр не пуст)*/
    if (LL_USART_IsActiveFlag_RXNE(USART1) && LL_USART_IsEnabledIT_RXNE(USART1))
    {
        uint8_t byte = LL_USART_ReceiveData8(USART1);
        
       /* Вычисляем следующий индекс для головы буфера*/
        uint16_t next_head = (rx_ring.head + 1) & (RX_BUF_SIZE - 1);
        
        /* Если буфер не переполнен, сохраняем байт*/
        if (next_head != rx_ring.tail) {
            rx_ring.buffer[rx_ring.head] = byte;
            rx_ring.head = next_head;
        }
    }
}

/**
  * @brief  Функция безопасного чтения одного байта из кольцевого буфера
  * @return Полученный байт или 0, если буфер пуст
  */
uint8_t uart_read_byte(void) {
    if (rx_ring.head == rx_ring.tail) {
        return 0; // Буфер пуст
    }
    
    uint8_t byte = rx_ring.buffer[rx_ring.tail];
    rx_ring.tail = (rx_ring.tail + 1) & (RX_BUF_SIZE - 1);
    return byte;
}

/**
  * @brief  Парсер текстовых команд, приходящих от ПК
  */
void uart_parse_commands(void) {
    static char cmd_line[32];
    static uint8_t idx = 0;
    
    uint8_t ch = uart_read_byte();
    
    if (ch != 0) 
    {
        // Если прилетел символ окончания строки (Putty шлет \r или \n при нажатии Enter)
        if (ch == '\r' || ch == '\n') 
        {
            if (idx > 0) 
            {
                cmd_line[idx] = '\0'; // Закрываем строку Си-нулём
                
                // Сравниваем пришедшую строку с нашими командами
                if (strcmp(cmd_line, "led_on") == 0) {
				    if (LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_2)==0) {
						LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2); 
						LOG_I("Получена команда: LED ВКЛЮЧЕН вручную");
					}
                } 
                else if (strcmp(cmd_line, "led_off") == 0) {
					if (LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_2)==1) {
						LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_2);
						LOG_I("Получена команда: LED ВЫКЛЮЧЕН вручную");
					}                 
                    
                } 
                else {
                    LOG_E("Неизвестная команда: '%s'", cmd_line);
                }
                
                idx = 0; // Сбрасываем индекс для следующей команды
            }
        } 
        else 
        {
            // Накапливаем символы в буфер строки, защищаясь от переполнения
            if (idx < (sizeof(cmd_line) - 1)) {
                cmd_line[idx++] = (char)ch;
            }
        }
    }
}