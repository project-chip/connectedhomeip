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
