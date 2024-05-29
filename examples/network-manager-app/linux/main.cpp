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
#include <app/clusters/wifi-network-management-server/wifi-network-management-server.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/Span.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void ApplicationInit() {}
void ApplicationShutdown() {}

ByteSpan ByteSpanFromCharSpan(CharSpan span)
{
    return ByteSpan(Uint8::from_const_char(span.data()), span.size());
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    WiFiNetworkManagementServer::Instance().SetNetworkCredentials(ByteSpanFromCharSpan("MatterAP"_span),
                                                                  ByteSpanFromCharSpan("Setec Astronomy"_span));

    ChipLinuxAppMainLoop();
    return 0;
}
