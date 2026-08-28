/**
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 * Defines used by endpoint_config.h that may be useful when manually creating
 * endpoint configurations (e.g. for dynamic endpoints).
 */
#define ZAP_TYPE(type) ZCL_##type##_ATTRIBUTE_TYPE
#ifndef __cplusplus
#error "Matter endpoint configuration headers require C++"
#endif

namespace chip {
namespace app {

/**
 * Sentinel placeholder whose address is stored in EmberAfDefaultOrMinMaxAttributeValue::ptrToDefaultValue
 * when ZAP_EMPTY_DEFAULT() is used.
 *
 * This serves as a marker indicating that no explicit default value was configured for the attribute.
 * It allows emberAfGetAttributeDefaultValue to distinguish between "no default provided" (ZAP_EMPTY_DEFAULT())
 * and an explicit scalar default of zero (ZAP_SIMPLE_DEFAULT(0)).
 *
 * When an attribute has an empty default (HasEmptyDefault() is true), emberAfGetAttributeDefaultValue
 * returns an empty ByteSpan, allowing nullable attributes to default to Null and non-nullable attributes
 * to default to zero.
 */
extern const uint8_t sZapEmptyDefaultSentinel;

} // namespace app
} // namespace chip

#define ZAP_EMPTY_DEFAULT()                                                                                                        \
    {                                                                                                                              \
        &chip::app::sZapEmptyDefaultSentinel                                                                                       \
    }
#define ZAP_SIMPLE_DEFAULT(x)                                                                                                      \
    {                                                                                                                              \
        (uint32_t) x                                                                                                               \
    }
#define ZAP_ATTRIBUTE_MASK(mask) MATTER_ATTRIBUTE_FLAG_##mask
#define ZAP_CLUSTER_MASK(bit) MATTER_CLUSTER_FLAG_##bit
