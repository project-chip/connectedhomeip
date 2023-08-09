/*******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
// Includes from this component
#include "mpc_config.h"

// Unify Components
#include "config.h"
#include "sl_log.h"

// Generic includes
#include <stdlib.h>
#include <string.h>

// Interfaces
#include "uic_version.h"

#define LOG_TAG "mpc_config"

// List of default settings
#define CONFIG_KEY_INTERFACE "mpc.interface"
#define CONFIG_KEY_KVS_PATH "mpc.kvs"
#define CONFIG_KEY_VENDOR_ID "mpc.vendor"
#define CONFIG_KEY_PRODUCT_ID "mpc.product"
#define CONFIG_KEY_DISCRIMINATOR_ID "mpc.discriminator"
#define CONFIG_KEY_PIN_ID "mpc.pin"
#define CONFIG_KEY_STRICT_DEVICE_MAPPING "mpc.strict_device_mapping"
#define CONFIG_KEY_REPORT_MAX_INTERVAL "mpc.report_max"

#ifdef __APPLE__
#define DEFAULT_INTERFACE "en0"
#else
#define DEFAULT_INTERFACE "eth0"
#endif
#define DEFAULT_KVS_PATH "/var/lib/uic-mpc/chip_unify_mpc.kvs"
#define DEFAULT_STRICT_MAPPING false
#define DEFAULT_VENDOR_ID 0xFFF1
#define DEFAULT_PROD_ID 0x8011
#define DEFAULT_DISCRIM_ID 0xFFE
#define DEFAULT_RMAX 3600 // 60 mins

static mpc_config_t config;

int mpc_config_init()
{
    int32_t random_pin     = 0;
    config_status_t status = CONFIG_STATUS_OK;

    random_pin = rand() & 0xFFFFFF;
    status |= config_add_string(CONFIG_KEY_MPC_DATASTORE_FILE, "MPC datastore database file", DEFAULT_MPC_DATASTORE_FILE);
    status |= config_add_string(CONFIG_KEY_INTERFACE, "Ethernet interface to use", DEFAULT_INTERFACE);
    status |= config_add_string(CONFIG_KEY_KVS_PATH, "Matter key value store path", DEFAULT_KVS_PATH);
    status |= config_add_bool(CONFIG_KEY_STRICT_DEVICE_MAPPING, "Only map devices we are certain conforms to specification",
                              DEFAULT_STRICT_MAPPING);
    status |= config_add_int(CONFIG_KEY_VENDOR_ID, "16 bit Vendor ID", DEFAULT_VENDOR_ID);
    status |= config_add_int(CONFIG_KEY_PRODUCT_ID, "16 bit Product ID", DEFAULT_PROD_ID);
    status |= config_add_int(CONFIG_KEY_DISCRIMINATOR_ID, "12 bit Discriminator ID", DEFAULT_DISCRIM_ID);
    status |= config_add_int(CONFIG_KEY_PIN_ID, "24 bit pin", random_pin);
    status |= config_add_int(CONFIG_KEY_REPORT_MAX_INTERVAL, "ceiled max interval for reportables (in seconds)", DEFAULT_RMAX);

    return status != CONFIG_STATUS_OK;
}

static int config_get_int_safe(const char * key)
{
    int val = 0;
    if (SL_STATUS_OK != config_get_as_int(key, &val))
    {
        sl_log_error(LOG_TAG, "Failed to get int for key: %s", key);
    }
    return val;
}

sl_status_t mpc_config_fixt_setup()
{
    memset(&config, 0, sizeof(config));
    config_status_t status = CONFIG_STATUS_OK;
    // Datastore configuration
    status |= config_get_as_string(CONFIG_KEY_MPC_DATASTORE_FILE, &config.datastore_file);
    // MQTT configuration
    status |= config_get_as_string(CONFIG_KEY_MQTT_HOST, &config.mqtt_host);
    status |= config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &config.mqtt_cafile);
    status |= config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &config.mqtt_certfile);
    status |= config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &config.mqtt_keyfile);
    config.mqtt_port = config_get_int_safe(CONFIG_KEY_MQTT_PORT);

    status |= config_get_as_string(CONFIG_KEY_INTERFACE, &config.interface);
    status |= config_get_as_string(CONFIG_KEY_KVS_PATH, &config.kvs_path);
    status |= config_get_as_bool(CONFIG_KEY_STRICT_DEVICE_MAPPING, &config.strict_device_mapping);
    config.vendor_id     = config_get_int_safe(CONFIG_KEY_VENDOR_ID);
    config.product_id    = config_get_int_safe(CONFIG_KEY_PRODUCT_ID);
    config.discriminator = config_get_int_safe(CONFIG_KEY_DISCRIMINATOR_ID) & 0xFFFFFF;
    config.pin           = config_get_int_safe(CONFIG_KEY_PIN_ID) & 0xFFF;
    config.reportMax     = config_get_int_safe(CONFIG_KEY_REPORT_MAX_INTERVAL) & 0xFFFF;

    return status == CONFIG_STATUS_OK ? SL_STATUS_OK : SL_STATUS_FAIL;
}

const mpc_config_t * mpc_get_config()
{
    return &config;
}
