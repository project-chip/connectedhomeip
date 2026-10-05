/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include <EnergyEvseManager.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

namespace {

bool IsEnabledAtStartup(const chip::app::DataModel::Nullable<uint32_t> & enabledUntil)
{
    if (enabledUntil.IsNull())
    {
        return true;
    }

    if (enabledUntil.Value() == 0)
    {
        return false;
    }

    uint32_t matterEpochSeconds = 0;
    CHIP_ERROR err              = chip::System::Clock::GetClock_MatterEpochS(matterEpochSeconds);
    return err != CHIP_NO_ERROR || enabledUntil.Value() > matterEpochSeconds;
}

} // namespace

CHIP_ERROR EnergyEvseManager::LoadPersistentValues()
{

    SafeAttributePersistenceProvider * aProvider = GetSafeAttributePersistenceProvider();
    if (aProvider == nullptr)
    {
        ChipLogError(AppServer, "GetSafeAttributePersistenceProvider returned NULL");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    EndpointId aEndpointId = mDelegate->GetEndpointId();
    CHIP_ERROR err;

    if (aProvider == nullptr)
    {
        ChipLogError(AppServer, "GetSafeAttributePersistenceProvider returned NULL");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    // Restore ChargingEnabledUntil value - via Instance (which owns the data).
    // A fresh installation defaults to charging enabled indefinitely, while a
    // stored zero is an explicit Disable that must persist across a restart.
    DataModel::Nullable<uint32_t> tempChargingEnabledUntil;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::ChargingEnabledUntil::Id),
                                     tempChargingEnabledUntil);
    bool chargingEnabledUntilStored = err == CHIP_NO_ERROR;
    bool useDefaultChargingEnabled  = false;
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded ChargingEnabledUntil from NVM");
        TEMPORARY_RETURN_IGNORED SetChargingEnabledUntil(tempChargingEnabledUntil);
    }
    else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        tempChargingEnabledUntil = DataModel::NullNullable;
        ReturnErrorOnFailure(SetChargingEnabledUntil(tempChargingEnabledUntil));
        useDefaultChargingEnabled = true;
        ChipLogProgress(AppServer, "EVSE: defaulting to charging enabled indefinitely");
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted ChargingEnabledUntil value");
    }

    // Restore DischargingEnabledUntil value - via Instance (which owns the data)
    DataModel::Nullable<uint32_t> tempDischargingEnabledUntil;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::DischargingEnabledUntil::Id),
                                     tempDischargingEnabledUntil);
    bool dischargingEnabledUntilStored = err == CHIP_NO_ERROR;
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded DischargingEnabledUntil from NVM");
        TEMPORARY_RETURN_IGNORED SetDischargingEnabledUntil(tempDischargingEnabledUntil);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted DischargingEnabledUntil value");
    }

    const bool chargingEnabled =
        (chargingEnabledUntilStored || useDefaultChargingEnabled) && IsEnabledAtStartup(tempChargingEnabledUntil);
    const bool dischargingEnabled = dischargingEnabledUntilStored && IsEnabledAtStartup(tempDischargingEnabledUntil);
    if (chargingEnabled && dischargingEnabled)
    {
        ReturnErrorOnFailure(SetSupplyState(SupplyStateEnum::kEnabled));
    }
    else if (chargingEnabled)
    {
        ReturnErrorOnFailure(SetSupplyState(SupplyStateEnum::kChargingEnabled));
    }
    else if (dischargingEnabled)
    {
        ReturnErrorOnFailure(SetSupplyState(SupplyStateEnum::kDischargingEnabled));
    }

    int64_t tempMinimumChargeCurrent;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::MinimumChargeCurrent::Id),
                                     tempMinimumChargeCurrent);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded MinimumChargeCurrent from NVM");
        ReturnErrorOnFailure(SetMinimumChargeCurrent(tempMinimumChargeCurrent));
    }
    else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted MinimumChargeCurrent value");
    }

    // These keys hold the raw command limits, not the derived maximum-current
    // attributes. They are written only by the enable, disable, and expiry paths.
    int64_t maximumChargingCurrent;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::MaximumChargeCurrent::Id),
                                     maximumChargingCurrent);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(maximumChargingCurrent >= kMinimumChargeCurrentLimit, CHIP_ERROR_INVALID_ARGUMENT);
        mDelegate->mMaximumChargingCurrentLimitFromCommand = maximumChargingCurrent;
    }
    else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted charging command current limit");
    }

    int64_t maximumDischargingCurrent;
    err = aProvider->ReadScalarValue(
        ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::MaximumDischargeCurrent::Id), maximumDischargingCurrent);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(maximumDischargingCurrent >= kMinimumChargeCurrentLimit, CHIP_ERROR_INVALID_ARGUMENT);
        mDelegate->mMaximumDischargingCurrentLimitFromCommand = maximumDischargingCurrent;
    }
    else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted discharging command current limit");
    }

    // Restore UserMaximumChargeCurrent value - via Instance (which owns the data)
    int64_t tempUserMaximumChargeCurrent;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::UserMaximumChargeCurrent::Id),
                                     tempUserMaximumChargeCurrent);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded UserMaximumChargeCurrent from NVM");
        TEMPORARY_RETURN_IGNORED SetUserMaximumChargeCurrent(tempUserMaximumChargeCurrent);
    }
    else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ReturnErrorOnFailure(mDelegate->InitializeUserMaximumChargeCurrent());
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted UserMaximumChargeCurrent value");
    }

    // Restore RandomizationDelayWindow value - via Instance (which owns the data)
    uint32_t tempRandomizationDelayWindow;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::RandomizationDelayWindow::Id),
                                     tempRandomizationDelayWindow);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded RandomizationDelayWindow from NVM");
        TEMPORARY_RETURN_IGNORED SetRandomizationDelayWindow(tempRandomizationDelayWindow);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted RandomizationDelayWindow value");
    }

    // Restore ApproximateEVEfficiency value - via Instance (which owns the data)
    DataModel::Nullable<uint16_t> tempApproxEVEfficiency;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::ApproximateEVEfficiency::Id),
                                     tempApproxEVEfficiency);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded ApproximateEVEfficiency from NVM");
        TEMPORARY_RETURN_IGNORED SetApproximateEVEfficiency(tempApproxEVEfficiency);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted ApproximateEVEfficiency value");
    }

    return CHIP_NO_ERROR; // It is ok to have no value loaded here
}

CHIP_ERROR EnergyEvseManager::Init()
{
    ReturnErrorOnFailure(Instance::Init());

    // Set up the EnergyEvseTargetsStore and persistent storage delegate
    EnergyEvseDelegate * dg = GetDelegate();
    VerifyOrReturnLogError(dg != nullptr, CHIP_ERROR_UNINITIALIZED);

    EvseTargetsDelegate * targetsStore = dg->GetEvseTargetsDelegate();
    VerifyOrReturnLogError(targetsStore != nullptr, CHIP_ERROR_UNINITIALIZED);

    ReturnErrorOnFailure(targetsStore->Init(&Server::GetInstance().GetPersistentStorage()));

    return LoadPersistentValues();
}

void EnergyEvseManager::Shutdown()
{
    EnergyEvseDelegate * dg = GetDelegate();
    if (dg)
    {
        EvseTargetsDelegate * targetsStore = dg->GetEvseTargetsDelegate();
        if (targetsStore)
        {
            targetsStore->Shutdown();
        }
    }
    Instance::Shutdown();
}
