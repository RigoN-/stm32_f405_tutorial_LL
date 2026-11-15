#######################################################################
# ШАБЛОН MAKEFILE ДЛЯ МИКРОКОНТРОЛЛЕРОВ STM32 (CORTEX-M4)
# Поддерживает: VPATH, Разделение Папок для сборки, Автозависимости, прошивка ST-LINK, GDB, OpenOCD
#######################################################################

# Способ вызова:
# mingw32-make                  -> сборка проекта в режиме DEBUG (по умолчанию)
# mingw32-make BUILD_MODE=RELEASE -> сборка проекта в режиме RELEASE 
# mingw32-make BUILD_MODE=DEBUG  -> сборка проекта в режиме RELEASE 
# mingw32-make clean            -> полная очистка папки сборки
# mingw32-make openocd          -> запуск сервера отладки (Окно 1)
# mingw32-make debug            -> запуск интерактивного GDB отладчика (Окно 2)
# mingw32-make BUILD_MODE=RELEASE  clean  -> очистка папки сборки RELEASE 
# mingw32-make BUILD_MODE=DEBUG  clean  -> очистка папки сборки DEBUG  
# mingw32-make BUILD_MODE=RELEASE  flash  -> загрузка прошивки сборки RELEASE 
# mingw32-make BUILD_MODE=DEBUG  flash  -> загрузка прошивки сборки DEBUG 

#######################################################################
# 1. КОНФИГУРАЦИЯ СБОРКИ И ПРОЕКТА
#######################################################################
BUILD_MODE ?= DEBUG
NAME = 00_blank

ifeq ($(BUILD_MODE), RELEASE)
    OUTPATH = ./_build/release/
else
    OUTPATH = ./_build/debug/
endif	
PROJECT = $(OUTPATH)$(NAME)

# Параметры целевой платы
DEVICE = STM32F405xx
BOARD = WeAct-STM32F405RGT6
HEAP_SIZE = 0x400

#######################################################################
# 2. АППАРАТНАЯ АРХИТЕКТУРА (ARM CORTEX-M4 FPU)
#######################################################################
MCU            = -mcpu=cortex-m4
MCU_TUNE       = -mtune=cortex-m4
FPU            = -mfpu=fpv4-sp-d16
FPU_FLOAT_ABI  = -mfloat-abi=hard
MCU_THUMB_ABI  = -mthumb
BYTE_ORDER     = -mlittle-endian

MCUFLAGS = $(MCU) $(MCU_TUNE) $(FPU) $(FPU_FLOAT_ABI) $(MCU_THUMB_ABI)

#######################################################################
# 3. ДИРЕКТОРИИ И ПУТИ ПРОЕКТА
#######################################################################
CORE = ../../driver/stm32f4xx/core
PLIB = ../../driver/stm32f4xx/plib

PLIB_LL_INC = $(PLIB)/LL/inc
PLIB_LL_SRC = $(PLIB)/LL/src
#PLIB_LL_INC = $(PLIB)/HAL/inc
#PLIB_LL_SRC = $(PLIB)/HAL/src
LIBS = ../../extrn_lib

# 3.1 # Includes:
#INCLUDES_LIB_XPRINTF = -I$($(LIBS)/xprintf/inc/)
INCLUDES = -I$(CORE) -I$(PLIB_LL_INC)
#INCLUDES += $(INCLUDES_LIB_XPRINTF)


#######################################################################
# 4. НАСТРОЙКА ИНСТРУМЕНТАРИЯ (TOOLCHAIN) И УТИЛИТ WINDOWS
#######################################################################
PATH_GCC      = ../../../tools/gcc-arm-none-eabi_14_3/bin
PATH_ST_LINK  = ../../../tools\ST-Link\ST-LINK_Utility
PATH_OPENOCD  = ..\..\..\tools\OpenOCD_0_12\bin

PREFIX  = $(PATH_GCC)/arm-none-eabi
CC      = $(PREFIX)-gcc
AS      = $(PREFIX)-as
AR      = $(PREFIX)-ar
LD      = $(PREFIX)-gcc
NM 		= $(PREFIX)-nm
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
READELF = $(PREFIX)-readelf
SIZE    = $(PREFIX)-size
GDB     = $(PREFIX)-gdb

# Утилиты отладки и прошивки
GDB_INIT_FILE = $(subst /,\,$(OUTPATH)gdb_init.txt)
OPENOCD      = $(PATH_OPENOCD)\openocd.exe
OPENOCD_ARGS = -f interface\stlink.cfg -f target\stm32f4x.cfg
ST_LINK      = $(PATH_ST_LINK)/ST-LINK_CLI.exe

#######################################################################
# Command clean:
# Команды совместимости с Windows CMD
RM          = rmdir /s /q
MKDIR       = mkdir
# Для утилиты mkdir под Windows нужно использовать обратные слэши
OBJ_DIR_WIN = $(subst /,\,$(OUTPATH))

#######################################################################
# 5. СПИСОК ИСХОДНЫХ ФАЙЛОВ ПРОЕКТА
#######################################################################
# Sources:
# Ассемблерные файлы стартапа
SOURCES_CORE_ASM = $(CORE)/startup_stm32f405xx.s

# Системные файлы ядра (включая syscalls.c)
SOURCES_CORE = $(wildcard $(CORE)/*.c)

# Периферия (Рекомендуется ручное управление для HAL; для LL можно wildcard)
# Sources PERIPH:
#SOURCES_PERIPH = $(wildcard $(PLIB_LL_SRC)/*.c)
SOURCES_PERIPH = $(PLIB_LL_SRC)/stm32f4xx_ll_rcc.c
SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_gpio.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_utils.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_i2c.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_usart.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_tim.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_sdmmc.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_spi.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_adc.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_dma.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_dma2d.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_crc.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_dac.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_exti.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_fmc.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_fmpi2c.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_fsmc.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_lptim.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_pwr.c 
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_rng.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_rtc.c
#SOURCES_PERIPH += $(PLIB_LL_SRC)/stm32f4xx_ll_usb.c

##EXTERN LIBRARY
EXTERN_SOURCES_LIB =
#EXTERN_SOURCES_LIB = $(wildcard $(LIBS)/*.c)
#SOURCES_LIB_XPRINTF = $(LIBS)/xprintf/src/xprintf.c
#SOURCES_LIB += $(SOURCES_LIB_XPRINTF)

# Объединение всех исходных кодов Си
SOURCES_C = $(wildcard *.c)
SOURCES_C += $(SOURCES_CORE)
SOURCES_C += $(SOURCES_PERIPH)
SOURCES_C += $(EXTERN_SOURCES_LIB)

#######################################################################
# 6. УПРАВЛЕНИЕ ОБЪЕКТНЫМИ ФАЙЛАМИ И ВИРТУАЛЬНЫМИ ПУТЯМИ (VPATH)
#######################################################################
# Формируем изолированные пути к объектным файлам в папку сборки _build
OBJS = $(addprefix $(OUTPATH), $(notdir $(SOURCES_C:.c=.o)))
OBJS += $(addprefix $(OUTPATH), $(notdir $(SOURCES_CORE_ASM:.s=.o)))

# Автоматически наполняем VPATH уникальными путями ко всем исходникам (.c и .s)
VPATH = $(sort $(dir $(SOURCES_C) $(SOURCES_CORE_ASM)))

#######################################################################
# 7. ГЛОБАЛЬНЫЕ МАКРООПРЕДЕЛЕНИЯ (DEFINES)
#######################################################################
DEFINES += -D$(DEVICE) 
DEFINES += -DSTM32F4 -DSTM32F405 
DEFINES += -DUSE_FULL_LL_DRIVER
DEFINES += -DHSE_VALUE=8000000
DEFINES += -DVDD_VALUE=3300 
DEFINES += -DHEAP_SIZE=$(HEAP_SIZE)

#######################################################################
# 8. СТРАТЕГИИ КОМПИЛЯЦИИ И ЛИНКОВКИ (ФЛАГИ)
#######################################################################
CFLAGS_INFO = -Wall -Wextra --pedantic -Wno-unused-parameter 
CFLAGS_MMD  = -MMD -MP

ifeq ($(BUILD_MODE), RELEASE)
	# Настройки для RELEASE
    OPTIMIZE_FLAGS = -Os -flto -fdata-sections -ffunction-sections -fsingle-precision-constant -fno-math-errno -ffast-math -fno-common
    CFLAGS         = $(MCUFLAGS) $(OPTIMIZE_FLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS_INFO) $(CFLAGS_MMD)
    LDFLAGS_MODE   = -Wl,--relax $(OPTIMIZE_FLAGS)
else
	 # Настройки для DEBUG (идеально для STMStudio)
    DEBUG_FLAGS    = -O0 -g3 -gdwarf-2 -fdata-sections -ffunction-sections
    CFLAGS         = $(MCUFLAGS) $(DEBUG_FLAGS) -DDEBUG $(DEFINES) $(INCLUDES) $(CFLAGS_INFO) $(CFLAGS_MMD)
    LDFLAGS_MODE   = 
endif
#создается файл main.su
#CFLAGS_EXTRA += -fstack-usage
#######################################################################
# Linker options flsgs 
# Сборка флагов компоновщика
LDFLAGS_STUB    = --specs=nano.specs --specs=nosys.specs
#LDFLAGS_FLOAT += -u _printf_float
#LDFLAGS_FLOAT  += -u _scanf_float  # Раскомментировать для поддержки %f в printf
LD_SCRIPT       = -T $(CORE)/STM32_f405_linker_script.ld
LDFLAGS_MAP     = -Wl,-Map=$(PROJECT).map,--cref
LDFLAGS_GC_SECT = -Wl,--gc-sections 
LDFLAGS_INFO    = -Wl,--print-memory-usage

# Базовый каркас LDFLAGS (Спецификации библиотек идут ПЕРВЫМИ ради борьбы с варнингами)
LDFLAGS = -static $(MCUFLAGS) $(LDFLAGS_STUB) $(LDFLAGS_MODE) $(LD_SCRIPT) $(LDFLAGS_MAP) $(LDFLAGS_GC_SECT) $(LDFLAGS_INFO)

#######################################################################
# 9. ЦЕЛИ И ЗАДАЧИ СБОРКИ (RULES)
#######################################################################
.PHONY: all clean flash erase start openocd debug

# Главная цель сборки
all: $(OUTPATH) $(PROJECT).bin $(PROJECT).hex $(PROJECT).asm

# Полная очистка папки текущего режима сборки
clean:
	@if exist $(OBJ_DIR_WIN) $(RM) $(OBJ_DIR_WIN)

# Создание целевой директории
$(OUTPATH):
	@if not exist $(OBJ_DIR_WIN) $(MKDIR) $(OBJ_DIR_WIN)

# Правило компоновки бинарного файла (.elf)
$(PROJECT).elf: $(OBJS)
	@echo --------------------Linker_START-------------------------------
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	@echo --------------------Linker_COMPLETE----------------------------
	@echo --------------------Memory_FULL_START--------------------------
	$(SIZE) -A $@
	@echo --------------------Memory_FULL_END----------------------------
	@echo --------------------SECTION Memory_START-----------------------
	$(SIZE) -B $@
	@echo --------------------SECTION Memory_END-------------------------
	@echo    TABLE comments:
	@echo  - End size  FLASH = text + data
	@echo  - End size  RAM   = data + bss
	@echo ---------------------------------------------------------------

# Шаблонное правило для всех исходных файлов Си
$(OUTPATH)%.o: %.c
	@echo [CC] Компиляция файла: $<
	$(CC) $(CFLAGS) -c $< -o $@ 

# Шаблонное правило для файлов Ассемблера (Использует GCC с подавлением сишного шума флагом -w)
$(OUTPATH)%.o: %.s
	@echo [AS] Сборка файла: $<
	$(CC) $(CFLAGS) -w -c $< -o $@ 

# Генерация финальных прошивок и листингов
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	
%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.asm: %.elf
	"$(OBJDUMP).exe" -dhw $< > $@

#######################################################################
# 10. ИНТЕРФЕЙС УПРАВЛЕНИЯ ЗАГРУЗКИ И ОТЛАДКИ
#######################################################################
# Быстрая прошивка через программатор ST-Link утилитой CLI
flash:	
	"$(ST_LINK)" -P $(PROJECT).bin 0x08000000 -V "after_programming" -Rst
	
# Дистанционный аппаратный сброс и старт программы
start:	
	"$(ST_LINK)" -run 0x08000000

# ОКНО 1: Запуск фонового GDB-сервера OpenOCD через ST-Link V2
openocd:
	@echo [OPENOCD] Запуск сервера связи с ST-Link V2 (Порт GDB: 3333)...
	"$(OPENOCD)" $(OPENOCD_ARGS)

# ОКНО 2: Запуск интерактивного консольного отладчика GDB в графическом текстовом режиме (-tui)
debug: $(PROJECT).elf
	@echo target remote localhost:3333 > $(GDB_INIT_FILE)
	@echo monitor reset halt >> $(GDB_INIT_FILE)
	@echo load >> $(GDB_INIT_FILE)
	@echo monitor reset halt >> $(GDB_INIT_FILE)
	@echo [GDB] Запуск консольного отладчика в режиме TUI...
	"$(GDB)" -tui -x $(GDB_INIT_FILE) $(PROJECT).elf

#######################################################################
# 11. ПОДКЛЮЧЕНИЕ АВТОМАТИЧЕСКИХ ЗАВИСИМОСТЕЙ (.d файлов)
#######################################################################
-include $(OBJS:.o=.d)
