/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>

#include "LockManager.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using chip::app::DataModel::Nullable;

// The remaining emberAfPluginDoorLock* hooks are not used anymore: the
// code-driven DoorLockCluster talks to the application through
// LockApp::LockAppDoorLockDelegate instead.
void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    LockManager::Instance().InitEndpoint(endpoint);
}
