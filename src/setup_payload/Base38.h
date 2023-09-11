/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Shared header for encoding and decoding Base38
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <stdint.h>

namespace chip {

static const char kCodes[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                               'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.' };
static const uint8_t kBase38CharactersNeededInNBytesChunk[] = { 2, 4, 5 };
static const uint8_t kRadix                                 = ArraySize(kCodes);

} // namespace chip
