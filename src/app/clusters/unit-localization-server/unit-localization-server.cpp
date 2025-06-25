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
#include <app/SafeAttributePersistenceProvider.h>
#include <app/reporting/reporting.h>
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

CHIP_ERROR UnitLocalizationServer::Init()
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint8_t storedTempUnit = 0;

    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(kRootEndpointId, UnitLocalization::Id, TemperatureUnit::Id), storedTempUnit);
    if (err == CHIP_NO_ERROR)
    {
        mTemperatureUnit = static_cast<TempUnitEnum>(storedTempUnit);
        ChipLogDetail(Zcl, "UnitLocalization ep0 Loaded TemperatureUnit: %u", storedTempUnit);
    }
    else
    {
        ChipLogDetail(Zcl, "UnitLocalization ep0 set default TemperatureUnit: %u", to_underlying(mTemperatureUnit));
    }
    return err;
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

CHIP_ERROR UnitLocalizationServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    if (aPath.mClusterId != UnitLocalization::Id)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case TemperatureUnit::Id: {
        TempUnitEnum newTempUnit = TempUnitEnum::kCelsius;
        ReturnErrorOnFailure(aDecoder.Decode(newTempUnit));
        ReturnErrorOnFailure(SetTemperatureUnit(newTempUnit));
        return CHIP_NO_ERROR;
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR UnitLocalizationServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != UnitLocalization::Id)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case TemperatureUnit::Id: {
        return aEncoder.Encode(mTemperatureUnit);
    }
    case SupportedTemperatureUnits::Id: {
        return aEncoder.Encode(GetSupportedTemperatureUnits());
    }
    case ClusterRevision::Id: {
        return aEncoder.Encode(kClusterRevision);
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR UnitLocalizationServer::SetTemperatureUnit(TempUnitEnum newTempUnit)
{
    bool isValid       = false;
    const auto & units = GetSupportedTemperatureUnits();
    for (auto const & unit : units)
    {
        if (unit == newTempUnit)
        {
            isValid = true;
            break;
        }
    }
    VerifyOrReturnError(isValid, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnValue(mTemperatureUnit != newTempUnit, CHIP_NO_ERROR);
    mTemperatureUnit = newTempUnit;
    MatterReportingAttributeChangeCallback(kRootEndpointId, UnitLocalization::Id, TemperatureUnit::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(
        ConcreteAttributePath(kRootEndpointId, UnitLocalization::Id, TemperatureUnit::Id), to_underlying(mTemperatureUnit)));
    return CHIP_NO_ERROR;
}

void MatterUnitLocalizationPluginServerInitCallback()
{
    UnitLocalizationServer::Instance().Init();
    AttributeAccessInterfaceRegistry::Instance().Register(&UnitLocalizationServer::Instance());
}

void MatterUnitLocalizationPluginServerShutdownCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(&UnitLocalizationServer::Instance());
}
