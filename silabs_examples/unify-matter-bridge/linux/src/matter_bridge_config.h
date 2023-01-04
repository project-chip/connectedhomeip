/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file matter_bridge_config.h
 * @addtogroup matter_bridge_config Configuration Extension
 * @ingroup components
 *
 * @brief Add the Matter Bridge-specific fixtures to the Unify \ref config system.
 *
 * @{
 */

#if !defined(MATTER_BRIDGE_CONFIG_H)
#define MATTER_BRIDGE_CONFIG_H

#include <stdint.h>

#define CONFIG_KEY_BLE_DEVICE_ID "matter_bridge.ble-device"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char * interface;
    const char * kvs_path;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t discriminator;
    uint32_t pin;
} matter_bridge_config_t;

/**
 * @brief Get the current configuration. This must only be called after
 * matter_bridge_config_init.
 */
const matter_bridge_config_t * matter_bridge_get_config();

/**
 * @brief Register Matter Bridge configurations in \ref config.
 *
 * This must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int matter_bridge_config_init();

#ifdef __cplusplus
}
#endif

/** @} end matter_bridge_config */

#endif // MATTER_BRIDGE_CONFIG_H
