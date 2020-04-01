/***************************************************************************//**
 * @file
 * @brief ZCL OTA Bootload Client API
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

#ifndef __OTA_BOOTLOAD_CLIENT_H__
#define __OTA_BOOTLOAD_CLIENT_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup OTA_Bootload
 * @{
 */

/**
 * @addtogroup OTA_Bootload_Types
 * @{
 */

// -----------------------------------------------------------------------------
// Types

/** This structure holds information about an OTA Server device. */
typedef struct {
  /** Protocol used to communicate with the server. */
  EmberZclScheme_t scheme;
  uint8_t const *name;
  uint8_t nameLength;
  /** IPv6 address of the server. */
  EmberIpv6Address address;
  /** UDP port of the server. */
  uint16_t port;
  /** UID of the server. */
  EmberZclUid_t uid;
  /** Endpoint ID of the server. */
  EmberZclEndpointId_t endpointId;
} EmberZclOtaBootloadClientServerInfo_t;

/** @} end addtogroup OTA_Bootload_Types */

/**
 * @addtogroup OTA_Bootload_API
 *
 * See @ref ota-bootload-client-callbacks for the application callbacks
 * associated with the OTA Bootload Client plugin.
 * @{
 */

// -----------------------------------------------------------------------------
// API

/** @} end addtogroup OTA_Bootload_API */

/** @} end addtogroup OTA_Bootload */

/** @} end addtogroup ZCLIP */

#endif // __OTA_BOOTLOAD_CLIENT_H__
