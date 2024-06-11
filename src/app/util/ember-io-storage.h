/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdint>

#include <app/util/attribute-metadata.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Compatibility {
namespace Internal {

/// A buffer guaranteed to be sized sufficiently large to contain any individual value from
/// the ember attribute/data store (i.e. a buffer that can be used to read ember data into
/// or as a temporary buffer to place data before asking ember to store it).
///
/// This buffer is intended to be used for calls to `emAfReadOrWriteAttribute` and
/// `emAfWriteAttributeExternal`: it is sufficiently sized to be able to handle any
/// max-sized data that ember is aware of.
extern MutableByteSpan gEmberAttributeIOBufferSpan;

/// Maps an attribute type that is not an integer but can be represented as an integer to the
/// corresponding basic int(8|16|32|64)(s|u) type
///
/// For example:
///    ZCL_ENUM8_ATTRIBUTE_TYPE maps to ZCL_INT8U_ATTRIBUTE_TYPE
///    ZCL_VENDOR_ID_ATTRIBUTE_TYPE maps to ZCL_INT16U_ATTRIBUTE_TYPE
///    ZCL_BITMAP32_ATTRIBUTE_TYPE maps to ZCL_INT32U_ATTRIBUTE_TYPE
///    ZCL_VOLTAGE_MV_ATTRIBUTE_TYPE maps to ZCL_INT64S_ATTRIBUTE_TYPE
///    ...
///
/// If the `type` cannot be mapped to a basic type (or is already a basic type) its value
/// is returned unchanged.
EmberAfAttributeType AttributeBaseType(EmberAfAttributeType type);

} // namespace Internal
} // namespace Compatibility
} // namespace app
} // namespace chip
