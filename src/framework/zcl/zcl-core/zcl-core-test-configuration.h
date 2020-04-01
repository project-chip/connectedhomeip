/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_CORE_TEST_CONFIGURATION_H
#define ZCL_CORE_TEST_CONFIGURATION_H

#define CHIP_AF_API_STACK "stack/include/chip.h"
#define CHIP_AF_API_BUFFER_MANAGEMENT "stack/framework/buffer-management.h"
#define CHIP_AF_API_BUFFER_QUEUE "stack/framework/buffer-queue.h"
#define CHIP_AF_API_HAL "hal/hal.h"
#define CHIP_ZCL_BINDING_TABLE_SIZE 8
#define CHIP_ZCL_CACHE_TABLE_SIZE 8
#define CHIP_ZCL_GROUP_TABLE_SIZE 8
#define CHIP_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE 8

#define CHIP_AF_API_ZCL_CORE "app/thread/plugin/zcl/zcl-core/zcl-core.h"
#define CHIP_AF_API_ZCL_CORE_DTLS_MANAGER "app/thread/plugin/zcl/zcl-core/zcl-core-dtls-manager.h"
#define CHIP_AF_API_ZCL_CORE_WELL_KNOWN "app/thread/plugin/zcl/zcl-core/zcl-core-well-known.h"
#define CHIP_AF_API_ZCL_CORE_RESOURCE_DIRECTORY "app/thread/plugin/zcl/zcl-core/zcl-core-resource-directory.h"

#define CH_ZCL_MAX_REMOTE_ACCESS_TOKENS 2

#define CHIP_AF_PRINT_CORE
#define CHIP_AF_PRINT_APP
#define CHIP_AF_PRINT_BITS
#define CHIP_AF_PRINT_NAMES
#define chipAfAppPrint(...)
#define chipAfAppPrintln(...)
#define chipAfAppFlush()
#define chipAfAppDebugExec(x)
#define chipAfAppPrintBuffer(buffer, len, withSpace)
#define chipAfAppPrintString(buffer)

#endif // ZCL_CORE_TEST_CONFIGURATION_H
