/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <platform/nxp/k32w/k32w0/FactoryDataProviderImpl.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief This is an example class that extends the platform factory data provider
 *        to support custom functionality. Users should implement their own custom
 *        provider based on this example.
 */

class CustomFactoryDataProvider : public FactoryDataProviderImpl
{
public:
    /* Custom IDs should start from FactoryDataId::kMaxId, which is
     * the next available valid ID. Last default ID is kMaxId - 1.
     */
    enum CustomFactoryIds
    {
        kCustomId1 = FactoryDataId::kMaxId,
        kCustomId2,
        kCustomId3,
        kCustomMaxId
    };

    static CustomFactoryDataProvider & GetDefaultInstance();

    CustomFactoryDataProvider() {}

    /* SetCustomIds() must be implemented in order to define custom IDs. */
    CHIP_ERROR SetCustomIds() override;

    /* Declare here custom functions to be implemented. */
    CHIP_ERROR ParseFunctionExample();
};

} // namespace DeviceLayer
} // namespace chip
