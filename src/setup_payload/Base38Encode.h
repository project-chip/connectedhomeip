/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "Base38.h"

#include <lib/support/Span.h>

#include <stddef.h>

namespace chip {

/*
 * The out_buf is null-terminated on success.
 *
 * The resulting size of the out_buf span will be the size of data written and not including the null terminator.
 */
CHIP_ERROR base38Encode(ByteSpan in_buf, MutableCharSpan & out_buf);

/*
 * Returns size needed to store encoded string given number of input bytes including null terminator.
 */
size_t base38EncodedLength(size_t num_bytes);

} // namespace chip
