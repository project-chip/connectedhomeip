/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <AppMain.h>

#include "CPAppCommandDelegate.h"
#include "NamedPipeCommands.h"
#include "commissioning-proxy-delegate-impl.h"

#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Server.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstring>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

NamedPipeCommands sChipNamedPipeCommands;
CPAppCommandDelegate sCPAppCommandDelegate;
CommissioningProxy::MyCPDelegate gMyCPDelegate;

BitMask<CommissioningProxy::Feature> gFeatures(CommissioningProxy::Feature::kBackgroundScan);

RegisteredServerCluster<CommissioningProxy::CommissioningProxyCluster>
    gCPCluster(CommissioningProxy::CommissioningProxyCluster::Config(CommissioningProxyEndpoint, gFeatures, gMyCPDelegate));
} // namespace

void ApplicationInit()
{
    std::string path = std::string(LinuxDeviceOptions::GetInstance().app_pipe);
    if ((!path.empty()) and (sChipNamedPipeCommands.Start(path, &sCPAppCommandDelegate) != CHIP_NO_ERROR))
    {
        ChipLogError(NotSpecified, "Failed to start CHIP NamedPipeCommands");
        TEMPORARY_RETURN_IGNORED sChipNamedPipeCommands.Stop();
    }

    // Register the Commissioning Proxy Code Driven mechanism
    VerifyOrDie(CodegenDataModelProvider::Instance().Registry().Register(gCPCluster.Registration()) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "%s(): Main function is Proxy Commissioner on endpoint %u", __func__, CommissioningProxyEndpoint);
}

void ApplicationShutdown()
{
    if (sChipNamedPipeCommands.Stop() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to stop CHIP NamedPipeCommands");
    }
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }
    ChipLinuxAppMainLoop();
    return 0;
}
