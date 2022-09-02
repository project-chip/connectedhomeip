/***************************************************************************//**
 * @file zigbee_zcl_callback_dispatcher.c
 * @brief ZCL callback dispatcher definitions.
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

#include PLATFORM_HEADER
#include "af.h"
#include "zigbee_zcl_callback_dispatcher.h"

// Reset Attributes
void emAfResetAttributes(uint8_t endpointId)
{
  (void)endpointId;

}

// Pre command received
bool emAfPreCommandReceived(EmberAfClusterCommand* cmd)
{
  return (
emberAfPreCommandReceivedCallback(cmd)
);
}

// Pre ZDO message received
bool emAfPreZDOMessageReceived(EmberNodeId nodeId, EmberApsFrame* apsFrame, uint8_t* message, uint16_t length)
{
  return (
emberAfPreZDOMessageReceivedCallback(nodeId, apsFrame, message, length)
);
}

// ZDO message received
void emAfZDOMessageReceived(EmberNodeId sender, EmberApsFrame* apsFrame, uint8_t* message, uint16_t length)
{
  (void)sender;
  (void)apsFrame;
  (void)message;
  (void)length;
}

// Retrieve attribute and craft response
bool emAfRetrieveAttributeAndCraftResponse(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attrId, uint8_t mask, uint16_t maunfacturerCode, uint16_t readLength)
{
  (void)endpoint;
  (void)clusterId;
  (void)attrId;
  (void)mask;
  (void)maunfacturerCode;
  (void)readLength;

return (false

);
}

// Read attributes response
bool emAfReadAttributesResponse(EmberAfClusterId clusterId, uint8_t* buffer, uint16_t bufLen)
{
  return (
emberAfReadAttributesResponseCallback(clusterId, buffer, bufLen)
);
}

// Report attributes
bool emAfReportAttributes(EmberAfClusterId clusterId, uint8_t * buffer, uint16_t bufLen)
{
  return (
emberAfReportAttributesCallback(clusterId, buffer, bufLen)
);
}