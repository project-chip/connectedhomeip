/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace chip {

class ProviderLocationList;

class OTARequestorStorage
{
public:
    using ProviderLocationType = app::Clusters::OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type;
    using OTAUpdateStateEnum   = app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum;

    virtual ~OTARequestorStorage() {}

    virtual CHIP_ERROR StoreDefaultProviders(const ProviderLocationList & provider) = 0;
    virtual CHIP_ERROR LoadDefaultProviders(ProviderLocationList & providers)       = 0;

    virtual CHIP_ERROR StoreCurrentProviderLocation(const ProviderLocationType & provider) = 0;
    virtual CHIP_ERROR ClearCurrentProviderLocation()                                      = 0;
    virtual CHIP_ERROR LoadCurrentProviderLocation(ProviderLocationType & provider)        = 0;

    virtual CHIP_ERROR StoreUpdateToken(ByteSpan updateToken)         = 0;
    virtual CHIP_ERROR LoadUpdateToken(MutableByteSpan & updateToken) = 0;
    virtual CHIP_ERROR ClearUpdateToken()                             = 0;

    virtual CHIP_ERROR StoreCurrentUpdateState(OTAUpdateStateEnum currentUpdateState)  = 0;
    virtual CHIP_ERROR LoadCurrentUpdateState(OTAUpdateStateEnum & currentUpdateState) = 0;
    virtual CHIP_ERROR ClearCurrentUpdateState()                                       = 0;

    virtual CHIP_ERROR StoreTargetVersion(uint32_t targetVersion)  = 0;
    virtual CHIP_ERROR LoadTargetVersion(uint32_t & targetVersion) = 0;
    virtual CHIP_ERROR ClearTargetVersion()                        = 0;
};

} // namespace chip
