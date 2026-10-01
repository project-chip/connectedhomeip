/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppTask.h"
#include "BoltLockManager.h"

#include <lib/core/DataModelTypes.h>

#include <zephyr/logging/log.h>

using namespace ::chip;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

// The code-driven DoorLock cluster gets its configuration (features, user
// capacities, auto relock time) from the server-config overrides installed by
// AppTask::Init before the server starts, so the legacy attribute writes and
// the MatterPostAttributeChangeCallback handling are gone. Only the initial
// cluster state sync remains.
void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    AppTask::Instance().UpdateClusterState(BoltLockMgr().GetState(), BoltLockManager::OperationSource::kUnspecified);
}
