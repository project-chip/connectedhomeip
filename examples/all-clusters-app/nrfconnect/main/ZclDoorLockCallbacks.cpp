/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <lib/support/CodeUtils.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::app::Clusters::DoorLock;

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

// Provided some empty callbacks and replaced feature map
// to simulate DoorLock endpoint for All-Clusters-App example
// without using kUsersManagement|kAccessSchedules|kRFIDCredentials|kPINCredentials

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    return true;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    return true;
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    DoorLockServer::Instance().InitServer(endpoint);

    // Set FeatureMap to 0, default is:
    // (kUsersManagement|kAccessSchedules|kRFIDCredentials|kPINCredentials) 0x113
    EmberAfStatus status = DoorLock::Attributes::FeatureMap::Set(endpoint, 0);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating feature map %x", status);
    }
}
