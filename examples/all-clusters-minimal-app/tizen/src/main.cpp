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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/util/af.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

#include <TizenServiceAppMain.h>
#include <binding-handler.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

// Network commissioning
namespace {
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

NetworkCommissioning::TizenEthernetDriver sEthernetDriver;
Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(kNetworkCommissioningEndpointMain, &sEthernetDriver);
} // namespace

void ApplicationInit()
{
    // Enable secondary endpoint only when we need it.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    sEthernetNetworkCommissioningInstance.Init();
}

void ApplicationShutdown(){};

int main(int argc, char * argv[])
{
    TizenServiceAppMain app;
    VerifyOrDie(app.Init(argc, argv) == 0);

    VerifyOrDie(InitBindingHandlers() == CHIP_NO_ERROR);

    return app.RunMainLoop();
}
