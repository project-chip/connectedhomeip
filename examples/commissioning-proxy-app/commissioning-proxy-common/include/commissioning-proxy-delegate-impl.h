/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// The following are the application-specific endpoints - it seems like this
// should be available from endpoint_config.h
const chip::EndpointId CommissioningProxyEndpoint = 1;

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

class MyCPDelegate : public chip::app::Clusters::CommissioningProxy::Delegate
{
public:
    void SetServer(CommissioningProxyCluster * server) override { mServer = server; }
    CommissioningProxyCluster * GetServer() const override { return mServer; }

protected:
    CommissioningProxyCluster & Server()
    {
        VerifyOrDie(mServer != nullptr);
        return *mServer;
    }

    const CommissioningProxyCluster & Server() const
    {
        VerifyOrDie(mServer != nullptr);
        return *mServer;
    }

    Protocols::InteractionModel::Status ProxyScanRequest(
        chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport, 
        chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
        chip::app::CommandHandler * commandObj, 
        const DataModel::InvokeRequest & request) override;

    Protocols::InteractionModel::Status ProxyConnectRequest(
                    DataModel::Nullable<chip::ByteSpan> address,
                    CapabilitiesBitmap transport,
                    uint16_t discriminator,
                    chip::VendorId vendorid,
                    uint16_t productid,
                    WiFiBandBitmap wiFiBand,
                    app::CommandHandler * commandObj,
                    const DataModel::InvokeRequest & request) override;

    uint8_t GetScanMaxTime() override;
    void SetScanMaxTime(uint8_t seconds) override;

private:
    CommissioningProxyCluster * mServer = nullptr;
};

} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
