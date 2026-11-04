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
#include <SafeAttributePersistenceAdapter.h>
#include <app/server/Server.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

CHIP_ERROR EnergyEvseManager::LoadPersistentValues()
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mDelegate->LoadPersistentValues();
}

CHIP_ERROR EnergyEvseManager::Init()
{
    ReturnErrorOnFailure(Instance::Init());

    // The cluster is now registered, so every failure below must call Shutdown() to unregister it,
    // as the caller destroys this object on failure.

    EnergyEvseDelegate * dg = GetDelegate();
    if (dg == nullptr)
    {
        ChipLogError(AppServer, "EVSE: no delegate");
        Shutdown();
        return CHIP_ERROR_UNINITIALIZED;
    }
    dg->SetInstance(this);
    dg->SetAttributeStorage(GetSafeAttributePersistenceAdapter());

    EvseTargetsDelegate * targetsStore = dg->GetEvseTargetsDelegate();
    if (targetsStore == nullptr)
    {
        ChipLogError(AppServer, "EVSE: no targets delegate");
        Shutdown();
        return CHIP_ERROR_UNINITIALIZED;
    }

    ReturnErrorOnFailure(targetsStore->Init(&Server::GetInstance().GetPersistentStorage(), Server::GetInstance().GetFabricTable()),
                         Shutdown());
    ReturnErrorOnFailure(LoadPersistentValues(), Shutdown());

    return CHIP_NO_ERROR;
}

void EnergyEvseManager::Shutdown()
{
    EnergyEvseDelegate * dg = GetDelegate();
    if (dg)
    {
        // A pending enable deadline timer would otherwise fire with no instance linked
        dg->CancelActiveTimers();
        dg->SetInstance(nullptr);
        dg->ClearAttributeStorage();
        EvseTargetsDelegate * targetsStore = dg->GetEvseTargetsDelegate();
        if (targetsStore)
        {
            targetsStore->Shutdown();
        }
    }
    Instance::Shutdown();
}
