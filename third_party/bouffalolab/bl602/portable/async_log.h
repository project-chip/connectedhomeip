#ifndef __ASYNC_LOT_H_
#define __ASYNC_LOT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int async_log_push(const void * buf, const uint16_t len);
void log_async_init(void);
void enable_async_log(void);
int async_log(void);

#ifdef __cplusplus
}
#endif
#endif
