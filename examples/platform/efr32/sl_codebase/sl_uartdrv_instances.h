#ifndef SL_UARTDRV_INSTANCES_H
#define SL_UARTDRV_INSTANCES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include "uartdrv.h"

extern UARTDRV_Handle_t sl_uartdrv_usart_vcom_handle;

void sl_uartdrv_init_instances(void);

/***************************************************************************//**
 * Set the handle as the default UARTDRV handle.
 *
 * @param[in] handle    UARTDRV handle to set as default.
 *
 * @return  Status result
 ******************************************************************************/
sl_status_t sl_uartdrv_set_default(UARTDRV_Handle_t handle);

/***************************************************************************//**
 * Get the default UARTDRV handle configured.
 *
 * @return  UARTDRV handle
 ******************************************************************************/
UARTDRV_Handle_t sl_uartdrv_get_default(void);

#ifdef __cplusplus
}
#endif

#endif // SL_UARTDRV_INSTANCES_H
