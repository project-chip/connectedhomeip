/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
