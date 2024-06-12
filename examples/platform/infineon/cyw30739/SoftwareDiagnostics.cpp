/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "SoftwareDiagnostics.h"

#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Infineon {
namespace CYW30739 {

using namespace chip::app::Clusters;

void OnSoftwareFaultEventHandler(const char * faultRecordString)
{
#ifdef MATTER_DM_PLUGIN_SOFTWARE_DIAGNOSTICS_SERVER
    SoftwareDiagnostics::Events::SoftwareFault::Type softwareFault;

    /* Unable to access thread ID in the application layer. */
    softwareFault.id = 0;

    if (DeviceLayer::PlatformMgrImpl().IsCurrentTask())
    {
        softwareFault.name.SetValue("Matter"_span);
    }
    else if (DeviceLayer::ThreadStackMgrImpl().IsCurrentTask())
    {
        softwareFault.name.SetValue("Thread"_span);
    }
    else
    {
        softwareFault.name.SetValue("App"_span);
    }

    softwareFault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(faultRecordString), strlen(faultRecordString)));

    SoftwareDiagnosticsServer::Instance().OnSoftwareFaultDetect(softwareFault);
#endif // MATTER_DM_PLUGIN_SOFTWARE_DIAGNOSTICS_SERVER
}

} // namespace CYW30739
} // namespace Infineon
} // namespace DeviceLayer
} // namespace chip
