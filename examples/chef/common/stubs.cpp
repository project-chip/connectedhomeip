/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BridgedActions::Attributes;

// ************************************
// Door Lock Stubs
// ************************************

using namespace ::chip;
using namespace chip::app::Clusters::DoorLock;

bool emberAfPluginDoorLockOnDoorLockCommand(EndpointId endpointId, const chip::Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    return true;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(EndpointId endpointId, const chip::Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    return true;
}

// ************************************
// Bridge Stubs
// ************************************

void MatterBridgedActionsPluginServerInitCallback(void) {}
