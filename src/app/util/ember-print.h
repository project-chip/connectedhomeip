/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/debug-printing.h>
#include <lib/support/logging/CHIPLogging.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Prints a log message
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress
 * @param format - Format string to print
 */
#if CHIP_PROGRESS_LOGGING
void emberAfPrint(int category, const char * format, ...) ENFORCE_FORMAT(2, 3);
#else
#define emberAfPrint(...) ((void) 0)
#endif

/**
 * @brief Prints a log followed by new line line
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress
 * @param format - Format string to print
 */
#if CHIP_PW_TOKENIZER_LOGGING
#define emberAfPrintln(MOD, MSG, ...) ChipLogProgress(Zcl, MSG, __VA_ARGS__);
#elif CHIP_PROGRESS_LOGGING
void emberAfPrintln(int category, const char * format, ...) ENFORCE_FORMAT(2, 3);
#else
#define emberAfPrintln(...) ((void) 0)
#endif

/**
 * @brief Prints a buffer
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress.
 * @param buffer - Buffer to print.
 * @param length - Length of buffer
 * @param withSpace - Pass in true if a space should be printed between each byte.
 */
#if CHIP_PROGRESS_LOGGING
void emberAfPrintBuffer(int category, const uint8_t * buffer, uint16_t length, bool withSpace);
#else
#define emberAfPrintBuffer(...) ((void) 0)
#endif

/**
 * @brief Prints a strings
 * @param category - Currently ignored as zcl categories do not map to chip categories. Defaults to kLogCategory_Progress.
 * @param string - Buffer to print as a string
 */
#if CHIP_PROGRESS_LOGGING
void emberAfPrintString(int category, const uint8_t * string);
#else
#define emberAfPrintString(...) ((void) 0)
#endif
