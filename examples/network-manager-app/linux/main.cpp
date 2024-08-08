/*
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/clusters/thread-network-directory-server/thread-network-directory-server.h>
#include <app/clusters/wifi-network-management-server/wifi-network-management-server.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

ByteSpan ByteSpanFromCharSpan(CharSpan span)
{
    return ByteSpan(Uint8::from_const_char(span.data()), span.size());
}

std::optional<DefaultThreadNetworkDirectoryServer> gThreadNetworkDirectoryServer;
void emberAfThreadNetworkDirectoryClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gThreadNetworkDirectoryServer);
    gThreadNetworkDirectoryServer.emplace(endpoint).Init();
}

std::optional<WiFiNetworkManagementServer> gWiFiNetworkManagementServer;
void emberAfWiFiNetworkManagementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gWiFiNetworkManagementServer);
    gWiFiNetworkManagementServer.emplace(endpoint).Init();
}

void ApplicationInit()
{
    gWiFiNetworkManagementServer->SetNetworkCredentials(ByteSpanFromCharSpan("MatterAP"_span),
                                                        ByteSpanFromCharSpan("Setec Astronomy"_span));
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrReturnValue(ChipLinuxAppInit(argc, argv) == 0, -1);
    ChipLinuxAppMainLoop();
    return 0;
}
