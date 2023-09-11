/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <lib/support/Span.h>

#pragma once

namespace chip {

/**
 * @brief Create ZCL string (pascal string) from char string.
 * The maximum size of the ZCL string is defined by the kBufferMaximumSize.
 * In case char string exceeds the maximum of size ZCL string, the string will be 0-sized
 * and the method will return an error.
 */
extern CHIP_ERROR MakeZclCharString(MutableByteSpan & buffer, const char * cString);

} // namespace chip
