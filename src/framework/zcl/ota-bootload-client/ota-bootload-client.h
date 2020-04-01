/***************************************************************************//**
 * @file
 * @brief ZCL OTA Bootload Client API
 ******************************************************************************/

#ifndef __OTA_BOOTLOAD_CLIENT_H__
#define __OTA_BOOTLOAD_CLIENT_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE

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
  ChipZclScheme_t scheme;
  uint8_t const *name;
  uint8_t nameLength;
  /** IPv6 address of the server. */
  ChipIpv6Address address;
  /** UDP port of the server. */
  uint16_t port;
  /** UID of the server. */
  ChipZclUid_t uid;
  /** Endpoint ID of the server. */
  ChipZclEndpointId_t endpointId;
} ChipZclOtaBootloadClientServerInfo_t;

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
