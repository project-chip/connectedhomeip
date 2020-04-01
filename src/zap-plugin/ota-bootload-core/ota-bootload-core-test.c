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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE
#include "ota-bootload-core-test.h"

// This file contains common test utilities for the ota-bootload-client
// and ota-bootload-server scripted tests.

const EmberZclClusterSpec_t emberZclClusterOtaBootloadServerSpec = {
  .role = EMBER_ZCL_ROLE_SERVER,
  .manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL,
  .id = 0x2000, // OTA cluster ID
};

#define USE_STUB_heapMemory
#define USE_STUB_heapMemorySize
#define USE_STUB_emTaskCount
#define USE_STUB_emTasks
#define USE_STUB_emberFreeMemoryForPacketHandler
#define USE_STUB_halCommonIdleForMilliseconds
#define USE_STUB_halCommonGetRandomTraced
#include "stack/ip/stubs.c"

static uint32_t systemTick = 0;

uint32_t scriptTime(void)
{
  return systemTick;
}

void scriptTick(void)
{
  systemTick++;
  emberRunEvents(emAppEvents);
}

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return systemTick;
}

// From coap.c.
uint32_t emberBlockOptionValue(bool more,
                               uint8_t logSize,
                               uint32_t number)
{
  return ((logSize - 4)
          | (more ? 0x08 : 0)
          | (number << 4));
}

void emberInitCoapOption(EmberCoapOption *option,
                         EmberCoapOptionType type,
                         uint32_t value)
{
  option->type = type;
  option->value = NULL;
  option->valueLength = 0;
  option->intValue = value;
}

uint32_t emberBlockOptionOffset(EmberCoapBlockOption *option)
{
  return option->number << option->logSize;
}

bool emberReadBlockOption(EmberCoapReadOptions *options,
                          EmberCoapOptionType type,
                          EmberCoapBlockOption *option)
{
  // Cheat to win.
  uint8_t *bytes = (void *)options;
  option->more = (bool)bytes[0];
  option->logSize = bytes[1];
  option->number = bytes[2];
  return true;
}

void emberParseBlockOptionValue(uint32_t value,
                                EmberCoapBlockOption *option)
{
  option->more = (value & 0x08) != 0;
  option->logSize = (value & 0x07) + 4;
  option->number = value >> 4;
}

bool emberVerifyBlockOption(const EmberCoapBlockOption *blockOption,
                            uint16_t payloadLength,
                            uint8_t expectedLogSize)
{
  return ((blockOption->more
           ? payloadLength == (1 << blockOption->logSize)
           : payloadLength <= (1 << blockOption->logSize))
          && (blockOption->number == 0
              ? blockOption->logSize <= expectedLogSize
              : blockOption->logSize == expectedLogSize));
}
