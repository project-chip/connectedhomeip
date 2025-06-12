/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/server/Server.h>

namespace chip {
namespace app {
namespace Clusters {

class AdministratorCommissioningLogic
{
public:
    AdministratorCommissioning::CommissioningWindowStatusEnum GetWindowStatus()
    {
        return Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatusForCluster();
    }

    const app::DataModel::Nullable<FabricIndex> & GetOpenerFabricIndex()
    {
        return Server::GetInstance().GetCommissioningWindowManager().GetOpenerFabricIndex();
    }

    const app::DataModel::Nullable<VendorId> & GetAdminVendorId()
    {
        return Server::GetInstance().GetCommissioningWindowManager().GetOpenerVendorId();
    }

    // Methods to handle the various commands this cluster may receive.
    DataModel::ActionReturnStatus
    OpenCommissioningWindow(FabricIndex fabricIndex,
                            const AdministratorCommissioning::Commands::OpenCommissioningWindow::DecodableType & commandData);

    DataModel::ActionReturnStatus OpenBasicCommissioningWindow(
        FabricIndex fabricIndex,
        const AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::DecodableType & commandData);

    DataModel::ActionReturnStatus
    RevokeCommissioning(const AdministratorCommissioning::Commands::RevokeCommissioning::DecodableType & commandData);
};

} // namespace Clusters
} // namespace app
} // namespace chip
