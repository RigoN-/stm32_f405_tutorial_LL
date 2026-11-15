#include "ssd1306_drv.h"
#include "debug.h"  // Интеграция с нашим логером LOG_E / ASSERT

// Локальный буфер экрана в RAM STM32F405
static uint8_t OLED_Buffer[OLED_BUFFER_SIZE];

// Текущие координаты курсора для текста
static struct {
    uint8_t x;
    uint8_t y;
} OLED_Cursor;

// Низкоуровневая отправка команды через базовый драйвер LL
static void OLED_WriteCommand(I2C_TypeDef *I2Cx, uint8_t byte) {
    // Генерация START
    LL_I2C_GenerateStartCondition(I2Cx);
    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    // Отправка адреса устройства (Write)
    LL_I2C_TransmitData8(I2Cx, OLED_I2C_ADDR);
    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
    LL_I2C_ClearFlag_ADDR(I2Cx);

    // Контрольный байт: 0x00 означает, что далее следует КОМАНДА
    while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    LL_I2C_TransmitData8(I2Cx, 0x00);

    // Сама команда
    while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    LL_I2C_TransmitData8(I2Cx, byte);

    // Ожидание окончания передачи и генерация STOP
    while(!LL_I2C_IsActiveFlag_BTF(I2Cx));
    LL_I2C_GenerateStopCondition(I2Cx);
}

// Инициализация контроллера SSD1306
uint8_t OLED_Init(I2C_TypeDef *I2Cx) {
    // Простейшая задержка перед стартом для стабилизации питания матрицы
    for(volatile uint32_t i = 0; i < 500000; i++);

    LOG_I("OLED: Starting initialization sequential...");

    // Последовательность команд инициализации для режима 128x64
    OLED_WriteCommand(I2Cx, 0xAE); // Выключить дисплей (Sleep Mode)
    OLED_WriteCommand(I2Cx, 0x20); // Настройка режима адресации памяти
    OLED_WriteCommand(I2Cx, 0x00); // 0x00 = Horizontal Addressing Mode
    OLED_WriteCommand(I2Cx, 0x21); // Установка адресов колонок
    OLED_WriteCommand(I2Cx, 0x00); // Старт = 0
    OLED_WriteCommand(I2Cx, 127);  // Конец = 127
    OLED_WriteCommand(I2Cx, 0x22); // Установка адресов страниц (Pages)
    OLED_WriteCommand(I2Cx, 0x00); // Старт = 0
    OLED_WriteCommand(I2Cx, 7);    // Конец = 7
    OLED_WriteCommand(I2Cx, 0x40); // Установить стартовую линию строки = 0
    OLED_WriteCommand(I2Cx, 0x81); // Настройка контрастности
    OLED_WriteCommand(I2Cx, 0xCF); // Значение яркости (0x00 - 0xFF)
    OLED_WriteCommand(I2Cx, 0xA1); // Переворот по горизонтали (Segment Re-map)
    OLED_WriteCommand(I2Cx, 0xC8); // Переворот по вертикали (COM Output Scan Direction)
    OLED_WriteCommand(I2Cx, 0xA6); // Нормальный режим отображения (не инверсный)
    OLED_WriteCommand(I2Cx, 0xA8); // Мультиплексирование
    OLED_WriteCommand(I2Cx, 0x3F); // 1/64 Duty
    OLED_WriteCommand(I2Cx, 0xD3); // Сдвиг дисплея (Display Offset)
    OLED_WriteCommand(I2Cx, 0x00); // Без сдвига
    OLED_WriteCommand(I2Cx, 0xD5); // Частота генератора (Display Clock Divide Ratio)
    OLED_WriteCommand(I2Cx, 0x80); // Рекомендуемое значение
    OLED_WriteCommand(I2Cx, 0xD9); // Период предзаряда (Pre-charge Period)
    OLED_WriteCommand(I2Cx, 0xF1); 
    OLED_WriteCommand(I2Cx, 0xDA); // Конфигурация аппаратных пинов COM
    OLED_WriteCommand(I2Cx, 0x12);
    OLED_WriteCommand(I2Cx, 0xDB); // Уровень VCOMH
    OLED_WriteCommand(I2Cx, 0x40);
    OLED_WriteCommand(I2Cx, 0x8D); // Управление встроенным насосом накачки напряжения
    OLED_WriteCommand(I2Cx, 0x14); // Включить Charge Pump (Обязательно для OLED 0.96)
    OLED_WriteCommand(I2Cx, 0xAF); // Включить дисплей (Display ON)

    OLED_Clear(OLED_COLOR_BLACK);
    OLED_UpdateScreen(I2Cx);
    
    LOG_I("OLED: Initialized successfully");
    return 1;
}

// Очистка локального буфера кадра
void OLED_Clear(OLED_Color_t color) {
    uint8_t fill = (color == OLED_COLOR_WHITE) ? 0xFF : 0x00;
    for(size_t i = 0; i < OLED_BUFFER_SIZE; i++) {
        OLED_Buffer[i] = fill;
    }
}

// Сброс буфера кадра из RAM контроллера в чип SSD1306 по I2C
void OLED_UpdateScreen(I2C_TypeDef *I2Cx) {
    LL_I2C_GenerateStartCondition(I2Cx);
    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    LL_I2C_TransmitData8(I2Cx, OLED_I2C_ADDR);
    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
    LL_I2C_ClearFlag_ADDR(I2Cx);

    // Контрольный байт: 0x40 означает, что далее идет поток ВИДЕОДАННЫХ
    while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    LL_I2C_TransmitData8(I2Cx, 0x40);

    // Последовательная выталкивание 1024 байт буфера
    for (size_t i = 0; i < OLED_BUFFER_SIZE; i++) {
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
        LL_I2C_TransmitData8	(I2Cx, OLED_Buffer[i]);
    }

    while(!LL_I2C_IsActiveFlag_BTF(I2Cx));
    LL_I2C_GenerateStopCondition(I2Cx);
}

// Точечный графический вывод (низкоуровневый рендеринг в RAM)
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_Color_t color) {
    if(x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        return; // Защита от выхода за границы экрана
    }

    // Вычисляем байт и конкретный бит в вертикальной структуре страниц SSD1306
    if (color == OLED_COLOR_WHITE) {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
    } else {
        OLED_Buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
    }
}

// Установка текстового курсора
void OLED_SetCursor(uint8_t x, uint8_t y) {
    OLED_Cursor.x = x;
    OLED_Cursor.y = y;
}

// Рендеринг одного символа шрифта в буфер кадра
char OLED_WriteChar(char ch, const OLED_Font_t *Font, OLED_Color_t color) {
    // Проверка поддержки кодовой таблицы (символы ASCII от 32 до 126)
    if (ch < 32 || ch > 126) return 0;

    // Проверка, помещается ли символ по ширине экрана
    if (OLED_WIDTH < (OLED_Cursor.x + Font->width) ||
        OLED_HEIGHT < (OLED_Cursor.y + Font->height)) {
        return 0;
    }

    // Побитовый перенос матрицы шрифта в FrameBuffer
    for(uint8_t i = 0; i < Font->height; i++) {
        // Извлекаем строку битовой маски символа
        uint16_t b = Font->data[(ch - 32) * Font->height + i];
        for(uint8_t j = 0; j < Font->width; j++) {
            if((b << j) & 0x8000) { // Проверка старшего бита
                OLED_DrawPixel(OLED_Cursor.x + j, OLED_Cursor.y + i, color);
            } else {
                OLED_DrawPixel(OLED_Cursor.x + j, OLED_Cursor.y + i, !color);
            }
        }
    }

    // Сдвигаем курсор вправо на ширину напечатанного символа
    OLED_Cursor.x += Font->width;
    return ch;
}

// Печать строки
void OLED_WriteString(const char *str, const OLED_Font_t *Font, OLED_Color_t color) {
    while (*str) {
        if (OLED_WriteChar(*str, Font, color) == 0) {
            break; // Ошибка вывода или конец строки экрана
        }
        str++;
    }
}

// Алгоритм Брезенхема для рисования прямых линий
void OLED_Line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_Color_t color) {
    int8_t dx = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    int8_t dy = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    int8_t sx = (x1 < x2) ? 1 : -1;
    int8_t sy = (y1 < y2) ? 1 : -1;
    int8_t err = dx - dy;

    while (1) {
        OLED_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}