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
#include "mode-select-integration.h"

#ifdef MATTER_DM_PLUGIN_MODE_SELECT

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <clusters/ModeSelect/Ids.h>

using namespace chip;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters::OnOff::Internal {

void ModeSelectSetOnModeFromStartup(EndpointId endpoint)
{
    VerifyOrReturn(emberAfContainsAttribute(endpoint, ModeSelect::Id, ModeSelect::Attributes::OnMode::Id));

    ModeSelect::Attributes::OnMode::TypeInfo::Type onMode;

    VerifyOrReturn(ModeSelect::Attributes::OnMode::Get(endpoint, onMode) == Status::Success);
    VerifyOrReturn(!onMode.IsNull());

    ChipLogProgress(Zcl, "Changing Current Mode to %x", onMode.Value());
    (void) ModeSelect::Attributes::CurrentMode::Set(endpoint, onMode.Value());
}

} // namespace chip::app::Clusters::OnOff::Internal

#endif
