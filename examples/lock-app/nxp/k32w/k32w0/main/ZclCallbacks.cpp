/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/logging/CHIPLogging.h>

#include "AppTask.h"
#include "BoltLockManager.h"
#include <platform/CHIPDeviceLayer.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <lib/core/DataModelTypes.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DoorLock;
using ::chip::app::DataModel::Nullable;

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    return BoltLockMgr().InitiateAction(0, BoltLockManager::LOCK_ACTION);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return BoltLockMgr().InitiateAction(0, BoltLockManager::UNLOCK_ACTION);
}
