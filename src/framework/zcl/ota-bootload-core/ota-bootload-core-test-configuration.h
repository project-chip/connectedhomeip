/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef OTA_BOOTLOAD_CORE_TEST_CONFIGURATION_H
#define OTA_BOOTLOAD_CORE_TEST_CONFIGURATION_H

#define EMBER_AF_API_HAL "hal/hal.h"
#define EMBER_AF_API_EEPROM "../../util/plugin/plugin-common/eeprom/eeprom.h"
#define EMBER_AF_API_STACK "stack/include/ember.h"
#define EMBER_AF_API_BUFFER_MANAGEMENT "stack/framework/buffer-management.h"
#define EMBER_AF_API_EVENT_QUEUE "stack/framework/event-queue.h"
#define EMBER_AF_API_ZCL_CORE "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#define EMBER_AF_API_ZCL_CORE_DTLS_MANAGER "app/thread/plugin/zcl/zcl-core/zcl-core-dtls-manager.h"
#define EMBER_AF_API_ZCL_CORE_WELL_KNOWN "app/thread/plugin/zcl/zcl-core/zcl-core-well-known.h"
#define EMBER_AF_API_ZCL_OTA_BOOTLOAD_CLIENT "app/thread/plugin/zcl/ota-bootload-client/ota-bootload-client.h"
#define EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE "app/thread/plugin/zcl/ota-bootload-core/ota-bootload-core.h"
#define EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE "app/thread/plugin/zcl/ota-bootload-storage-core/ota-bootload-storage-core.h"
#define EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_UNIX "app/thread/plugin/zcl/ota-bootload-storage-unix/ota-bootload-storage-unix.h"
#define EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_STATIC_FILE "app/thread/plugin/zcl/ota-bootload-storage-core/ota-static-file-data.h"

#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_AUTO_START
#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_BLOCK_ERROR_THRESHOLD 7
#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_DISCOVER_SERVER_PERIOD_MINUTES 10
#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_QUERY_NEXT_IMAGE_PERIOD_MINUTES 60

#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_START_ADDRESS 0
#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_EEPROM_END_ADDRESS 204799

#define EMBER_AF_PLUGIN_EEPROM_PARTIAL_WORD_STORAGE_COUNT 2

#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_UNIX_FILE_CLOSE_PERIOD_MS 16384
#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_UNIX_MAX_FILE_COUNT 1
#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_STORAGE_UNIX_MAX_FILE_SIZE 1024

#define EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_IMAGE_NOTIFY_PERIOD_MINUTES 60

#endif // OTA_BOOTLOAD_CORE_TEST_CONFIGURATION_H
