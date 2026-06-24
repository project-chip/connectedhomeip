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

#pragma once

#include <access/AccessControl.h>
#include <app/server-cluster/ServerClusterContext.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccessControl {

/**
 * @brief Emits the AuxiliaryAccessUpdated event for the Access Control cluster.
 *
 * This function is used by clusters to emit AuxiliaryAccessUpdated events. This event needs
 * to be emitted whenever there is a change to auxiliary ACL entries on the access control cluster.
 * This can be caused from operations in a variety of clusters (groups, groupcast, etc.).
 * This function helps to separate the event logic from the access control cluster directly so it
 * can be called wherever needed.
 *
 * @param eventsGenerator The generator used to emit the event.
 * @param subjectDescriptor The subject descriptor associated with the change.
 */
void EmitAuxiliaryAccessUpdated(DataModel::EventsGenerator & eventsGenerator,
                                const chip::Access::SubjectDescriptor & subjectDescriptor);

} // namespace AccessControl
} // namespace Clusters
} // namespace app
} // namespace chip
