/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "unit-localization-server.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitLocalization;
using namespace chip::app::Clusters::UnitLocalization::Attributes;

UnitLocalizationServer UnitLocalizationServer::mInstance;

UnitLocalizationServer & UnitLocalizationServer::Instance()
{
    return UnitLocalizationServer::mInstance;
}

CHIP_ERROR UnitLocalizationServer::ReadSupportedTemperatureUnits(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(UnitLocalizationServer::Instance().GetSupportedTemperatureUnits());
}

CHIP_ERROR UnitLocalizationServer::ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(kClusterRevision);
}

CHIP_ERROR UnitLocalizationServer::SetSupportedTemperatureUnits(DataModel::List<TempUnitEnum> & units)
{
    VerifyOrReturnError(units.size() >= kMinSupportedLocalizationUnits, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(units.size() <= kMaxSupportedLocalizationUnits, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    size_t i = 0;
    for (auto & item : units)
    {
        mUnitsBuffer[i++] = item;
    }
    mSupportedTemperatureUnits = DataModel::List<TempUnitEnum>(mUnitsBuffer, units.size());
    ChipLogProgress(Zcl, "Set number of supported temperature units to: %u",
                    static_cast<unsigned int>(mSupportedTemperatureUnits.size()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR UnitLocalizationServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == UnitLocalization::Id);

    switch (aPath.mAttributeId)
    {
    case SupportedTemperatureUnits::Id: {
        return ReadSupportedTemperatureUnits(aPath.mEndpointId, aEncoder);
    }
    case ClusterRevision::Id: {
        return ReadClusterRevision(aPath.mEndpointId, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

void MatterUnitLocalizationPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&UnitLocalizationServer::Instance());
}

void MatterUnitLocalizationPluginServerShutdownCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(&UnitLocalizationServer::Instance());
}
