/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
constexpr EndpointId kMockEndpoint1   = 0xFFFE;
constexpr EndpointId kMockEndpoint2   = 0xFFFD;
constexpr EndpointId kMockEndpoint3   = 0xFFFC;
constexpr EndpointId kMockEndpointMin = 0xFFF1;

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
} // namespace Test
} // namespace chip
