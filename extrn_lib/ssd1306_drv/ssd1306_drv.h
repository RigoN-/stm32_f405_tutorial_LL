#ifndef SSD1306_DRV_H
#define SSD1306_DRV_H

#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include <stdint.h>
#include <stddef.h>

// I2C адрес дисплея (обычно 0x3C, реже 0x3D)
#define OLED_I2C_ADDR         (0x3C << 1)

// Размеры экрана
#define OLED_WIDTH            128
#define OLED_HEIGHT           64
#define OLED_BUFFER_SIZE      (OLED_WIDTH * OLED_HEIGHT / 8)

// Цвета
typedef enum {
    OLED_COLOR_BLACK = 0x00, // Пиксель выключен
    OLED_COLOR_WHITE = 0x01  // Пиксель включен
} OLED_Color_t;

// Структура шрифта
typedef struct {
    const uint8_t width;      // Ширина символа в пикселях
    const uint8_t height;     // Высота символа в пикселях
    const uint16_t *data;     // Указатель на массив бит символов
} OLED_Font_t;

// Внешние ссылки на базовые шрифты
extern const OLED_Font_t Font_7x10;

// API Драйвера
uint8_t OLED_Init(I2C_TypeDef *I2Cx);
void OLED_Clear(OLED_Color_t color);
void OLED_UpdateScreen(I2C_TypeDef *I2Cx);
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_Color_t color);
void OLED_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_Color_t color);

// Текстовый вывод
void OLED_SetCursor(uint8_t x, uint8_t y);
char OLED_WriteChar(char ch, const OLED_Font_t *Font, OLED_Color_t color);
void OLED_WriteString(const char *str, const OLED_Font_t *Font, OLED_Color_t color);

#endif // SSD1306_DRV_H