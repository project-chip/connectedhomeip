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

#include <platform/nxp/k32w/k32w0/FactoryDataProviderImpl.h>

namespace chip {
namespace DeviceLayer {

/**
 * @brief This is an example class that extends the platform factory data provider
 *        to support custom functionality. Users should implement their own custom
 *        provider based on this example.
 */

class CustomFactoryDataProvider
{
public:
    /* Custom IDs should start from at least FactoryDataId::kMaxId, which is
     * the next available valid ID. Last default ID is kMaxId - 1.
     */
    enum CustomFactoryIds
    {
        kCustomId1 = 200, // Random id that is greater than FactoryDataId::kMaxId.
        kCustomId2,
        kCustomId3,
        kCustomMaxId
    };

    CustomFactoryDataProvider();

    /* Declare here custom functions to be implemented. */
    CHIP_ERROR ParseFunctionExample();
};

} // namespace DeviceLayer
} // namespace chip
