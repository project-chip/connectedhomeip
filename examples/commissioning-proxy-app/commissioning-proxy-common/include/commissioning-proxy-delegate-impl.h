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

#pragma once

#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class MyCPDelegate : public chip::app::Clusters::CommissioningProxy::Delegate
{
public:
    Protocols::InteractionModel::Status ProxyScanRequest(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport, 
        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
        chip::app::CommandHandler * commandObj, 
        const DataModel::InvokeRequest & request) override;


    uint8_t GetScanMaxTime() override;
    void SetScanMaxTime(uint8_t seconds) override;

private:
    //CommissioningProxyCluster * mServerLogic = nullptr;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
