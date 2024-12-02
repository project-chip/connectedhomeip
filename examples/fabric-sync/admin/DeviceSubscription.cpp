/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "DeviceSubscription.h"
#include "DeviceManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;
using chip::app::ReadClient;

namespace admin {

namespace {

void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    reinterpret_cast<DeviceSubscription *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<DeviceSubscription *>(context)->OnDeviceConnectionFailure(peerId, error);
}

} // namespace

DeviceSubscription::DeviceSubscription() :
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this)
{}

void DeviceSubscription::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    VerifyOrDie(path.mEndpointId == kRootEndpointId);
    VerifyOrDie(path.mClusterId == Clusters::AdministratorCommissioning::Id);

    switch (path.mAttributeId)
    {
    case Clusters::AdministratorCommissioning::Attributes::WindowStatus::Id: {
        Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum windowStatus;
        CHIP_ERROR err = data->Get(windowStatus);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(NotSpecified, "Failed to read WindowStatus"));
        VerifyOrReturn(windowStatus != Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kUnknownEnumValue);
        mCurrentAdministratorCommissioningAttributes.windowStatus = windowStatus;
        mChangeDetected                                           = true;
        break;
    }
    case Clusters::AdministratorCommissioning::Attributes::AdminFabricIndex::Id: {
        FabricIndex fabricIndex;
        CHIP_ERROR err = data->Get(fabricIndex);
        if (err == CHIP_NO_ERROR)
        {
            mCurrentAdministratorCommissioningAttributes.openerFabricIndex = fabricIndex;
        }
        else
        {
            mCurrentAdministratorCommissioningAttributes.openerFabricIndex.reset();
        }

        mChangeDetected = true;
        break;
    }
    case Clusters::AdministratorCommissioning::Attributes::AdminVendorId::Id: {
        VendorId vendorId;
        CHIP_ERROR err = data->Get(vendorId);
        if (err == CHIP_NO_ERROR)
        {
            mCurrentAdministratorCommissioningAttributes.openerVendorId = vendorId;
        }
        else
        {
            mCurrentAdministratorCommissioningAttributes.openerVendorId.reset();
        }

        mChangeDetected = true;
        break;
    }
    default:
        break;
    }
}

void DeviceSubscription::OnReportEnd()
{
    // Report end is at the end of all attributes (success)
    if (mChangeDetected)
    {
        CHIP_ERROR err =
            bridge::FabricBridge::Instance().AdminCommissioningAttributeChanged(mCurrentAdministratorCommissioningAttributes);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Cannot forward Administrator Commissioning Attribute to fabric bridge %" CHIP_ERROR_FORMAT,
                         err.Format());
        }
        mChangeDetected = false;
    }
}

void DeviceSubscription::OnDone(ReadClient * apReadClient)
{
    // After calling mOnDoneCallback we are indicating that `this` is deleted and we shouldn't do anything else with
    // DeviceSubscription.
    MoveToState(State::AwaitingDestruction);
    mOnDoneCallback(mScopedNodeId);
}

void DeviceSubscription::OnError(CHIP_ERROR error)
{
    if (error == CHIP_ERROR_TIMEOUT && mState == State::SubscriptionStarted)
    {
        if (bridge::FabricBridge::Instance().DeviceReachableChanged(mCurrentAdministratorCommissioningAttributes.id, false) !=
            CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to update the device reachability state");
        }
    }

    ChipLogProgress(NotSpecified, "Error subscribing: %" CHIP_ERROR_FORMAT, error.Format());
}

void DeviceSubscription::OnDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    if (mState == State::Stopping)
    {
        // After calling mOnDoneCallback we are indicating that `this` is deleted and we shouldn't do anything else with
        // DeviceSubscription.
        MoveToState(State::AwaitingDestruction);
        mOnDoneCallback(mScopedNodeId);
        return;
    }
    VerifyOrDie(mState == State::Connecting);
    mClient = std::make_unique<ReadClient>(app::InteractionModelEngine::GetInstance(), &exchangeMgr /* echangeMgr */,
                                           *this /* callback */, ReadClient::InteractionType::Subscribe);
    VerifyOrDie(mClient);

    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(kRootEndpointId, Clusters::AdministratorCommissioning::Id);

    ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mMaxIntervalCeilingSeconds   = 5 * 60;

    CHIP_ERROR err = mClient->SendRequest(readParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to issue subscription to AdministratorCommissioning data");
        // After calling mOnDoneCallback we are indicating that `this` is deleted and we shouldn't do anything else with
        // DeviceSubscription.
        MoveToState(State::AwaitingDestruction);
        mOnDoneCallback(mScopedNodeId);
        return;
    }
    MoveToState(State::SubscriptionStarted);
}

void DeviceSubscription::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(NotSpecified, "DeviceSubscription moving to [%10.10s]", GetStateStr());
}

const char * DeviceSubscription::GetStateStr() const
{
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::Connecting:
        return "Connecting";

    case State::Stopping:
        return "Stopping";

    case State::SubscriptionStarted:
        return "SubscriptionStarted";

    case State::AwaitingDestruction:
        return "AwaitingDestruction";
    }
    return "N/A";
}

void DeviceSubscription::OnDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    VerifyOrDie(mState == State::Connecting || mState == State::Stopping);
    ChipLogError(NotSpecified, "DeviceSubscription failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));

    if (mState == State::Connecting)
    {
        if (bridge::FabricBridge::Instance().DeviceReachableChanged(mCurrentAdministratorCommissioningAttributes.id, false) !=
            CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to update the device reachability state");
        }
    }

    // After calling mOnDoneCallback we are indicating that `this` is deleted and we shouldn't do anything else with
    // DeviceSubscription.
    MoveToState(State::AwaitingDestruction);
    mOnDoneCallback(mScopedNodeId);
}

CHIP_ERROR DeviceSubscription::StartSubscription(OnDoneCallback onDoneCallback, Controller::DeviceController & controller,
                                                 ScopedNodeId scopedNodeId)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrDie(mState == State::Idle);
    VerifyOrReturnError(controller.GetFabricIndex() == scopedNodeId.GetFabricIndex(), CHIP_ERROR_INVALID_ARGUMENT);

    mScopedNodeId = scopedNodeId;

    mCurrentAdministratorCommissioningAttributes    = AdministratorCommissioningChanged_init_default;
    mCurrentAdministratorCommissioningAttributes.id = scopedNodeId;
    mCurrentAdministratorCommissioningAttributes.windowStatus =
        Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen;

    mOnDoneCallback = onDoneCallback;
    MoveToState(State::Connecting);
    CHIP_ERROR err =
        controller.GetConnectedDevice(scopedNodeId.GetNodeId(), &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        MoveToState(State::Idle);
    }
    return err;
}

void DeviceSubscription::StopSubscription()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrDie(mState != State::Idle);
    // Something is seriously wrong if we die on the line below
    VerifyOrDie(mState != State::AwaitingDestruction);

    if (mState == State::Stopping)
    {
        // Stop is called again while we are still waiting on connected callbacks
        return;
    }

    if (mState == State::Connecting)
    {
        MoveToState(State::Stopping);
        return;
    }

    // By calling reset on our ReadClient we terminate the subscription.
    VerifyOrDie(mClient);
    mClient.reset();
    // After calling mOnDoneCallback we are indicating that `this` is deleted and we shouldn't do anything else with
    // DeviceSubscription.
    MoveToState(State::AwaitingDestruction);
    mOnDoneCallback(mScopedNodeId);
}

} // namespace admin
