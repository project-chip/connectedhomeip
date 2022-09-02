/***************************************************************************//**
 * @brief ZigBee Debug Print component configuration header.
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

// <<< Use Configuration Wizard in Context Menu >>>

// <h>ZigBee Debug Print configuration

// <q SL_ZIGBEE_DEBUG_STACK_GROUP_ENABLED> Stack group enabled
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "stack" group shall be included in the build.
#define SL_ZIGBEE_DEBUG_STACK_GROUP_ENABLED                                     (1)

// <q SL_ZIGBEE_DEBUG_STACK_GROUP_RUNTIME_DEFAULT> Stack group runtime default
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "stack" group shall be runtime enabled by default.
#define SL_ZIGBEE_DEBUG_STACK_GROUP_RUNTIME_DEFAULT                             (1)

// <q SL_ZIGBEE_DEBUG_CORE_GROUP_ENABLED> Core group enabled
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "core" group shall be included in the build.
#define SL_ZIGBEE_DEBUG_CORE_GROUP_ENABLED                                      (1)

// <q SL_ZIGBEE_DEBUG_CORE_GROUP_RUNTIME_DEFAULT> Core group runtime default
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "core" group shall be runtime enabled by default.
#define SL_ZIGBEE_DEBUG_CORE_GROUP_RUNTIME_DEFAULT                              (1)

// <q SL_ZIGBEE_DEBUG_APP_GROUP_ENABLED> App group enabled
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "app" group shall be included in the build.
#define SL_ZIGBEE_DEBUG_APP_GROUP_ENABLED                                       (1)

// <q SL_ZIGBEE_DEBUG_APP_GROUP_RUNTIME_DEFAULT> App group enabled
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "app" group shall be runtime enabled by default.
#define SL_ZIGBEE_DEBUG_APP_GROUP_RUNTIME_DEFAULT                               (1)

// <q SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED> ZCL group enabled
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "zcl" group shall be included in the build.
#define SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED                                       (1)

// <q SL_ZIGBEE_DEBUG_ZCL_GROUP_RUNTIME_DEFAULT> ZCL group enabled
// <i> Default: 1
// <i> If this option is enabled, prints belonging to the "zcl" group shall be runtime enabled by default.
#define SL_ZIGBEE_DEBUG_ZCL_GROUP_RUNTIME_DEFAULT                               (1)

// <q SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED> Legacy App Framework Debug group enabled
// <i> Default: 0
// <i> If both this option and ZIGBEE_DEBUG_ZCL_GROUP_ENABLED are enabled, prints belonging to the "legacy app framework debug" group shall be included in the build.
#define SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED                      (0)

// <q SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_RUNTIME_DEFAULT> Legacy App Framework Debug runtime default
// <i> Default: 0
// <i> If both this option and ZIGBEE_DEBUG_ZCL_GROUP_ENABLED are enabled, prints belonging to the "legacy app framework debug" group shall be runtime enabled by default.
#define SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_RUNTIME_DEFAULT              (0)

// </h>

// <<< end of configuration section >>>
