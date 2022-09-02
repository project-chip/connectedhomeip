/***************************************************************************//**
 * @file zigbee_zcl_callback_dispatcher.h
 * @brief ZCL callback dispatcher declarations.
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

#ifndef SL_ZIGBEE_AF_CALLBACK_DISPATCHER_H
#define SL_ZIGBEE_AF_CALLBACK_DISPATCHER_H

#include PLATFORM_HEADER
#include "af.h"

// Reset Attributes
void emAfResetAttributes(uint8_t endpointId);

// Pre command received
bool emAfPreCommandReceived(EmberAfClusterCommand* cmd);
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd);

// Pre ZDO message received
bool emAfPreZDOMessageReceived(EmberNodeId nodeId, EmberApsFrame* apsFrame, uint8_t* message, uint16_t length);
bool emberAfPreZDOMessageReceivedCallback(EmberNodeId nodeId, EmberApsFrame* apsFrame, uint8_t* message, uint16_t length);

// ZDO message received
void emAfZDOMessageReceived(EmberNodeId sender, EmberApsFrame* apsFrame, uint8_t* message, uint16_t length);

// Retrieve attribute and craft response
bool emAfRetrieveAttributeAndCraftResponse(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attrId, uint8_t mask, uint16_t maunfacturerCode, uint16_t readLength);

bool emAfReadAttributesResponse(EmberAfClusterId clusterId, uint8_t* buffer, uint16_t bufLen);
// Read attributes response
bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId, uint8_t* buffer, uint16_t bufLen);

// Report attributes
bool emAfReportAttributes(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen);
bool emberAfReportAttributesCallback(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

#endif // SL_ZIGBEE_AF_CALLBACK_DISPATCHER_H