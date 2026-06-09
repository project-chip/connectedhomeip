/*
 * All linker intercepted functions were listed in this file.
 *
 * Some of them were implemented in newlib nad unable to replace easily,
 * some are skipped due to link order.
 */

/****************************************************************************/

#include <stdio.h>

extern int log_write(char * buf, int len);

int __wrap__write(int file, char * ptr, int len)
{
    return log_write(ptr, len);
}

/****************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <sys/stat.h>

int __wrap__gettimeofday(struct timeval * tv, void * ptz)
{
    int ticks = xTaskGetTickCount();
    if (tv != NULL)
    {
        tv->tv_sec  = (ticks / 1000);
        tv->tv_usec = (ticks % 1000) * 1000;
        return 0;
    }

    return -1;
}

/****************************************************************************/

#include <assert.h>
#include <stdio.h>

extern void platform_assert(const char * expr, const char * file, int line);

void __assert_func(const char * file, int line, const char * func, const char * expr)
{
    fflush(NULL);
    platform_assert(expr, file, line);
    while (true)
        ;
}

/****************************************************************************/

#include <FreeRTOS.h>

#include <assert.h>
#include <stdlib.h>

void * __wrap__calloc_r(size_t nmemb, size_t size)
{
    void * p = pvPortCalloc(nmemb, size);
    while (!p)
        ;
    return p;
}

void * __wrap__malloc_r(void * REENT, size_t size)
{
    void * p = pvPortMalloc(size);
    while (!p)
        ;
    return p;
}

void __wrap__free_r(void * REENT, void * ptr)
{
    return vPortFree(ptr);
}

void * __wrap__realloc_r(void * REENT, void * ptr, size_t size)
{
    void * p = pvPortRealloc(ptr, size);
    while (!p)
        ;
    return p;
}

/****************************************************************************/

#include <FreeRTOS.h>
#include <stdlib.h>

extern void mt793xLog(const char * aFormat, ...);

extern void mt793x_wpa_log_cb(void * ctx, int level, int type, const char * txt, size_t len);

void __wrap__wlan_printf(int skip, int level, const char * fmt, ...)
{
    va_list ap;

    if (skip)
        return;

    va_start(ap, fmt);

    mt793xLog(fmt, ap);

    va_end(ap);
}

/****************************************************************************/

extern int __io_getchar(void) __attribute__((weak));
int _read(int file, char * ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = __io_getchar();
    }

    return len;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat * st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char * name, int flags, int mode)
{
    return -1;
}

int _getpid(void)
{
    errno = ENOTSUP;
    return -1;
}

int _kill(int pid, int sig)
{
    errno = ENOTSUP;
    return -1;
}