/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <variant>

#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

#include <lib/support/ReadOnlyBuffer.h>

namespace chip::app::OOBDataSerializer {

static constexpr uint8_t kTagEndpointId  = 1;
static constexpr uint8_t kTagClusterId   = 2;
static constexpr uint8_t kTagAttributeId = 3;
static constexpr uint8_t kTagValue       = 4;

struct AttributeRequest
{
    ConcreteDataAttributePath path;
    chip::TLV::TLVReader value;
};

/**
 * @brief Parses a unified TLV buffer containing an out-of-band "SetAttribute" request.
 *
 * @note This parser expects a strict, sequential binary layout. It does not loop or support out-of-order elements.
 * The incoming TLV buffer MUST contain exactly four sequential elements in the following strict order:
 *
 * Structure
 * ├── Context Tag 1: EndpointId (uint16_t)
 * ├── Context Tag 2: ClusterId (uint32_t)
 * ├── Context Tag 3: AttributeId (uint32_t)
 * └── Context Tag 4: Raw, unwrapped Attribute Value (MUST be the last element to allow direct parsing).
 */
std::variant<CHIP_ERROR, AttributeRequest> ParseAttributeRequest(ByteSpan tlvBuffer);

/**
 * @brief Dynamically allocates and builds a unified, out-of-band "SetAttribute" request TLV buffer.
 *
 * This helper handles dynamic memory allocation based on the actual size of the attribute value element,
 * serializes the metadata, copies the value, and returns the completed package as a ReadOnlyBuffer.
 *
 * The built request will have the following serialized TLV format:
 * Structure
 * ├── Context Tag 1: EndpointId (uint16_t)
 * ├── Context Tag 2: ClusterId (uint32_t)
 * ├── Context Tag 3: AttributeId (uint32_t)
 * └── Context Tag 4: Attribute Value (any valid TLV element/container representing the new value)
 *
 * @param[in]     path                  The concrete data attribute path containing Endpoint, Cluster, and Attribute IDs.
 * @param[in]     attributeValueReader  A TLVReader that MUST be positioned exactly on the data element of the attribute's value
 *                                      to be copied.
 *
 * @return        A variant containing either a CHIP_ERROR on failure or the finalized ReadOnlyBuffer on success.
 */
std::variant<CHIP_ERROR, ReadOnlyBuffer<uint8_t>> BuildSetAttributeRequest(const ConcreteDataAttributePath & path,
                                                                           const chip::TLV::TLVReader & attributeValueReader);

} // namespace chip::app::OOBDataSerializer
