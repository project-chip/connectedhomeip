/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_ZCL_CORE
#include "ota-bootload-core-test.h"

// This file contains common test utilities for the ota-bootload-client
// and ota-bootload-server scripted tests.

const ChipZclClusterSpec_t chipZclClusterOtaBootloadServerSpec = {
  .role = CHIP_ZCL_ROLE_SERVER,
  .manufacturerCode = CHIP_ZCL_MANUFACTURER_CODE_NULL,
  .id = 0x2000, // OTA cluster ID
};

#define USE_STUB_heapMemory
#define USE_STUB_heapMemorySize
#define USE_STUB_emTaskCount
#define USE_STUB_emTasks
#define USE_STUB_chipFreeMemoryForPacketHandler
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
  chipRunEvents(emAppEvents);
}

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return systemTick;
}

// From coap.c.
uint32_t chipBlockOptionValue(bool more,
                               uint8_t logSize,
                               uint32_t number)
{
  return ((logSize - 4)
          | (more ? 0x08 : 0)
          | (number << 4));
}

void chipInitCoapOption(ChipCoapOption *option,
                         ChipCoapOptionType type,
                         uint32_t value)
{
  option->type = type;
  option->value = NULL;
  option->valueLength = 0;
  option->intValue = value;
}

uint32_t chipBlockOptionOffset(ChipCoapBlockOption *option)
{
  return option->number << option->logSize;
}

bool chipReadBlockOption(ChipCoapReadOptions *options,
                          ChipCoapOptionType type,
                          ChipCoapBlockOption *option)
{
  // Cheat to win.
  uint8_t *bytes = (void *)options;
  option->more = (bool)bytes[0];
  option->logSize = bytes[1];
  option->number = bytes[2];
  return true;
}

void chipParseBlockOptionValue(uint32_t value,
                                ChipCoapBlockOption *option)
{
  option->more = (value & 0x08) != 0;
  option->logSize = (value & 0x07) + 4;
  option->number = value >> 4;
}

bool chipVerifyBlockOption(const ChipCoapBlockOption *blockOption,
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
