#ifndef I2C_DRV_H
#define I2C_DRV_H

#include "stm32f4xx_ll_i2c.h"
#include <stdint.h>
#include <stdbool.h>

/* Коды возврата I2C драйвера */
typedef enum {
    I2C_OK       = 0x00,
    I2C_ERROR_SB = 0x01, /* Ошибка флага START (SB) */
    I2C_ERROR_ADDR= 0x02, /* Ошибка передачи адреса (ADDR) / Устройство не ответило */
    I2C_ERROR_TXE  = 0x03, /* Ошибка опустошения буфера передачи (TXE) */
    I2C_ERROR_RXNE = 0x04, /* Ошибка заполнения буфера приема (RXNE) */
    I2C_ERROR_BTF  = 0x05, /* Ошибка окончания передачи байта (BTF) */
    I2C_ERROR_BUSY = 0x06  /* Шина заблокирована изначально */
} i2c_status_t;

/* Скоростные режимы */
#define I2C_SPEED_STANDARD 100000 /* 100 кГц */
#define I2C_SPEED_FAST     400000 /* 400 кГц */

/* Прототипы функций */
void i2c1_init(uint32_t clock_speed);
i2c_status_t i2c1_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t size);
i2c_status_t i2c1_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t size);
i2c_status_t i2c1_scan(void);
void i2c1_scaner(void);

#endif /* I2C_DRV_H */