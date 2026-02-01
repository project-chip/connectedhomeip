/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "level-control-internals.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;
using chip::Protocols::InteractionModel::Status;

void ResolveOnLevel(EndpointId endpoint, const DataModel::Nullable<uint8_t> & currentLevelCache,
                    DataModel::Nullable<uint8_t> & resolvedLevel, bool & useOnLevel)
{
    Status status;
    bool containsAttribute = emberAfContainsAttribute(endpoint, LevelControl::Id, Attributes::OnLevel::Id);
    if (!containsAttribute)
    {
        resolvedLevel.SetNonNull(currentLevelCache.Value());
        useOnLevel = false;
        return;
    }

    status = Attributes::OnLevel::Get(endpoint, resolvedLevel);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "ERR: reading on level %x", to_underlying(status));
        resolvedLevel.SetNonNull(currentLevelCache.Value());
        useOnLevel = false;
        return;
    }

    if (resolvedLevel.IsNull())
    {
        // OnLevel has undefined value; fall back to CurrentLevel.
        resolvedLevel.SetNonNull(currentLevelCache.Value());
        useOnLevel = false;
    }
    else
    {
        useOnLevel = true;
    }
}
