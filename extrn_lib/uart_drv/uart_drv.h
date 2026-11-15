#ifndef UART_DRV_H
#define UART_DRV_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"


// Ваша исходная конфигурация буфера
#define RX_BUF_SIZE 64
#define RX_RING_MASK (RX_BUF_SIZE - 1)

// Делаем структуру буфера видимой для обработчика прерываний и функций
typedef struct {
    volatile uint8_t buffer[RX_BUF_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
} RingBuffer_t;

extern volatile RingBuffer_t rx_ring;

// Интерфейс модуля
void usart1_config(uint32_t baudrate);
void USART1_IRQHandler(void);
uint8_t uart_read_byte(void);
void uart_parse_commands(void);
#endif // UART_DRV_H