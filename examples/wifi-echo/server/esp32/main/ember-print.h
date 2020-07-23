#ifndef EMBER_PRINT
#define EMBER_PRINT 1

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Prints a log
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress
 * @param format - Format string to print
 * */
void emberAfPrint(int category, const char * format, ...);

/**
 * @brief Prints a log followed by new line line
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress
 * @param format - Format string to print
 * */
void emberAfPrintln(int category, const char * format, ...);

/**
 * @brief Prints a buffer
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress.
 * @param buffer - Buffer to print.
 * @param length - Format string to print.
 * @param withSpace - Pass in true if a space should be printed between each byte.
 * */
void emberAfPrintBuffer(int category, const uint8_t * buffer, uint16_t length, bool withSpace);

#endif // EMBER_PRINT
