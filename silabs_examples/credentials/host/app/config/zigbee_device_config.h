/***************************************************************************//**
 * @brief ZigBee device configuration header.
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// <h>ZigBee device configuration

#ifndef SILABS_ZIGBEE_DEVICE_CONFIG_H
#define SILABS_ZIGBEE_DEVICE_CONFIG_H

// Mirrors the legacy EmberNodeType enum
#define SLI_ZIGBEE_NETWORK_DEVICE_TYPE_COORDINATOR_OR_ROUTER  0x01
#define SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER                 0x02
#define SLI_ZIGBEE_NETWORK_DEVICE_TYPE_END_DEVICE             0x03
#define SLI_ZIGBEE_NETWORK_DEVICE_TYPE_SLEEPY_END_DEVICE      0x04

// Mirrors the legacy EmberAfSecurityProfile enum
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_NO_SECURITY          0x00
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_HA                   0x01
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_HA_1_2               0x02
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_SE_TEST              0x03
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_SE_FULL              0x04
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0                  0x05
#define SLI_ZIGBEE_NETWORK_SECURITY_TYPE_CUSTOM               0xFF

#define SLI_ZIGBEE_DEFAULT_NETWORK_PRIMARY                    0x00
#define SLI_ZIGBEE_DEFAULT_NETWORK_SECONDARY                  0x01

#define SLI_ZIGBEE_TX_POWER_MODE_DEFAULT                      EMBER_TX_POWER_MODE_DEFAULT
#define SLI_ZIGBEE_TX_POWER_MODE_BOOST                        EMBER_TX_POWER_MODE_BOOST
#define SLI_ZIGBEE_TX_POWER_MODE_ALTERNATE                    EMBER_TX_POWER_MODE_ALTERNATE
#define SLI_ZIGBEE_TX_POWER_MODE_BOOST_AND_ALTERNATE          EMBER_TX_POWER_MODE_BOOST_AND_ALTERNATE
#define SLI_ZIGBEE_TX_POWER_MODE_USE_TOKEN                    EMBER_TX_POWER_MODE_USE_TOKEN

// <o SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE> Primary Network Device Type
// <i> The ZigBee Primary Network Device Type
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_COORDINATOR_OR_ROUTER=> Coordinator or Router
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER=> Router
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_END_DEVICE=> End Device
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_SLEEPY_END_DEVICE=> Sleepy End Device
// <i> Default: SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER
#define SLI_ZIGBEE_PRIMARY_NETWORK_DEVICE_TYPE       SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER

// <o SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE> Primary Network Security Type
// <i> The ZigBee Primary Network Security Type
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_NO_SECURITY => No Security
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_HA => Home Automation Security
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_HA_1_2 => Home Automation Security 1.2
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_SE_FULL => Smart Energy Security Full (compliant)
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_SE_TEST => Smart Energy Security Test (dev only)
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0 => ZigBee 3.0 Security
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_CUSTOM => Custom Security
// <i> Default: SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0
#define SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE     SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0

// <e SLI_ZIGBEE_SECONDARY_NETWORK_ENABLED> Enable Secondary Network
// <i> Default: 0
// <i> Enable/Disable the ZigBee Secondary Network
#define SLI_ZIGBEE_SECONDARY_NETWORK_ENABLED         (0)

// <o SLI_ZIGBEE_SECONDARY_NETWORK_DEVICE_TYPE> Secondary Network Device Type
// <i> The ZigBee Secondary Network Device Type
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_COORDINATOR_OR_ROUTER=> Coordinator or Router
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER=> Router
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_END_DEVICE=> End Device
// <SLI_ZIGBEE_NETWORK_DEVICE_TYPE_SLEEPY_END_DEVICE=> Sleepy End Device
// <i> Default: SLI_ZIGBEE_NETWORK_DEVICE_TYPE_ROUTER
#define SLI_ZIGBEE_SECONDARY_NETWORK_DEVICE_TYPE     SLI_ZIGBEE_NETWORK_DEVICE_TYPE_SLEEPY_END_DEVICE

// <o SLI_ZIGBEE_SECONDARY_NETWORK_SECURITY_TYPE> Secondary Network Security Type
// <i> The ZigBee Secondary Network Security Type
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_NO_SECURITY => No Security
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_HA => Home Automation Security
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_SE_FULL => Smart Energy Security Full (compliant)
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_SE_TEST => Smart Energy Security Test (dev only)
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0 => ZigBee 3.0 Security
// <SLI_ZIGBEE_NETWORK_SECURITY_TYPE_CUSTOM => Custom Security
// <i> Default: SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0
#define SLI_ZIGBEE_SECONDARY_NETWORK_SECURITY_TYPE   SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0

// <o SLI_ZIGBEE_DEFAULT_NETWORK> Default Network
// <i> The ZigBee Default Network
// <SLI_ZIGBEE_DEFAULT_NETWORK_PRIMARY => Primary Network
// <SLI_ZIGBEE_DEFAULT_NETWORK_SECONDARY => Secondary Network
// <i> Default: SLI_ZIGBEE_NETWORK_SECURITY_TYPE_3_0
#define SLI_ZIGBEE_DEFAULT_NETWORK                   SLI_ZIGBEE_DEFAULT_NETWORK_PRIMARY

// <o SLI_ZIGBEE_TX_POWER_MODE> TX power mode
// <i> The ZigBee node transmission power mode
// <SLI_ZIGBEE_TX_POWER_MODE_DEFAULT => Default
// <SLI_ZIGBEE_TX_POWER_MODE_BOOST => Boost
// <SLI_ZIGBEE_TX_POWER_MODE_ALTERNATE => Alternate
// <SLI_ZIGBEE_TX_POWER_MODE_BOOST_AND_ALTERNATE => Boost & Alternate
// <SLI_ZIGBEE_TX_POWER_MODE_USE_TOKEN => Use token
// <i> Default: SLI_ZIGBEE_TX_POWER_MODE_USE_TOKEN
#define SLI_ZIGBEE_TX_POWER_MODE                     SLI_ZIGBEE_TX_POWER_MODE_USE_TOKEN

// </e>

#endif // SILABS_ZIGBEE_DEVICE_CONFIG_H

// </h>

// <<< end of configuration section >>>
