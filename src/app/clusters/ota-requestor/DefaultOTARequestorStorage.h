/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "OTARequestorStorage.h"

namespace chip {

class PersistentStorageDelegate;

class DefaultOTARequestorStorage : public OTARequestorStorage
{
public:
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

    CHIP_ERROR StoreDefaultProviders(const ProviderLocationList & provider) override;
    CHIP_ERROR LoadDefaultProviders(ProviderLocationList & providers) override;

    CHIP_ERROR StoreCurrentProviderLocation(const ProviderLocationType & provider) override;
    CHIP_ERROR ClearCurrentProviderLocation() override;
    CHIP_ERROR LoadCurrentProviderLocation(ProviderLocationType & provider) override;

    CHIP_ERROR StoreUpdateToken(ByteSpan updateToken) override;
    CHIP_ERROR ClearUpdateToken() override;
    CHIP_ERROR LoadUpdateToken(MutableByteSpan & updateToken) override;

    CHIP_ERROR StoreCurrentUpdateState(OTAUpdateStateEnum currentUpdateState) override;
    CHIP_ERROR LoadCurrentUpdateState(OTAUpdateStateEnum & currentUpdateState) override;
    CHIP_ERROR ClearCurrentUpdateState() override;

    CHIP_ERROR StoreTargetVersion(uint32_t targetVersion) override;
    CHIP_ERROR LoadTargetVersion(uint32_t & targetVersion) override;
    CHIP_ERROR ClearTargetVersion() override;

private:
    CHIP_ERROR Load(const char * key, MutableByteSpan & buffer);
    PersistentStorageDelegate * mPersistentStorage = nullptr;
};

} // namespace chip
