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

#include <app/clusters/identify-server/IdentifyIntegrationDelegate.h>
#include <credentials/GroupDataProvider.h>

namespace chip::scenes {

// Forward declared rather than included: this header is used by StubbedGroupsCluster, which must
// not pull in (and link) the scenes cluster implementation.
class ScenesIntegrationDelegate;

} // namespace chip::scenes

namespace chip::app::Clusters {

/// Construction arguments common to both Groups cluster implementation.
///
/// The shape is identical across implementations so that callers can build a context without
/// knowing which one `GroupsCluster` resolves to. StubbedGroupsCluster only uses
/// `groupDataProvider`; the other members are for implementations that support the commands
/// requiring them.
struct GroupsClusterContext
{
    Credentials::GroupDataProvider & groupDataProvider;
    scenes::ScenesIntegrationDelegate * scenesIntegration = nullptr; // if null, no scenes support
    IdentifyIntegrationDelegate * identifyIntegration     = nullptr; // if null, no identify support
};

} // namespace chip::app::Clusters
