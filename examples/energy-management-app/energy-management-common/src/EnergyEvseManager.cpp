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

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

CHIP_ERROR EnergyEvseManager::LoadPersistentAttributes()
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

    // Restore ChargingEnabledUntil value
    DataModel::Nullable<uint32_t> tempChargingEnabledUntil;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::ChargingEnabledUntil::Id),
                                     tempChargingEnabledUntil);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded ChargingEnabledUntil from NVM");
        mDelegate->SetChargingEnabledUntil(tempChargingEnabledUntil);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted ChargingEnabledUntil value");
    }

    // Restore DischargingEnabledUntil value
    DataModel::Nullable<uint32_t> tempDischargingEnabledUntil;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::DischargingEnabledUntil::Id),
                                     tempDischargingEnabledUntil);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded DischargingEnabledUntil from NVM");
        mDelegate->SetDischargingEnabledUntil(tempDischargingEnabledUntil);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted DischargingEnabledUntil value");
    }

    // Restore UserMaximumChargeCurrent value
    int64_t tempUserMaximumChargeCurrent;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::UserMaximumChargeCurrent::Id),
                                     tempUserMaximumChargeCurrent);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded UserMaximumChargeCurrent from NVM");
        mDelegate->SetUserMaximumChargeCurrent(tempUserMaximumChargeCurrent);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted UserMaximumChargeCurrent value");
    }

    // Restore RandomizationDelayWindow value
    uint32_t tempRandomizationDelayWindow;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::RandomizationDelayWindow::Id),
                                     tempRandomizationDelayWindow);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded RandomizationDelayWindow from NVM");
        mDelegate->SetRandomizationDelayWindow(tempRandomizationDelayWindow);
    }
    else
    {
        ChipLogError(AppServer, "EVSE: Unable to restore persisted RandomizationDelayWindow value");
    }

    // Restore ApproximateEVEfficiency value
    DataModel::Nullable<uint16_t> tempApproxEVEfficiency;
    err = aProvider->ReadScalarValue(ConcreteAttributePath(aEndpointId, EnergyEvse::Id, Attributes::ApproximateEVEfficiency::Id),
                                     tempApproxEVEfficiency);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "EVSE: successfully loaded ApproximateEVEfficiency from NVM");
        mDelegate->SetApproximateEVEfficiency(tempApproxEVEfficiency);
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

    return LoadPersistentAttributes();
}

void EnergyEvseManager::Shutdown()
{
    Instance::Shutdown();
}
