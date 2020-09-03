/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef EMBER_PRINT_H
#define EMBER_PRINT_H 1

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Prints a log message
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
 * @param length - Length of buffer
 * @param withSpace - Pass in true if a space should be printed between each byte.
 * */
void emberAfPrintBuffer(int category, const uint8_t * buffer, uint16_t length, bool withSpace);

/**
 * @brief Prints a strings
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress.
 * @param string - Buffer to print as a string
 * */
void emberAfPrintString(int category, const uint8_t * string);
#endif // EMBER_PRINT
