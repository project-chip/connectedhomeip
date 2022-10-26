#ifndef SL_IOSTREAM_HANDLES_H
#define SL_IOSTREAM_HANDLES_H
#include "sl_iostream.h"
#include "sl_iostream_init_usart_instances.h"


#ifdef __cplusplus
extern "C" {
#endif

extern const sl_iostream_instance_info_t *sl_iostream_instances_info[];
extern const uint32_t sl_iostream_instances_count;

sl_iostream_t *sl_iostream_get_handle(char *name);

#ifdef __cplusplus
}
#endif

#endif // SL_IOSTREAM_HANDLES_H
