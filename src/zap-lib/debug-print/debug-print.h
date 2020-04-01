/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include PLATFORM_HEADER
#include EMBER_AF_API_STACK

/** @brief Indicate if a printing area is enabled. */
bool emberAfPrintEnabled(uint16_t area);

/** @brief Enable a printing area. */
void emberAfPrintOn(uint16_t userArea);

/** @brief Disable a printing area. */
void emberAfPrintOff(uint16_t userArea);

/** @brief Enable all printing areas. */
void emberAfPrintAllOn(void);

/** @brief Disable all printing areas. */
void emberAfPrintAllOff(void);

/** @brief Print the status of the printing areas. */
void emberAfPrintStatus(void);

/** @brief Print a formatted message. */
void emberAfPrint(uint16_t area, PGM_P formatString, ...);

/** @brief Print a formatted message followed by a newline. */
void emberAfPrintln(uint16_t area, PGM_P formatString, ...);

/** @brief Print a buffer as a series of bytes in hexidecimal format. */
void emberAfPrintBuffer(uint16_t area,
                        const uint8_t *buffer,
                        uint16_t bufferLen,
                        bool withSpace);

/** @brief Print regular C string. */
void emberAfPrintCharacters(uint16_t area,
                            const uint8_t * str,
                            uint16_t strLen);

/** @brief Print an EUI64 (IEEE address) in big-endian format. */
void emberAfPrintBigEndianEui64(const EmberEui64 *eui64);

/** @brief Print an EUI64 (IEEE address) in little-endian format. */
void emberAfPrintLittleEndianEui64(const EmberEui64 *eui64);

/** @brief Print an extended PAN identifier. */
void emberAfPrintExtendedPanId(const uint8_t *extendedPanId);

/** @brief Print an IPv6 address. */
void emberAfPrintIpv6Address(const EmberIpv6Address *address);

/** @brief Print an IPv6 prefix. */
void emberAfPrintIpv6Prefix(const EmberIpv6Address *prefix, uint8_t prefixBits);

/** @brief Print a 16-byte ZigBee key. */
void emberAfPrintZigbeeKey(const uint8_t *key);

/** @brief Wait for all data currently queued to be transmitted. */
void emberAfFlush(uint16_t area);

extern uint16_t emberAfPrintActiveArea;

#endif // DEBUG_PRINT_H
