/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTARequestorStorage.h"

namespace chip {

class PersistentStorageDelegate;

namespace DeviceLayer {

class GenericOTARequestorStorage : public OTARequestorStorage
{
public:
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

    CHIP_ERROR StoreDefaultProvider(const ProviderLocationType & provider) override;
    CHIP_ERROR ClearDefaultProvider(FabricIndex fabricIndex) override;
    CHIP_ERROR LoadDefaultProviders(ProviderLocationList & providers) override;

    CHIP_ERROR StoreCurrentProviderLocation(const ProviderLocationType & provider) override;
    CHIP_ERROR ClearCurrentProviderLocation() override;
    CHIP_ERROR LoadCurrentProviderLocation(ProviderLocationType & provider) override;

    CHIP_ERROR StoreUpdateToken(ByteSpan updateToken) override;
    CHIP_ERROR ClearUpdateToken() override;
    CHIP_ERROR LoadUpdateToken(MutableByteSpan & updateToken) override;

private:
    static constexpr size_t kMaxSerializedSize = 128u;

    CHIP_ERROR LoadProvider(const char * key, ProviderLocationType & provider);

    PersistentStorageDelegate * mPersistentStorage = nullptr;
};

} // namespace DeviceLayer
} // namespace chip
