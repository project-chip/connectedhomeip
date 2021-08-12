/*******************************************************************************
 * (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * ("Software"), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software ("EULA").
 *
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress's integrated circuit products.
 * Any reproduction, modification, translation, compilation, or representation
 * of this Software except as specified above is prohibited without the express
 * written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 *******************************************************************************/

/*******************************************************************************
 * File Name: cycfg_bt_settings.c
 * Version: 2.40.0.4650
 *
 * Description:
 *   Runtime Bluetooth stack configuration parameters.
 *
 *******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "cycfg_bt_settings.h"
#include "cycfg_gap.h"

#include "wiced_bt_ble.h"
#include "wiced_bt_cfg.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"

#if !defined(WICED_BTSTACK_VERSION_MAJOR)
#define WICED_BTSTACK_VERSION_MAJOR (2)
#endif

/*****************************************************************************
 * wiced_bt core stack configuration
 ****************************************************************************/
#if (WICED_BTSTACK_VERSION_MAJOR >= 3)
/* BLE scan settings  */
const wiced_bt_cfg_ble_scan_settings_t cy_bt_cfg_scan_settings = {
    .scan_mode =
        CY_BT_SCAN_MODE, /* BLE scan mode (BTM_BLE_SCAN_MODE_PASSIVE, BTM_BLE_SCAN_MODE_ACTIVE, or BTM_BLE_SCAN_MODE_NONE) */

    /* Advertisement scan configuration */
    .high_duty_scan_interval = CY_BT_HIGH_DUTY_SCAN_INTERVAL, /* High duty scan interval (in slots (1 slot = 0.625 ms)) */
    .high_duty_scan_window   = CY_BT_HIGH_DUTY_SCAN_WINDOW,   /* High duty scan window (in slots (1 slot = 0.625 ms)) */
    .high_duty_scan_duration = CY_BT_HIGH_DUTY_SCAN_DURATION, /* High duty scan duration in seconds (0 for infinite) */

    .low_duty_scan_interval = CY_BT_LOW_DUTY_SCAN_INTERVAL, /* Low duty scan interval (in slots (1 slot = 0.625 ms)) */
    .low_duty_scan_window   = CY_BT_LOW_DUTY_SCAN_WINDOW,   /* Low duty scan window (in slots (1 slot = 0.625 ms)) */
    .low_duty_scan_duration = CY_BT_LOW_DUTY_SCAN_DURATION, /* Low duty scan duration in seconds (0 for infinite) */

    /* Connection scan configuration */
    .high_duty_conn_scan_interval =
        CY_BT_HIGH_DUTY_CONN_SCAN_INTERVAL, /* High duty cycle connection scan interval (in slots (1 slot = 0.625 ms)) */
    .high_duty_conn_scan_window =
        CY_BT_HIGH_DUTY_CONN_SCAN_WINDOW, /* High duty cycle connection scan window (in slots (1 slot = 0.625 ms)) */
    .high_duty_conn_duration =
        CY_BT_HIGH_DUTY_CONN_SCAN_DURATION, /* High duty cycle connection duration in seconds (0 for infinite) */

    .low_duty_conn_scan_interval =
        CY_BT_LOW_DUTY_CONN_SCAN_INTERVAL, /* Low duty cycle connection scan interval (in slots (1 slot = 0.625 ms)) */
    .low_duty_conn_scan_window =
        CY_BT_LOW_DUTY_CONN_SCAN_WINDOW, /* Low duty cycle connection scan window (in slots (1 slot = 0.625 ms)) */
    .low_duty_conn_duration =
        CY_BT_LOW_DUTY_CONN_SCAN_DURATION, /* Low duty cycle connection duration in seconds (0 for infinite) */

    /* Connection configuration */
    .conn_min_interval        = CY_BT_CONN_MIN_INTERVAL,        /* Minimum connection interval (in slots (1 slot = 1.25 ms)) */
    .conn_max_interval        = CY_BT_CONN_MAX_INTERVAL,        /* Maximum connection interval (in slots (1 slot = 1.25 ms)) */
    .conn_latency             = CY_BT_CONN_LATENCY,             /* Connection latency */
    .conn_supervision_timeout = CY_BT_CONN_SUPERVISION_TIMEOUT, /* Connection link supervision timeout (in 10 ms) */
};

/* BLE advertisement settings */
const wiced_bt_cfg_ble_advert_settings_t cy_bt_cfg_adv_settings = {
    .channel_map = CY_BT_CHANNEL_MAP, /* Advertising channel map (mask of BTM_BLE_ADVERT_CHNL_37, BTM_BLE_ADVERT_CHNL_38,
                                         BTM_BLE_ADVERT_CHNL_39) */

    .high_duty_min_interval = CY_BT_HIGH_DUTY_ADV_MIN_INTERVAL, /* High duty undirected connectable minimum advertising interval (in
                                                                   slots (1 slot = 0.625 ms)) */
    .high_duty_max_interval = CY_BT_HIGH_DUTY_ADV_MAX_INTERVAL, /* High duty undirected connectable maximum advertising interval (in
                                                                   slots (1 slot = 0.625 ms)) */
    .high_duty_duration =
        CY_BT_HIGH_DUTY_ADV_DURATION, /* High duty undirected connectable advertising duration in seconds (0 for infinite) */

    .low_duty_min_interval = CY_BT_LOW_DUTY_ADV_MIN_INTERVAL, /* Low duty undirected connectable minimum advertising interval (in
                                                                 slots (1 slot = 0.625 ms)) */
    .low_duty_max_interval = CY_BT_LOW_DUTY_ADV_MAX_INTERVAL, /* Low duty undirected connectable maximum advertising interval (in
                                                                 slots (1 slot = 0.625 ms)) */
    .low_duty_duration =
        CY_BT_LOW_DUTY_ADV_DURATION, /* Low duty undirected connectable advertising duration in seconds (0 for infinite) */

    .high_duty_directed_min_interval =
        CY_BT_HIGH_DUTY_DIRECTED_ADV_MIN_INTERVAL, /* High duty directed connectable minimum advertising interval (in slots (1 slot
                                                      = 0.625 ms)) */
    .high_duty_directed_max_interval =
        CY_BT_HIGH_DUTY_DIRECTED_ADV_MAX_INTERVAL, /* High duty directed connectable maximum advertising interval (in slots (1 slot
                                                      = 0.625 ms)) */

    .low_duty_directed_min_interval = CY_BT_LOW_DUTY_DIRECTED_ADV_MIN_INTERVAL, /* Low duty directed connectable minimum advertising
                                                                                   interval (in slots (1 slot = 0.625 ms)) */
    .low_duty_directed_max_interval = CY_BT_LOW_DUTY_DIRECTED_ADV_MAX_INTERVAL, /* Low duty directed connectable maximum advertising
                                                                                   interval (in slots (1 slot = 0.625 ms)) */
    .low_duty_directed_duration =
        CY_BT_LOW_DUTY_DIRECTED_ADV_DURATION, /* Low duty directed connectable advertising duration in seconds (0 for infinite) */

    .high_duty_nonconn_min_interval = CY_BT_HIGH_DUTY_NONCONN_ADV_MIN_INTERVAL, /* High duty non-connectable minimum advertising
                                                                                   interval (in slots (1 slot = 0.625 ms)) */
    .high_duty_nonconn_max_interval = CY_BT_HIGH_DUTY_NONCONN_ADV_MAX_INTERVAL, /* High duty non-connectable maximum advertising
                                                                                   interval (in slots (1 slot = 0.625 ms)) */
    .high_duty_nonconn_duration =
        CY_BT_HIGH_DUTY_NONCONN_ADV_DURATION, /* High duty non-connectable advertising duration in seconds (0 for infinite) */

    .low_duty_nonconn_min_interval = CY_BT_LOW_DUTY_NONCONN_ADV_MIN_INTERVAL, /* Low duty non-connectable minimum advertising
                                                                                 interval (in slots (1 slot = 0.625 ms)) */
    .low_duty_nonconn_max_interval = CY_BT_LOW_DUTY_NONCONN_ADV_MAX_INTERVAL, /* Low duty non-connectable maximum advertising
                                                                                 interval (in slots (1 slot = 0.625 ms)) */
    .low_duty_nonconn_duration =
        CY_BT_LOW_DUTY_NONCONN_ADV_DURATION /* Low duty non-connectable advertising duration in seconds (0 for infinite) */
};

/* BLE configuration settings */
const wiced_bt_cfg_ble_t cy_bt_cfg_ble = {
    .ble_max_simultaneous_links =
        (CY_BT_CLIENT_MAX_LINKS +
         CY_BT_SERVER_MAX_LINKS),                      /* Max number for simultaneous connections for a layer, profile, protocol */
    .ble_max_rx_pdu_size = CY_BT_L2CAP_MTU_SIZE,       /* Maximum size allowed for any received L2CAP PDU
                                                        * Minimum value - 65 (to support SM)
                                                        * Maximum GATT MTU over legacy bearers shall be set to <= this value
                                                        * Maximum MPS for EATT channels shall be set to <= this value */
    .appearance                   = CY_BT_APPEARANCE,  /* GATT appearance (see gatt_appearance_e) */
    .rpa_refresh_timeout          = CY_BT_RPA_TIMEOUT, /* Interval of  random address refreshing - secs */
    .host_addr_resolution_db_size = 5, /* LE Address Resolution DB settings - effective only for pre 4.2 controller */
    .p_ble_scan_cfg               = &cy_bt_cfg_scan_settings, /* BLE scan settings */
    .p_ble_advert_cfg             = &cy_bt_cfg_adv_settings,  /* BLE advertisement settings */
    .default_ble_power_level      = CY_BT_TX_POWER, /* Default LE power level, Refer lm_TxPwrTable table for the power range */
};

/* GATT settings */
const wiced_bt_cfg_gatt_t cy_bt_cfg_gatt = {
    .max_db_service_modules = 0, /* Maximum number of service modules in the DB*/
    .max_eatt_bearers       = 0, /* Maximum number of allowed gatt bearers */
};

/* Application-managed L2CAP protocol configuration */
const wiced_bt_cfg_l2cap_application_t cy_bt_cfg_l2cap = {
    .max_app_l2cap_psms              = CY_BT_L2CAP_MAX_LE_PSM,      /* Maximum number of application-managed PSMs */
    .max_app_l2cap_channels          = CY_BT_L2CAP_MAX_LE_CHANNELS, /* Maximum number of application-managed channels */
    .max_app_l2cap_le_fixed_channels = 0, /* Maximum number of application-managed fixed channels supported */
};

/* Bluetooth stack configuration */
const wiced_bt_cfg_settings_t wiced_bt_cfg_settings = {
    .device_name       = (uint8_t *) app_gap_device_name, /* Local device name (NULL terminated) */
    .security_required = CY_BT_SECURITY_LEVEL,            /* BTM_SEC_BEST_EFFORT is recommended choice for most applications,
                                                           * to connect to the widest range of devices. Allows stack to choose
                                                           * the highest level of security possible between the two devices */
    .p_ble_cfg       = &cy_bt_cfg_ble,                    /* BLE related configuration */
    .p_gatt_cfg      = &cy_bt_cfg_gatt,                   /* GATT settings */
    .p_l2cap_app_cfg = &cy_bt_cfg_l2cap,                  /* Application-managed L2CAP protocol configuration */
};
#else
/* Bluetooth stack configuration */
const wiced_bt_cfg_settings_t wiced_bt_cfg_settings =
    {
        .device_name                         = (uint8_t*)app_gap_device_name,                             /* Local device name (NULL terminated) */
        .device_class                        = {0x00, 0x00, 0x00},                                        /* Local device class */
        .security_requirement_mask           = BTM_SEC_NONE,                                              /* Security requirements mask (BTM_SEC_NONE, or combination of BTM_SEC_IN_AUTHENTICATE, BTM_SEC_OUT_AUTHENTICATE, BTM_SEC_ENCRYPT (see #wiced_bt_sec_level_e)) */

        .max_simultaneous_links              = (CY_BT_CLIENT_MAX_LINKS + CY_BT_SERVER_MAX_LINKS),         /* Maximum number simultaneous links to different devices */

        /* BLE scan settings  */
        .ble_scan_cfg =
        {
            .scan_mode                       = CY_BT_SCAN_MODE,                                           /* BLE scan mode (BTM_BLE_SCAN_MODE_PASSIVE, BTM_BLE_SCAN_MODE_ACTIVE, or BTM_BLE_SCAN_MODE_NONE) */

            /* Advertisement scan configuration */
            .high_duty_scan_interval         = CY_BT_HIGH_DUTY_SCAN_INTERVAL,                             /* High duty scan interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_scan_window           = CY_BT_HIGH_DUTY_SCAN_WINDOW,                               /* High duty scan window (in slots (1 slot = 0.625 ms)) */
            .high_duty_scan_duration         = CY_BT_HIGH_DUTY_SCAN_DURATION,                             /* High duty scan duration in seconds (0 for infinite) */

            .low_duty_scan_interval          = CY_BT_LOW_DUTY_SCAN_INTERVAL,                              /* Low duty scan interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_scan_window            = CY_BT_LOW_DUTY_SCAN_WINDOW,                                /* Low duty scan window (in slots (1 slot = 0.625 ms)) */
            .low_duty_scan_duration          = CY_BT_LOW_DUTY_SCAN_DURATION,                              /* Low duty scan duration in seconds (0 for infinite) */

            /* Connection scan configuration */
            .high_duty_conn_scan_interval    = CY_BT_HIGH_DUTY_CONN_SCAN_INTERVAL,                        /* High duty cycle connection scan interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_conn_scan_window      = CY_BT_HIGH_DUTY_CONN_SCAN_WINDOW,                          /* High duty cycle connection scan window (in slots (1 slot = 0.625 ms)) */
            .high_duty_conn_duration         = CY_BT_HIGH_DUTY_CONN_SCAN_DURATION,                        /* High duty cycle connection duration in seconds (0 for infinite) */

            .low_duty_conn_scan_interval     = CY_BT_LOW_DUTY_CONN_SCAN_INTERVAL,                         /* Low duty cycle connection scan interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_conn_scan_window       = CY_BT_LOW_DUTY_CONN_SCAN_WINDOW,                           /* Low duty cycle connection scan window (in slots (1 slot = 0.625 ms)) */
            .low_duty_conn_duration          = CY_BT_LOW_DUTY_CONN_SCAN_DURATION,                         /* Low duty cycle connection duration in seconds (0 for infinite) */

            /* Connection configuration */
            .conn_min_interval               = CY_BT_CONN_MIN_INTERVAL,                                   /* Minimum connection interval (in slots (1 slot = 1.25 ms)) */
            .conn_max_interval               = CY_BT_CONN_MAX_INTERVAL,                                   /* Maximum connection interval (in slots (1 slot = 1.25 ms)) */
            .conn_latency                    = CY_BT_CONN_LATENCY,                                        /* Connection latency */
            .conn_supervision_timeout        = CY_BT_CONN_SUPERVISION_TIMEOUT,                            /* Connection link supervision timeout (in 10 ms) */
        },

        /* BLE advertisement settings */
        .ble_advert_cfg =
        {
            .channel_map                     = CY_BT_CHANNEL_MAP,                                         /* Advertising channel map (mask of BTM_BLE_ADVERT_CHNL_37, BTM_BLE_ADVERT_CHNL_38, BTM_BLE_ADVERT_CHNL_39) */

            .high_duty_min_interval          = CY_BT_HIGH_DUTY_ADV_MIN_INTERVAL,                          /* High duty undirected connectable minimum advertising interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_max_interval          = CY_BT_HIGH_DUTY_ADV_MAX_INTERVAL,                          /* High duty undirected connectable maximum advertising interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_duration              = CY_BT_HIGH_DUTY_ADV_DURATION,                              /* High duty undirected connectable advertising duration in seconds (0 for infinite) */

            .low_duty_min_interval           = CY_BT_LOW_DUTY_ADV_MIN_INTERVAL,                           /* Low duty undirected connectable minimum advertising interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_max_interval           = CY_BT_LOW_DUTY_ADV_MAX_INTERVAL,                           /* Low duty undirected connectable maximum advertising interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_duration               = CY_BT_LOW_DUTY_ADV_DURATION,                               /* Low duty undirected connectable advertising duration in seconds (0 for infinite) */

            .high_duty_directed_min_interval = CY_BT_HIGH_DUTY_DIRECTED_ADV_MIN_INTERVAL,                 /* High duty directed connectable minimum advertising interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_directed_max_interval = CY_BT_HIGH_DUTY_DIRECTED_ADV_MAX_INTERVAL,                 /* High duty directed connectable maximum advertising interval (in slots (1 slot = 0.625 ms)) */

            .low_duty_directed_min_interval  = CY_BT_LOW_DUTY_DIRECTED_ADV_MIN_INTERVAL,                  /* Low duty directed connectable minimum advertising interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_directed_max_interval  = CY_BT_LOW_DUTY_DIRECTED_ADV_MAX_INTERVAL,                  /* Low duty directed connectable maximum advertising interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_directed_duration      = CY_BT_LOW_DUTY_DIRECTED_ADV_DURATION,                      /* Low duty directed connectable advertising duration in seconds (0 for infinite) */

            .high_duty_nonconn_min_interval  = CY_BT_HIGH_DUTY_NONCONN_ADV_MIN_INTERVAL,                  /* High duty non-connectable minimum advertising interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_nonconn_max_interval  = CY_BT_HIGH_DUTY_NONCONN_ADV_MAX_INTERVAL,                  /* High duty non-connectable maximum advertising interval (in slots (1 slot = 0.625 ms)) */
            .high_duty_nonconn_duration      = CY_BT_HIGH_DUTY_NONCONN_ADV_DURATION,                      /* High duty non-connectable advertising duration in seconds (0 for infinite) */

            .low_duty_nonconn_min_interval   = CY_BT_LOW_DUTY_NONCONN_ADV_MIN_INTERVAL,                   /* Low duty non-connectable minimum advertising interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_nonconn_max_interval   = CY_BT_LOW_DUTY_NONCONN_ADV_MAX_INTERVAL,                   /* Low duty non-connectable maximum advertising interval (in slots (1 slot = 0.625 ms)) */
            .low_duty_nonconn_duration       = CY_BT_LOW_DUTY_NONCONN_ADV_DURATION                        /* Low duty non-connectable advertising duration in seconds (0 for infinite) */
        },

        /* GATT configuration */
        .gatt_cfg =
        {
            .appearance                     = CY_BT_APPEARANCE,                                           /* GATT appearance (see gatt_appearance_e) */
            .client_max_links               = CY_BT_CLIENT_MAX_LINKS,                                     /* Client config: maximum number of servers that local client can connect to  */
            .server_max_links               = CY_BT_SERVER_MAX_LINKS,                                     /* Server config: maximum number of remote clients connections allowed by the local */
            .max_attr_len                   = CY_BT_MAX_ATTR_LEN,                                         /* Maximum attribute length; gki_cfg must have a corresponding buffer pool that can hold this length */
            .max_mtu_size                   = CY_BT_MTU_SIZE                                              /* Maximum MTU size for GATT connections, should be between 23 and (max_attr_len + 5) */
        },

        /* RFCOMM configuration */
        .rfcomm_cfg =
        {
            .max_links                      = 0,                                                          /* Maximum number of simultaneous connected remote devices */
            .max_ports                      = 0                                                           /* Maximum number of simultaneous RFCOMM ports */
        },

        /* Application-managed L2CAP protocol configuration */
        .l2cap_application =
        {
            .max_links                      = 0,                                                          /* Maximum number of application-managed l2cap links (BR/EDR and LE) */

            /* BR EDR l2cap configuration */
            .max_psm                        = 0,                                                          /* Maximum number of application-managed BR/EDR PSMs */
            .max_channels                   = 0,                                                          /* Maximum number of application-managed BR/EDR channels */

            /* LE L2cap connection-oriented channels configuration */
            .max_le_psm                     = CY_BT_L2CAP_MAX_LE_PSM,                                     /* Maximum number of application-managed LE PSMs */
            .max_le_channels                = CY_BT_L2CAP_MAX_LE_CHANNELS,                                /* Maximum number of application-managed LE channels */
            /* LE L2cap fixed channel configuration */
            .max_le_l2cap_fixed_channels    = 0,                                                          /* Maximum number of application-managed fixed channels supported (in addition to mandatory channels 4, 5 and 6) */
            .max_rx_mtu                     = CY_BT_L2CAP_MTU_SIZE                                        /* Maximum RX MTU allowed */
        },

        .addr_resolution_db_size            = 5,                                                          /* LE Address Resolution DB settings - effective only for pre 4.2 controller */
        .rpa_refresh_timeout                = CY_BT_RPA_TIMEOUT,                                          /* Interval of  random address refreshing - secs */
        .stack_scratch_size                 = WICED_BT_CFG_DEFAULT_STACK_SCRATCH_SIZE,                    /* Memory area reserved for the stack transient memory requirements */
        .ble_white_list_size                = CY_BT_WHITE_LIST_SIZE,                                      /* Maximum number of white list devices allowed. Cannot be more than 128 */
        .default_ble_power_level            = CY_BT_TX_POWER                                              /* Default LE power level */
    };
#endif

#ifdef __cplusplus
}
#endif
