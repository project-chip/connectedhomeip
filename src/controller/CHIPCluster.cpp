/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *  @file
 *    This file contains definitions for a base Cluster class. This class will
 *    be derived by various ZCL clusters supported by CHIP. The objects of the
 *    ZCL cluster class will be used by Controller applications to interact with
 *    the CHIP device.
 */

#include <app/InteractionModelEngine.h>
#include <controller/CHIPCluster.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Controller {

CHIP_ERROR ClusterBase::Associate(DeviceProxy * device, EndpointId endpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Check if the device supports mCluster at the requested endpoint

    mDevice   = device;
    mEndpoint = endpoint;

    return err;
}

CHIP_ERROR ClusterBase::AssociateWithGroup(DeviceProxy * device, GroupId groupId)
{
    // TODO Update this function to work in all possible conditions Issue #11850

    CHIP_ERROR err = CHIP_NO_ERROR;

    mDevice = device;
    if (mDevice->GetSecureSession().HasValue())
    {
        // Local copy to preserve original SessionHandle for future Unicast communication.
        Optional<SessionHandle> session = mDevice->GetExchangeManager()->GetSessionManager()->CreateGroupSession(
            groupId, mDevice->GetSecureSession().Value()->GetFabricIndex(), mDevice->GetDeviceId());

        // Sanity check
        if (!session.HasValue() || !session.Value()->IsGroupSession())
        {
            err = CHIP_ERROR_INCORRECT_STATE;
        }

        mGroupSession.Grab(session.Value());
    }
    else
    {
        // something fishy is going on
        err = CHIP_ERROR_INCORRECT_STATE;
    }

    // Set to 0 for now.
    mEndpoint = 0;

    return err;
}

void ClusterBase::Dissociate()
{
    mDevice = nullptr;
}

} // namespace Controller
} // namespace chip
