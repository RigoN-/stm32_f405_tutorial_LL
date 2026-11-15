#ifndef LED4_DRV_H
#define LED4_DRV_H

#include <stdint.h>
#include <stdbool.h>

/* Адреса внутренних регистров модуля остаются прежними */
#define REG_LED4_BRIGHTNESS     0x09
#define REG_LED4_DEG            0x12 

/* Прототипы функций драйвера с поддержкой динамического адреса */
bool led4_init(uint8_t dev_addr, uint8_t brightness);
void led4_set_brightness(uint8_t dev_addr, uint8_t volume);
void led4_print_time(uint8_t dev_addr, uint8_t minutes, uint8_t seconds, bool show_colon);
void led4_clear(uint8_t dev_addr);

#endif /* LED4_DRV_H */