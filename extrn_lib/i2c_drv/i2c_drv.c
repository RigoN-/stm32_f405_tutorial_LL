#include "i2c_drv.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "debug.h"

/* Тайм-аут по умолчанию для ожидания флагов (в миллисекундах) */
#define I2C_TIMEOUT_MS  10

/* Импортируем наш системный тикер времени из main.c */
extern volatile uint32_t sys_count_ms;

/**
 * @brief Инициализация шины I2C1 (PB6 - SCL, PB7 - SDA)
 */
void i2c1_init(uint32_t clock_speed) {
	LL_RCC_ClocksTypeDef rcc_clocks;
    /* 1. Включаем тактирование GPIO и периферии I2C1 */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    /* 2. Конфигурируем выводы PB6 (SCL) и PB7 (SDA) */
    /* Режим: Альтернативная функция */
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);

    /* Тип выхода: Открытый дренаж (Open-Drain) — КРИТИЧЕСКИ для I2C! */
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7, LL_GPIO_OUTPUT_OPENDRAIN);

    /* Скорость: Высокая (High Speed) */
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH);

    /* Подтяжка: Внутренние подтяжки отключаем (No Pull), упор на внешние резисторы платы WeAct */
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_6, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_7, LL_GPIO_PULL_NO);

    /* Назначаем альтернативную функцию AF4 (I2C1..3) */
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_4);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_4);

    /* 3. Конфигурация параметров I2C */
    LL_I2C_Disable(I2C1);
    
    LL_RCC_GetSystemClocksFreq(&rcc_clocks);
    /* Автоматический расчет таймингов исходя из частоты APB1 (42МГц) и целевой скорости */
    //LL_I2C_ConfigSpeed(I2C1, 42000000, clock_speed, LL_I2C_DUTYCYCLE_2);
	//режим LL_I2C_DUTYCYCLE_2 является оптимальным. Переходить на LL_I2C_DUTYCYCLE_16_9 имеет смысл только при сильных аппаратных помехах или длинных соединительных проводах к датчикам.
	LL_I2C_ConfigSpeed(I2C1, rcc_clocks.PCLK1_Frequency, clock_speed, LL_I2C_DUTYCYCLE_2);
    /* Включаем периферию обратно */
    LL_I2C_Enable(I2C1);
    
    /* Разрешаем генерацию подтверждения (ACK) по умолчанию */
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    
    LOG_I("Интерфейс I2C1 успешно запущен на частоте %lu Гц", clock_speed);
}

/**
 * @brief Безопасная запись данных в регистр ведомого устройства
 */
i2c_status_t i2c1_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t size) {
    uint32_t start_tick;

    /* Проверяем, свободна ли шина */
    start_tick = sys_count_ms;
    while (LL_I2C_IsActiveFlag_BUSY(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_BUSY;
    }

    /* 1. Генерируем START */
    LL_I2C_GenerateStartCondition(I2C1);
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_SB;
    }

    /* 2. Отправляем 7-битный адрес устройства + бит записи (0) */
    LL_I2C_TransmitData8(I2C1, dev_addr << 1);
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_ADDR;
    }
    LL_I2C_ClearFlag_ADDR(I2C1); /* Сброс флага чтения регистра статуса */

    /* 3. Передаем адрес внутреннего регистра устройства */
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_TXE;
    }
    LL_I2C_TransmitData8(I2C1, reg_addr);

    /* 4. Цикл передачи основного массива данных */
    for (uint16_t i = 0; i < size; i++) {
        start_tick = sys_count_ms;
        while (!LL_I2C_IsActiveFlag_TXE(I2C1)) {
            if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_TXE;
        }
        LL_I2C_TransmitData8(I2C1, p_data[i]);
    }

    /* Ожидаем завершения последней физической передачи на линию (BTF) */
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_BTF(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_BTF;
    }

    /* 5. Генерируем STOP */
    LL_I2C_GenerateStopCondition(I2C1);
    return I2C_OK;
}

/**
 * @brief Безопасное чтение данных из регистра ведомого устройства (Режим повторного СТАРТа)
 */
i2c_status_t i2c1_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t size) {
    uint32_t start_tick;

    /* Проверка занятости шины */
    start_tick = sys_count_ms;
    while (LL_I2C_IsActiveFlag_BUSY(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_BUSY;
    }

    /* ФАЗА ЗАПРОСА: Пишем адрес регистра, который хотим прочесть */
    LL_I2C_GenerateStartCondition(I2C1);
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_SB;
    }

    LL_I2C_TransmitData8(I2C1, dev_addr << 1);
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_ADDR;
    }
    LL_I2C_ClearFlag_ADDR(I2C1);

    while (!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1, reg_addr);
    while (!LL_I2C_IsActiveFlag_BTF(I2C1));

    /* ФАЗА ЧТЕНИЯ: Генерируем повторный START (Re-Start) */
    LL_I2C_GenerateStartCondition(I2C1);
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_SB;
    }

    /* Передаем адрес со знаком ЧТЕНИЯ (бит 0 = 1) */
    LL_I2C_TransmitData8(I2C1, (dev_addr << 1) | 0x01);
    start_tick = sys_count_ms;
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
        if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_ADDR;
    }

    if (size == 1) {
        /* Если считываем всего 1 байт — согласно RM, мы должны сбросить ACK до очистки флага ADDR */
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
        LL_I2C_ClearFlag_ADDR(I2C1);
        LL_I2C_GenerateStopCondition(I2C1); /* Сразу взводим стоп */
        
        start_tick = sys_count_ms;
        while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) {
            if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_RXNE;
        }
        p_data[0] = LL_I2C_ReceiveData8(I2C1);
    } 
    else {
        /* Многобайтовое чтение */
        LL_I2C_ClearFlag_ADDR(I2C1);
        for (uint16_t i = 0; i < size; i++) {
            if (i == (size - 1)) {
                /* На последнем байте шлем NACK и генерируем STOP */
                LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
                LL_I2C_GenerateStopCondition(I2C1);
            }
            
            start_tick = sys_count_ms;
            while (!LL_I2C_IsActiveFlag_RXNE(I2C1)) {
                if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) return I2C_ERROR_RXNE;
            }
            p_data[i] = LL_I2C_ReceiveData8(I2C1);
        }
    }

    /* Возвращаем шину в состояние генерации ACK по умолчанию */
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    return I2C_OK;
}

/**
 * @brief Функция сканирования шины I2C на наличие устройств
 */
i2c_status_t i2c1_scan(void) {
    uint32_t start_tick;
    uint8_t devices_found = 0;
    
    printf("\r\n--- СКАНИРОВАНИЕ ШИНЫ I2C1 (0x01 - 0x7F) ---\r\n");

    for (uint8_t addr = 1; addr < 128; addr++) {
        /* Генерируем старт */
        LL_I2C_GenerateStartCondition(I2C1);
        start_tick = sys_count_ms;
        while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
            if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) {
                printf("Сбой шины при сканировании.\r\n");
                return I2C_ERROR_SB;
            }
        }

        /* Передаем адрес на запись */
        LL_I2C_TransmitData8(I2C1, addr << 1);
        
        /* Ждем ответа (ADDR флаг) или таймаута (если устройства нет — флага не будет) */
        start_tick = sys_count_ms;
        bool ack_received = true;
        while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
            /* Проверяем аппаратный флаг ошибки подтверждения (AF) */
            if (LL_I2C_IsActiveFlag_AF(I2C1)) {
                LL_I2C_ClearFlag_AF(I2C1);
                ack_received = false;
                break;
            }
            if ((sys_count_ms - start_tick) > 2) { // 2 мс на устройство более чем достаточно
                ack_received = false;
                break;
            }
        }

        if (ack_received) {
            LL_I2C_ClearFlag_ADDR(I2C1);
            printf("Найдено устройство по адресу: " ANSI_CLR_GREEN "0x%02X" ANSI_CLR_RESET "\r\n", addr);
            devices_found++;
        }

        /* Гарантированный стоп в конце шага */
        LL_I2C_GenerateStopCondition(I2C1);
        
        /* Короткая пауза для стабилизации линий */
        uint32_t wait = sys_count_ms;
        while((sys_count_ms - wait) < 1);
    }

    printf("Сканирование завершено. Найдено устройств: %d\r\n\r\n", devices_found);
    return I2C_OK;
}

/**
 * @brief Вывод карты устройств I2C в стиле утилиты i2cdetect
 */
void i2c1_scaner(void) {
    uint32_t start_tick;
    uint8_t found_list[128];
    uint8_t found_count = 0;

    printf("\r\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");

    for (uint8_t row = 0; row < 128; row += 16) {
        printf("%02x: ", row);

        for (uint8_t col = 0; col < 16; col++) {
            uint8_t addr = row + col;

            /* Адреса 0x00-0x02 и 0x78-0x7F зарезервированы протоколом, их не сканируем */
            if (addr < 0x03 || addr > 0x77) {
                printf("   ");
                continue;
            }

            /* Проверка шины на BUSY перед стартом */
            start_tick = sys_count_ms;
            while (LL_I2C_IsActiveFlag_BUSY(I2C1)) {
                if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) {
                    printf("BSY");
                    goto row_end;
                }
            }

            /* Генерация START */
            LL_I2C_GenerateStartCondition(I2C1);
            start_tick = sys_count_ms;
            while (!LL_I2C_IsActiveFlag_SB(I2C1)) {
                if ((sys_count_ms - start_tick) > I2C_TIMEOUT_MS) {
                    printf("err");
                    goto row_end;
                }
            }

            /* Передача адреса на запись */
            LL_I2C_TransmitData8(I2C1, addr << 1);
            
            start_tick = sys_count_ms;
            bool ack_received = true;
            while (!LL_I2C_IsActiveFlag_ADDR(I2C1)) {
                if (LL_I2C_IsActiveFlag_AF(I2C1)) {
                    LL_I2C_ClearFlag_AF(I2C1);
                    ack_received = false;
                    break;
                }
                if ((sys_count_ms - start_tick) > 2) {
                    ack_received = false;
                    break;
                }
            }

            if (ack_received) {
                LL_I2C_ClearFlag_ADDR(I2C1);
                /* Подсвечиваем найденный адрес зеленым цветом ANSI */
                printf(ANSI_CLR_GREEN "%02x " ANSI_CLR_RESET, addr);
                found_list[found_count++] = addr;
            } else {
                printf("-- ");
            }

            /* Обязательный STOP в конце каждого шага */
            LL_I2C_GenerateStopCondition(I2C1);
            
            /* Короткая пауза 1 мс между опросами */
            uint32_t wait = sys_count_ms;
            while((sys_count_ms - wait) < 1);
        }
        printf("\r\n");
    }

row_end:
    /* Вывод финального списка адресов */
    printf("\r\n>>> Найдено устройств: %d", found_count);
    if (found_count > 0) {
        printf(" (Адреса: ");
        for(uint8_t i = 0; i < found_count; i++) {
            printf(ANSI_CLR_GREEN "0x%02X" ANSI_CLR_RESET "%s", found_list[i], (i == found_count - 1) ? "" : ", ");
        }
        printf(")");
    }
    printf("\r\n--------------------------------------------------\r\n");
}