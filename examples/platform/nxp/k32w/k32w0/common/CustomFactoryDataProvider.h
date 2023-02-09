/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "K32W0FactoryDataProvider.h"

namespace chip {
namespace DeviceLayer {

/**
 * @brief This is an example class that extends the platform factory data provider
 *        to support custom functionality. Users should implement their own custom
 *        provider based on this example.
 */

class CustomFactoryDataProvider : public K32W0FactoryDataProvider
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
