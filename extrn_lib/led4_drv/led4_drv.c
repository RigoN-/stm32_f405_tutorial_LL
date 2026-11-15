#include "led4_drv.h"
#include "i2c_drv.h"
#include "debug.h"

/* Таблица кодирования цифр 0..9 */
static const uint8_t led4_font[] = {
    0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6
};

/**
 * @brief Инициализация индикатора по указанному I2C адресу
 */
bool led4_init(uint8_t dev_addr, uint8_t brightness) {
    uint8_t dummy = 0;
    
    /* Проверяем физический ответ устройства по переданному адресу */
    if (i2c1_write(dev_addr, REG_LED4_BRIGHTNESS, &dummy, 1) != I2C_OK) {
        LOG_E("4LED индикатор не ответил по адресу 0x%02X!", dev_addr);
        return false;
    }

    led4_set_brightness(dev_addr, brightness);
    led4_clear(dev_addr);
    LOG_I("4LED индикатор успешно запущен по адресу 0x%02X", dev_addr);
    return true;
}

/**
 * @brief Установка яркости дисплея (уровень от 0 до 7) для конкретного адреса
 */
void led4_set_brightness(uint8_t dev_addr, uint8_t volume) {
    if (volume > 7) volume = 7;
    
    uint8_t raw_brightness = 0;
    switch(volume) {
        case 0: raw_brightness = 0 << 3;  break;
        case 1: raw_brightness = 1 << 3;  break;
        case 2: raw_brightness = 2 << 3;  break;
        case 3: raw_brightness = 4 << 3;  break;
        case 4: raw_brightness = 8 << 3;  break;
        case 5: raw_brightness = 14 << 3; break;
        case 6: raw_brightness = 22 << 3; break;
        case 7: raw_brightness = 31 << 3; break;
    }

    i2c1_write(dev_addr, REG_LED4_BRIGHTNESS, &raw_brightness, 1);
}

/**
 * @brief Вывод времени в формате ММ:СС на указанный дисплей
 */
void led4_print_time(uint8_t dev_addr, uint8_t minutes, uint8_t seconds, bool show_colon) {
    uint8_t tx_buf[5];

    if (minutes > 99) minutes = 99;
    if (seconds > 59) seconds = 59;

    tx_buf[0] = led4_font[minutes / 10];
    tx_buf[1] = led4_font[minutes % 10];
    tx_buf[2] = led4_font[seconds / 10];
    tx_buf[3] = led4_font[seconds % 10];
    tx_buf[4] = show_colon ? 0x03 : 0x00;

    /* Пишем пачку по целевому адресу dev_addr */
    i2c1_write(dev_addr, REG_LED4_DEG, tx_buf, 5);
}

/**
 * @brief Очистка экрана конкретного дисплея
 */
void led4_clear(uint8_t dev_addr) {
    uint8_t tx_buf[5] = {0, 0, 0, 0, 0};
    i2c1_write(dev_addr, REG_LED4_DEG, tx_buf, 5);
}