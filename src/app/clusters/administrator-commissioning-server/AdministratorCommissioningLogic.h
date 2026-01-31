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

#include <app/FailSafeContext.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/server/CommissioningWindowManager.h>
#include <clusters/AdministratorCommissioning/Commands.h>
#include <clusters/AdministratorCommissioning/Enums.h>
#include <credentials/FabricTable.h>

namespace chip {
namespace app {
namespace Clusters {

class AdministratorCommissioningLogic
{
public:
    struct Context
    {
        CommissioningWindowManager & commissioningWindowManager;
        FabricTable & fabricTable;
        /**
         * The FailSafeContext used by the Administrator Commissioning Cluster.
         * * IMPORTANT: This MUST be the same FailSafeContext instance used by the
         * provided commissioningWindowManager. In the standard Server implementation,
         * both the Manager and this Context should retrieve this from
         * Server::GetInstance().GetFailSafeContext().
         */
        FailSafeContext & failSafeContext;
    };

    constexpr AdministratorCommissioningLogic(Context context) : mContext(context) {}

    AdministratorCommissioning::CommissioningWindowStatusEnum GetWindowStatus()
    {
        return mContext.commissioningWindowManager.CommissioningWindowStatusForCluster();
    }

    const app::DataModel::Nullable<FabricIndex> & GetAdminFabricIndex()
    {
        return mContext.commissioningWindowManager.GetOpenerFabricIndex();
    }

    const app::DataModel::Nullable<VendorId> & GetAdminVendorId()
    {
        return mContext.commissioningWindowManager.GetOpenerVendorId();
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

private:
    Context mContext;
};

} // namespace Clusters
} // namespace app
} // namespace chip
