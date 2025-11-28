/**
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
#include "mode-base-integration.h"

#ifdef MATTER_DM_PLUGIN_MODE_BASE

// nogncheck because the gn dependency checker does not understand
// conditional includes, so will fail in an application that has an On/Off
// cluster but no ModeBase-derived cluster.
#include <app/clusters/mode-base-server/mode-base-cluster-objects.h> // nogncheck
#include <app/clusters/mode-base-server/mode-base-server.h>          // nogncheck

using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::OnOff::Internal::ModeBase {

void UpdateCurrentModeToOnMode(chip::EndpointId endpoint)
{
    for (auto & modeBaseInstance : Clusters::ModeBase::GetModeBaseInstanceList())
    {
        if (modeBaseInstance.GetEndpointId() != endpoint)
        {
            continue;
        }

        if (!modeBaseInstance.HasFeature(Clusters::ModeBase::Feature::kOnOff))
        {
            continue;
        }
        Clusters::ModeBase::Attributes::OnMode::TypeInfo::Type onMode = modeBaseInstance.GetOnMode();
        if (onMode.IsNull())
        {
            continue;
        }

        Status status = modeBaseInstance.UpdateCurrentMode(onMode.Value());
        if (status == Status::Success)
        {
            ChipLogProgress(Zcl, "Changed the Current Mode to %x", onMode.Value());
        }
        else
        {
            ChipLogError(Zcl, "Failed to Changed the Current Mode to %x: %u", onMode.Value(), to_underlying(status));
        }
    }
}

} // namespace chip::app::Clusters::OnOff::Internal::ModeBase

#endif // MATTER_DM_PLUGIN_MODE_BASE
