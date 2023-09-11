/*
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file contains common headers and definitions for working with data encoded in CHIP TLV format.
 *
 *      CHIP TLV (Tag-Length-Value) is a generalized encoding method for simple structured data. It
 *      shares many properties with the commonly used JSON serialization format while being considerably
 *      more compact over the wire.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>

#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/Constants.h>

#include <stdarg.h>
#include <stdlib.h>
#include <type_traits>

/**
 * @namespace chip::TLV
 *
 * Definitions for working with data encoded in CHIP TLV format.
 *
 * CHIP TLV is a generalized encoding method for simple structured data. It shares many properties
 * with the commonly used JSON serialization format while being considerably more compact over the wire.
 */

namespace chip {
namespace TLV {

/* Forward declarations */
class TLVReader;
class TLVWriter;
class TLVUpdater;
class TLVBackingStore;

constexpr inline uint8_t operator|(TLVElementType lhs, TLVTagControl rhs)
{
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

constexpr inline uint8_t operator|(TLVTagControl lhs, TLVElementType rhs)
{
    return static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs);
}

enum
{
    kTLVControlByte_NotSpecified = 0xFFFF
};

constexpr size_t EstimateStructOverhead()
{
    // The struct itself has a control byte and an end-of-struct marker.
    return 2;
}

template <typename... FieldSizes>
constexpr size_t EstimateStructOverhead(size_t firstFieldSize, FieldSizes... otherFields)
{
    // Estimate 4 bytes of overhead per field.  This can happen for a large
    // octet string field: 1 byte control, 1 byte context tag, 2 bytes
    // length.
    return firstFieldSize + 4u + EstimateStructOverhead(otherFields...);
}

} // namespace TLV
} // namespace chip
