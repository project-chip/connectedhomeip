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
#include <credentials/FabricTable.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

void EventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        ChipLogProgress(DeviceLayer, "Receive kCommissioningComplete");

        // By default, restrict access to the wifi network management cluster's network passphrase request command
        // This restriction is automatically removed when a ReviewFabricRestrictions command is received
        // NOTE: shouldnt this be moved to the newly added CommissioningARL attribute?
        Access::AccessRestriction::Entry entry = {
            .fabricIndex    = event->CommissioningComplete.fabricIndex,
            .endpointNumber = 1,
            .clusterId      = 0x451, // wifi network management cluster id
        };

        Access::AccessRestriction::Restriction restriction = { .restrictionType =
                                                                   Access::AccessRestriction::Type::kCommandForbidden };
        restriction.id.SetValue(0); // network passphrase request
        entry.restrictions.push_back(restriction);

        Access::GetAccessControl().GetAccessRestriction()->CreateEntry(nullptr, entry, event->CommissioningComplete.fabricIndex);
    }
}

} // namespace

void ApplicationInit()
{
    DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);
}

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
