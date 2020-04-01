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
#include EMBER_AF_API_HAL
#include EMBER_AF_API_SERIAL
#include EMBER_AF_API_DEBUG_PRINT

// When EMBER_AF_DEBUG_PRINT_USE_PORT is defined, the underlying serial code
// functions require a port, so one is passed in.
#ifdef EMBER_AF_DEBUG_PRINT_USE_PORT
  #ifdef HAL_CONFIG
  #include "hal-config.h"
  #include "ember-hal-config.h"
  #endif
  #define emAfWaitSend()            emberSerialWaitSend(APP_SERIAL)
  #define emAfPrintf(...)           emberSerialPrintf(APP_SERIAL, __VA_ARGS__)
  #define emAfPrintfLine(...)       emberSerialPrintfLine(APP_SERIAL, __VA_ARGS__)
  #define emAfPrintCarriageReturn() emberSerialPrintCarriageReturn(APP_SERIAL)
  #define emAfPrintfVarArg(...)     emberSerialPrintfVarArg(APP_SERIAL, __VA_ARGS__)
#else
  #define emAfWaitSend()            emberSerialWaitSend()
  #define emAfPrintf(...)           emberSerialPrintf(__VA_ARGS__)
  #define emAfPrintfLine(...)       emberSerialPrintfLine(__VA_ARGS__)
  #define emAfPrintCarriageReturn() emberSerialPrintCarriageReturn()
  #define emAfPrintfVarArg(...)     emberSerialPrintfVarArg(__VA_ARGS__)
#endif

// A internal printing area is a 16-bit value.  The high byte is an index and
// the low byte is a bitmask.  The index is used to look up a byte and the
// bitmask is used to check if a single bit in that byte is set.  If it is set,
// the area is enabled.  Otherwise, the area is diabled.
#define AREA_INDEX(area)    HIGH_BYTE(area)
#define AREA_BITMASK(area)  LOW_BYTE(area)

// Areas can be enabled or disabled at runtime.  This is not done using the
// internal area, but with a user area, which is simply an offset into a zero-
// indexed array of areas.  The idea is that area names are printed like this:
//   [0] Core : YES
//   [1] Debug : no
//   [2] Applicaion : YES
//   ...
// If the user wanted to turn on the "Debug" area, he would call
// emberAfPrintOn(1), presumably via a CLI command.  The internal area can be
// reconstructed from the user area through bit magic.
#define USER_AREA_TO_INTERNAL_AREA(userArea) \
  HIGH_LOW_TO_INT((userArea) / 8, BIT((userArea) % 8))

uint16_t emberAfPrintActiveArea = 0;

#ifdef EMBER_AF_PRINT_BITS
static uint8_t bitmasks[] = EMBER_AF_PRINT_BITS;

  #define ENABLE(userArea) \
  printEnable(USER_AREA_TO_INTERNAL_AREA(userArea), true);
  #define DISABLE(userArea) \
  printEnable(USER_AREA_TO_INTERNAL_AREA(userArea), false);
  #define ENABLE_ALL() MEMSET(bitmasks, 0xFF, sizeof(bitmasks))
  #define DISABLE_ALL() MEMSET(bitmasks, 0x00, sizeof(bitmasks))

static void printEnable(uint16_t area, bool on)
{
  uint8_t index = AREA_INDEX(area);
  if (index < sizeof(bitmasks)) {
    uint8_t bitmask = AREA_BITMASK(area);
    if (on) {
      SETBITS(bitmasks[index], bitmask);
    } else {
      CLEARBITS(bitmasks[index], bitmask);
    }
  }
}
#else
  #define ENABLE(userArea)
  #define DISABLE(userArea)
  #define ENABLE_ALL()
  #define DISABLE_ALL()
#endif

#ifdef EMBER_AF_PRINT_NAMES
static PGM_P names[] = EMBER_AF_PRINT_NAMES;
#endif

bool emberAfPrintEnabled(uint16_t area)
{
  emberAfPrintActiveArea = area;
  if (area == 0xFFFF) {
    return true;
#ifdef EMBER_AF_PRINT_BITS
  } else {
    uint8_t index = AREA_INDEX(area);
    if (index < sizeof(bitmasks)) {
      uint8_t bitmask = AREA_BITMASK(area);
      return READBITS(bitmasks[index], bitmask);
    }
#endif
  }
  return false;
}

void emberAfPrintOn(uint16_t userArea)
{
  ENABLE(userArea);
}

void emberAfPrintOff(uint16_t userArea)
{
  DISABLE(userArea);
}

void emberAfPrintAllOn(void)
{
  ENABLE_ALL();
}

void emberAfPrintAllOff(void)
{
  DISABLE_ALL();
}

void emberAfPrintStatus(void)
{
#ifdef EMBER_AF_PRINT_NAMES
  uint8_t i;
  for (i = 0; i < EMBER_AF_PRINT_NAME_NUMBER; i++) {
    emAfPrintfLine("[%d] %p : %p",
                   i,
                   names[i],
                   (emberAfPrintEnabled(USER_AREA_TO_INTERNAL_AREA(i))
                    ? "YES"
                    : "no"));
    emAfWaitSend();
  }
#endif
}

#if defined(EMBER_AF_PRINT_AREA_NAME) && defined(EMBER_AF_PRINT_NAMES)
// If the area is bogus, this may still print a name, but it shouldn't crash.
static void printAreaName(uint16_t area)
{
  uint16_t index;
  uint8_t bitmask = AREA_BITMASK(area);
  uint8_t bit;

  for (bit = 0; bit < 8; bit++) {
    if (READBIT(bitmask, bit)) {
      break;
    }
  }
  index = AREA_INDEX(area) * 8 + bit;

  if (area != 0xFFFF
      && index < EMBER_AF_PRINT_NAME_NUMBER) {
    emAfPrintf("%p:", names[index]);
  }
}
#else
  #define printAreaName(area)
#endif

static void printVarArg(uint16_t area,
                        bool newline,
                        PGM_P formatString,
                        va_list args)
{
  if (emberAfPrintEnabled(area)) {
    printAreaName(area);
    emAfPrintfVarArg(formatString, args);
    if (newline) {
      emAfPrintCarriageReturn();
    }
  }
}

void emberAfPrint(uint16_t area, PGM_P formatString, ...)
{
  va_list args;
  va_start(args, formatString);
  printVarArg(area, false, formatString, args);
  va_end(args);
}

void emberAfPrintln(uint16_t area, PGM_P formatString, ...)
{
  va_list args;
  va_start(args, formatString);
  printVarArg(area, true, formatString, args);
  va_end(args);
}

static void printBuffer(uint16_t area,
                        const uint8_t *buffer,
                        uint16_t bufferLen,
                        PGM_P formatString,
                        bool reverse)
{
  if (emberAfPrintEnabled(area)) {
    uint16_t i;
    for (i = 0; i < bufferLen; i++) {
      emberAfPrint(area, formatString, buffer[reverse ? bufferLen - i - 1 : i]);
      if (i % 16 == 6) {
        emberAfFlush(area);
      }
    }
  }
}

void emberAfPrintCharacters(uint16_t area,
                            const uint8_t * str,
                            uint16_t strLen)
{
  printBuffer(area,
              str,
              strLen,
              "%c",
              false); // forward
}

void emberAfPrintBuffer(uint16_t area,
                        const uint8_t *buffer,
                        uint16_t bufferLen,
                        bool withSpace)
{
  printBuffer(area,
              buffer,
              bufferLen,
              (withSpace ? "%x " : "%x"),
              false); // forward
}

void emberAfPrintBigEndianEui64(const EmberEui64 *eui64)
{
  emberAfPrint(emberAfPrintActiveArea, ">");
  printBuffer(emberAfPrintActiveArea,
              eui64->bytes,
              EUI64_SIZE,
              "%x",
              true); // reverse
}

void emberAfPrintLittleEndianEui64(const EmberEui64 *eui64)
{
  emberAfPrint(emberAfPrintActiveArea, "<");
  printBuffer(emberAfPrintActiveArea,
              eui64->bytes,
              EUI64_SIZE,
              "%x",
              false); // forward
}

void emberAfPrintExtendedPanId(const uint8_t *extendedPanId)
{
  emberAfPrint(emberAfPrintActiveArea, ">");
  printBuffer(emberAfPrintActiveArea,
              extendedPanId,
              EXTENDED_PAN_ID_SIZE,
              "%x",
              true); // reverse
}

void emberAfPrintIpv6Address(const EmberIpv6Address *address)
{
  uint8_t dst[EMBER_IPV6_ADDRESS_STRING_SIZE];
  if (emberIpv6AddressToString(address, dst, sizeof(dst))) {
    emberAfPrint(emberAfPrintActiveArea, "%s", dst);
  }
}

void emberAfPrintIpv6Prefix(const EmberIpv6Address *prefix, uint8_t prefixBits)
{
  uint8_t dst[EMBER_IPV6_PREFIX_STRING_SIZE];
  if (emberIpv6PrefixToString(prefix, prefixBits, dst, sizeof(dst))) {
    emberAfPrint(emberAfPrintActiveArea, "%s", dst);
  }
}

void emberAfPrintZigbeeKey(const uint8_t *key)
{
  // ZigBee keys are 16 bytes.
  emberAfPrintBuffer(emberAfPrintActiveArea, key, 8, true);
  emberAfPrint(emberAfPrintActiveArea, " ");
  emberAfPrintBuffer(emberAfPrintActiveArea, key + 8, 8, true);
  emberAfPrintln(emberAfPrintActiveArea, "");
}

void emberAfFlush(uint16_t area)
{
  if (emberAfPrintEnabled(area)) {
    emAfWaitSend();
  }
}
