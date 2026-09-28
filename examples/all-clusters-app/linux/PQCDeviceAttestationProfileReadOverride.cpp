/*
 * Copyright (c) 2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "PQCDeviceAttestationProfileReadOverride.h"

#include <clusters/OperationalCredentials/AttributeIds.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;

CHIP_ERROR PQCDeviceAttestationProfileReadOverride::SetReadMode(CharSpan mode)
{
    if (mode.data_equal("Normal"_span))
    {
        mReadError = CHIP_NO_ERROR;
    }
    else if (mode.data_equal("UnsupportedAttribute"_span))
    {
        mReadError = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
    else if (mode.data_equal("Failure"_span))
    {
        mReadError = CHIP_IM_GLOBAL_STATUS(Failure);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PQCDeviceAttestationProfileReadOverride::Read(const app::ConcreteReadAttributePath & path,
                                                         app::AttributeValueEncoder & encoder)
{
    if (path.mEndpointId == kRootEndpointId && path.mClusterId == app::Clusters::OperationalCredentials::Id &&
        path.mAttributeId == app::Clusters::OperationalCredentials::Attributes::PQCDeviceAttestationProfile::Id)
    {
        return mReadError;
    }
    // No encoding means the codegen provider falls through to the real cluster.
    return CHIP_NO_ERROR;
}

PQCDeviceAttestationProfileReadOverride & GetPQCDeviceAttestationProfileReadOverride()
{
    static PQCDeviceAttestationProfileReadOverride instance;
    return instance;
}
