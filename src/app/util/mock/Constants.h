/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *     This file contains the constants for the mocked attribute-storage.cpp
 */

#pragma once

#include <app/ConcreteAttributePath.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>

namespace chip {
namespace Test {
inline constexpr EndpointId kMockEndpoint1   = 0xFFFE;
inline constexpr EndpointId kMockEndpoint2   = 0xFFFD;
inline constexpr EndpointId kMockEndpoint3   = 0xFFFC;
inline constexpr EndpointId kMockEndpointMin = 0xFFF1;

constexpr AttributeId MockAttributeId(const uint16_t & id)
{
    return (0xFFF1'0000 | id);
}

constexpr ClusterId MockClusterId(const uint16_t & id)
{
    // Vendor-specific cluster ids must be at least 0xFC00
    return (0xFFF1'0000 | (0xFC00 + id));
}

constexpr EventId MockEventId(const uint16_t & id)
{
    return (0xFFF1'0000 | id);
}

// Cluster Revision value returned by mock clusters
extern const uint16_t mockClusterRevision;

// Feature Map value returned by mock clusters
extern const uint32_t mockFeatureMap;

// Scalar value returned by mock clusters for MockAttributeId(1)
extern const bool mockAttribute1;

// Scalar value returned by mock clusters for MockAttributeId(2)
extern const int16_t mockAttribute2;

// Scalar value returned by mock clusters for MockAttributeId(3)
extern const uint64_t mockAttribute3;

// MockAttributeId(4) returns a list of octstr with this value
extern const uint8_t mockAttribute4[256];

} // namespace Test
} // namespace chip
