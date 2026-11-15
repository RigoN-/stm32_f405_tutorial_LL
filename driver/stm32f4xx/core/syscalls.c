#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

// Подключаем LL-драйвер для работы с UART
#include "stm32f4xx_ll_usart.h"


#undef errno
extern int errno;

// Защита линковки: гарантируем Си-связывание для всех системных вызовов Newlib
#ifdef __cplusplus
extern "C" {
#endif

/* 
 * 1. Функция вывода (используется для printf)
 */
int _write(int file, char *ptr, int len) {
    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        for (int i = 0; i < len; i++) {
            while (!LL_USART_IsActiveFlag_TXE(USART1)) {}
            LL_USART_TransmitData8(USART1, (uint8_t)ptr[i]);
        }
        return len;
    }
    errno = EBADF;
    return -1;
}

int _read(int file, char *ptr, int len) {
    if (file == STDIN_FILENO) {
        for (int i = 0; i < len; i++) {
            while (!LL_USART_IsActiveFlag_RXNE(USART1)) {}
            ptr[i] = (char)LL_USART_ReceiveData8(USART1);
        }
        return len;
    }
    errno = EBADF;
    return -1;
}

/* 
 * 3. Заглушка изменения указателя в файле (убирает исходное предупреждение)
 */
__attribute__((weak))  off_t _lseek(int file, off_t ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0; // Имитируем успешное смещение
}

/* 
 * 4. Минимальная заглушка получения информации о файле
 */
__attribute__((weak))  int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR; // Говорим библиотеке Си, что это символьное устройство (терминал)
    return 0;
}

/* 
 * 5. Минимальная заглушка проверки, является ли устройство терминалом
 */
__attribute__((weak))  int _isatty(int file) {
    if (file == STDOUT_FILENO || file == STDERR_FILENO || file == STDIN_FILENO) {
        return 1;
    }
    errno = EBADF;
    return 0;
}

/* 
 * 6. Минимальная заглушка для закрытия файла
 */
__attribute__((weak))  int _close(int file) {
    (void)file;
    return -1; // Возвращаем -1, так как реальные файлы мы закрывать не умеем
}

/* 
 * 7. Управление выделением динамической памяти (кучей) для printf/malloc.
 * Необходима для работы libc_a-sbrkr.o
 */
__attribute__((weak)) void* _sbrk(ptrdiff_t incr) {
    // Внешние переменные, определяющие границы кучи (задаются линкер-скриптом)
    extern char _end; // Начало кучи (сразу после секции BSS)
   // extern char _estack; // Конец стека (верхняя граница RAM)
    extern uint32_t _Min_Stack_Size; // Минимальный резерв под стек (из линкер-скрипта)
    
    static char *heap_end;
    char *prev_heap_end;
    char *stack_ptr;

    // Считываем текущий указатель стека процессора (MSP)
    __asm volatile ("MRS %0, msp" : "=r" (stack_ptr));

    if (heap_end == 0) {
        heap_end = &_end;
    }

    prev_heap_end = heap_end;

    // Проверяем, чтобы куча при выделении памяти не врезалась в стек
    if (heap_end + incr > stack_ptr - (uint32_t)&_Min_Stack_Size) {
        // Если память закончилась, возвращаем ошибку ENOMEM
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}

/* 
 * 8. Заглушка аварийного завершения программы (нужна для abort.o)
 */
void _exit(int status) {
    (void)status;
    __disable_irq();
    while (1) {
        // Зависаем в бесконечном пустом цикле при критическом сбое библиотеки Си
    }
}

/* 
 * 9. Заглушка отправки сигналов процессам (нужна для signalr.o)
 */
int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL; // Возвращаем ошибку "Недопустимый аргумент"
    return -1;
}

/* 
 * 10. Заглушка получения ID текущего процесса (нужна для signalr.o)
 */
int _getpid(void) {
    return 1; // Всегда возвращаем фиктивный ID процесса равный 1
}

// Стандартные алиасы POSIX-функций к их системным реализациям под Newlib
int kill(int pid, int sig) __attribute__((alias("_kill")));
int getpid(void) __attribute__((alias("_getpid")));

#ifdef __cplusplus
}
#endif
