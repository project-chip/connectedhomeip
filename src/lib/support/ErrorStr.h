/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines functions to translate error codes and status reports
 *      used throughout the CHIP package into human-readable strings.
 *
 */

#pragma once

#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>

namespace chip {

/**
 *  @def CHIP_NO_ERROR_STRING
 *
 *  @brief
 *    This defines the CHIP error string for success or no error.
 *
 */
#define CHIP_NO_ERROR_STRING "Success"

struct ErrorFormatter
{
    typedef bool (*FormatFunct)(char * buf, uint16_t bufSize, CHIP_ERROR err);

    FormatFunct FormatError;
    ErrorFormatter * Next;
};

extern const char * ErrorStr(CHIP_ERROR err);
extern void RegisterErrorFormatter(ErrorFormatter * errFormatter);
extern void DeregisterErrorFormatter(ErrorFormatter * errFormatter);
extern void FormatError(char * buf, uint16_t bufSize, const char * subsys, CHIP_ERROR err, const char * desc);

extern const char * StatusReportStr(uint32_t profileId, uint16_t statusCode);

} // namespace chip
