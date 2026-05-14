/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app/ConcreteAttributePath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/util/af-types.h> // For EmberAfAttributeType
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {

struct AttributeDecoderParams
{
    const ConcreteAttributePath & path;
    ServerClusterInterface & cluster;
    EmberAfAttributeType emberType;
    uint16_t emberSize;
    bool isNullable = false;
};

/// Reads the attribute specified by path from the given cluster and decodes it into the provided buffer in Ember format.
///
/// Supports fixed-size integer/float types as well as strings/octet strings.
/// Specifically DOES NOT support:
///   - Lists (ZCL_ARRAY_ATTRIBUTE_TYPE)
///   - Structs (ZCL_STRUCT_ATTRIBUTE_TYPE)
///
/// Intended use is as a bridge between ServerClusterInterface data and
/// ember encoded data (hence the above limitations: ember has no direct encoding
/// for lists/structs).
///
/// @note The `outBuffer` is used as scratch space for intermediate TLV encoding. It must be large enough to hold the
///       full TLV representation of the attribute (including framing overhead like structure and array tags), not just
///       the final Ember representation. For small attributes, the TLV representation will be significantly larger than
///       the Ember representation.
CHIP_ERROR DecodeAttributeToEmberBuffer(const AttributeDecoderParams & params, MutableByteSpan & outBuffer);

} // namespace app
} // namespace chip
