/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AccessControlEventHelper.h"
#include <clusters/AccessControl/Events.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {

void EmitAuxiliaryAccessUpdated(DataModel::EventsGenerator & eventsGenerator,
                                const chip::Access::SubjectDescriptor & subjectDescriptor)
{
    Events::AuxiliaryAccessUpdated::Type event;
    event.fabricIndex = subjectDescriptor.fabricIndex;
    if (subjectDescriptor.subject != kUndefinedNodeId)
    {
        event.adminNodeID.SetNonNull(subjectDescriptor.subject);
    }
    else
    {
        event.adminNodeID.SetNull();
    }

    (void) eventsGenerator.GenerateEvent(event, kRootEndpointId);
}

} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
