

#include <FreeRTOS.h>

#include <stdlib.h>

/****************************************************************************/

extern void mt793xLog(const char * aFormat, ...);

extern void mt793x_wpa_log_cb(void * ctx, int level, int type, const char * txt, size_t len);

/****************************************************************************/

void __wrap__wlan_printf(int skip, int level, const char * fmt, ...)
{
    va_list ap;

    if (skip)
        return;

    va_start(ap, fmt);

    mt793xLog(fmt, ap);

    va_end(ap);
}
