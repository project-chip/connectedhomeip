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

#include <app/AppConfig.h>

#if CHIP_CONFIG_USE_STUBBED_GROUPS_CLUSTER
#include <app/clusters/groups-server/StubbedGroupsCluster.h> // nogncheck
#else
#include <app/clusters/groups-server/GroupsClusterImpl.h> // nogncheck
#endif

namespace chip::app::Clusters {

/// The Groups cluster implementation selected by the build configuration.
///
/// Groupcast supersedes the Groups cluster commands, so a build with groupcast enabled gets the
/// compatibility-only StubbedGroupsCluster and one without it gets the full implementation. Both
/// are constructed from a `GroupsCluster::Context`, so callers do not need to know which one is
/// in use.
#if CHIP_CONFIG_USE_STUBBED_GROUPS_CLUSTER
using GroupsCluster = StubbedGroupsCluster;
#else
using GroupsCluster = GroupsClusterImpl;
#endif

} // namespace chip::app::Clusters
