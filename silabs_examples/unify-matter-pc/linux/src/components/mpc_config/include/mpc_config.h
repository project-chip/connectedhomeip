/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup mpc_config MPC Configuration Extension
 * @ingroup mpc_components
 *
 * @brief Add the MPC-specific fixtures to the Unify \ref config system.
 *
 * This module is the source of all MPC-specific configuration parameters.
 * The source of the configuration parameters are command line arguments
 * and the configuration file.
 *
 * The after initialization the configurations parameters are constant.
 *
 * @{
 */

#ifndef MPC_CONFIG_H
#define MPC_CONFIG_H

// Unify includes
#include "uic_version.h"

#include <stdbool.h>
#include <stdint.h>

// Default setting for the mpc.datastore_file.
#define DEFAULT_MPC_DATASTORE_FILE "/var/lib/uic-mpc/mpc.db"

// Config key for the MPC datastore file
#define CONFIG_KEY_MPC_DATASTORE_FILE "mpc.datastore_file"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    /// Hostname of the MQTT broker
    const char * mqtt_host;
    /// Path to a file containing the PEM encoded trusted CA certificate files.
    const char * mqtt_cafile;
    /// Path to a file containing the PEM encoded certificate file for this client.
    const char * mqtt_certfile;
    /// Path to a file containing the PEM encoded unencrypted private key for this client.
    const char * mqtt_keyfile;
    /// Client ID for MQTT Client for TLS Authentication and encryption
    const char * mqtt_client_id;
    /// Pre shared Key for MQTT Client for TLS Authentication and encryption
    const char * mqtt_client_psk;
    /// Port of the MQTT broker
    int mqtt_port;
    /// File name for datastore/persistent storage
    const char * datastore_file;

    /// interface to be used to connect
    const char * interface;
    /// Path to a file containing key value store
    const char * kvs_path;
    /// Flag to restrict mapping devices only if we are certain it conforms to specification
    bool strict_device_mapping;
    /// 16 bit vendor ID
    uint16_t vendor_id;
    /// 16 bit product ID
    uint16_t product_id;
    /// 12 bit Discriminator ID to be used for commissioning
    uint16_t discriminator;
    /// 24 bit pin code to be used for commissioning
    uint32_t pin;
    /// 16 bit max interval expected between reportables
    uint16_t reportMax;
} mpc_config_t;

/**
 * @brief Get the current configuration. This must only be called after
 * mpc_config_init.
 */
const mpc_config_t * mpc_get_config();

/**
 * @brief Register MPC configurations in \ref config.
 *
 * This must be called before \ref uic_main.
 *
 * @returns 0 on success.
 */
int mpc_config_init();

#ifdef __cplusplus
}
#endif

/** @} end mpc_config */

#endif // MPC_CONFIG_H
