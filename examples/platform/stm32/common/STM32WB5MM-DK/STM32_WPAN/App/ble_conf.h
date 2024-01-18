/**
 ******************************************************************************
 * File Name          : App/ble_conf.h
 * Description        : Configuration file for BLE Middleware.
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BLE_CONF_H
#define BLE_CONF_H

#include "app_conf.h"

/******************************************************************************
 *
 * BLE SERVICES CONFIGURATION
 * blesvc
 *
 ******************************************************************************/

/**
 * This setting shall be set to '1' if the device needs to support the Peripheral Role
 * In the MS configuration, both BLE_CFG_PERIPHERAL and BLE_CFG_CENTRAL shall be set to '1'
 */
#define BLE_CFG_PERIPHERAL 1

/**
 * This setting shall be set to '1' if the device needs to support the Central Role
 * In the MS configuration, both BLE_CFG_PERIPHERAL and BLE_CFG_CENTRAL shall be set to '1'
 */
#define BLE_CFG_CENTRAL 0

/**
 * There is one handler per service enabled
 * Note: There is no handler for the Device Information Service
 *
 * This shall take into account all registered handlers
 * (from either the provided services or the custom services)
 */
#define BLE_CFG_SVC_MAX_NBR_CB 3

#define BLE_CFG_CLT_MAX_NBR_CB 0

/******************************************************************************
 * GAP Service - Appearance
 ******************************************************************************/

#define BLE_CFG_UNKNOWN_APPEARANCE (0)
#define BLE_CFG_HR_SENSOR_APPEARANCE (832)
#define BLE_CFG_GAP_APPEARANCE (BLE_CFG_UNKNOWN_APPEARANCE)

/******************************************************************************
 * Over The Air Feature (OTA) - STM Proprietary
 ******************************************************************************/
#define BLE_CFG_OTA_REBOOT_CHAR 0 /**< REBOOT OTA MODE CHARACTERISTIC */

#endif /*BLE_CONF_H */
