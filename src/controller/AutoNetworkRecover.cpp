/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <controller/AutoNetworkRecover.h>

#include <app-common/zap-generated/cluster-objects.h>

using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

CHIP_ERROR NetworkRecoverBase::RecoverNetwork(NodeId remoteNodeId, Transport::PeerAddress & addr,
                                              const WiFiCredentials & wiFiCredentials, uint64_t breadcrumb,
                                              chip::Callback::Callback<OnNetworkRecover> * callback)
{
    mRemoteNodeId           = remoteNodeId;
    mNetworkType            = NetworkType::kWiFi;
    mBreadcrumb             = breadcrumb;
    mNetworkRecoverCallback = callback;
    mWiFiCredentials.SetValue(wiFiCredentials);
    return mController->GetConnectedDevice(remoteNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback, addr);
}

CHIP_ERROR NetworkRecoverBase::SendArmFailSafe(uint16_t timeout, Messaging::ExchangeManager & exchangeMgr,
                                               const SessionHandle & sessionHandle)
{
    GeneralCommissioning::Commands::ArmFailSafe::Type request;
    request.expiryLengthSeconds = timeout;
    request.breadcrumb          = 0;

    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    return cluster.InvokeCommand(request, this, OnArmFailSafeResponse, OnCommandFailure);
}

CHIP_ERROR NetworkRecoverBase::ReadLastNetworkID(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    using TypeInfo = NetworkCommissioning::Attributes::LastNetworkID::TypeInfo;

    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    return cluster.ReadAttribute<TypeInfo>(this, OnSuccessReadLastNetworkID, OnReadAttributeFailure);
}

CHIP_ERROR NetworkRecoverBase::SendRemoveNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    NetworkCommissioning::Commands::RemoveNetwork::Type request;

    request.networkID = mLastNetworkID;

    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    return cluster.InvokeCommand(request, this, OnNetworkConfigResponse, OnCommandFailure);
}

CHIP_ERROR NetworkRecoverBase::SendAddOrUpdateNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);
    if (mNetworkType == NetworkType::kWiFi)
    {
        NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type request;

        request.ssid        = mWiFiCredentials.Value().ssid;
        request.credentials = mWiFiCredentials.Value().credentials;
        request.breadcrumb.Emplace(mBreadcrumb);

        return cluster.InvokeCommand(request, this, OnNetworkConfigResponse, OnCommandFailure);
    }
    else
    {
        NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Type request;
        request.operationalDataset = mThreadOperationalDataset;
        request.breadcrumb.Emplace(mBreadcrumb);

        return cluster.InvokeCommand(request, this, OnNetworkConfigResponse, OnCommandFailure);
    }
}

CHIP_ERROR NetworkRecoverBase::SendConnectNetwork(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    NetworkCommissioning::Commands::ConnectNetwork::Type request;
    if (mNetworkType == NetworkType::kWiFi)
    {
        request.networkID = mWiFiCredentials.Value().ssid;
        request.breadcrumb.Emplace(mBreadcrumb);
    }
    else
    {
        // TODO for thread network recovery
    }

    return cluster.InvokeCommand(request, this, OnConnectNetworkResponse, OnCommandFailure);
}

CHIP_ERROR NetworkRecoverBase::ReleaseSessions(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    CHIP_ERROR err = mController->ExpireAllSessions(mRemoteNodeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to release session: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        mNextStep = Step::kSendCommissioningComplete;
        err = mController->GetConnectedDevice(mRemoteNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    }
    return err;
}

CHIP_ERROR NetworkRecoverBase::SendCommissioningComplete(Messaging::ExchangeManager & exchangeMgr,
                                                         const SessionHandle & sessionHandle)
{
    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    GeneralCommissioning::Commands::CommissioningComplete::Type request;

    return cluster.InvokeCommand(request, this, OnCommissioningCompleteResponse, OnCommandFailure);
}

void NetworkRecoverBase::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                             const SessionHandle & sessionHandle)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto * self    = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));
    ChipLogDetail(Controller, "device conncet callback, next step = %d", to_underlying(self->mNextStep));
    switch (self->mNextStep)
    {
    case Step::kSendArmFailSafe: {
        err = self->SendArmFailSafe(kDefaultFailsafeTimeout, exchangeMgr, sessionHandle);
        break;
    }
    case Step::kReadLastNetworkID: {
        err = self->ReadLastNetworkID(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendRemoveNetwork: {
        err = self->SendRemoveNetwork(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendAddOrUpdateNetwork: {
        err = self->SendAddOrUpdateNetwork(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendConnectNetwork: {
        err = self->SendConnectNetwork(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kReleaseSessions: {
        // clean up the case session then re-establish in operational network
        err = self->ReleaseSessions(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendCommissioningComplete: {
        err = self->SendCommissioningComplete(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendDisArmFailSafe: {
        err = self->SendArmFailSafe(0, exchangeMgr, sessionHandle);
        break;
    }
    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Network Recovery failure : %" CHIP_ERROR_FORMAT, err.Format());
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn: %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected failure callback with null context. Ignoring"));

    FinishRecoverNetwork(context, err);
}

void NetworkRecoverBase::OnArmFailSafeResponse(void * context,
                                               const GeneralCommissioning::Commands::ArmFailSafeResponse::DecodableType & data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Arm Fail Safe Response callback with null context. Ignoring"));

    ChipLogProgress(Controller, "Received ArmFailSafe response errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    else if (self->mNextStep == Step::kSendDisArmFailSafe)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        self->mNextStep = Step::kReadLastNetworkID;
        err             = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                                &self->mOnDeviceConnectionFailureCallback);
    }
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnSuccessReadLastNetworkID(void * context,
                                                    const chip::app::DataModel::Nullable<chip::ByteSpan> & networkID)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Read Last Network ID callback with null context. Ignoring"));

    if (networkID.IsNull())
    {
        self->mNextStep = Step::kSendAddOrUpdateNetwork;
    }
    else
    {
        self->mLastNetworkID = networkID.Value();
        self->mNextStep      = Step::kSendRemoveNetwork;
    }
    err = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                &self->mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnNetworkConfigResponse(void * context,
                                                 const NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Success Newtork Configure callback with null context. Ignoring"));

    ChipLogProgress(Controller, "Received NetworkConfig response, networkingStatus=%u", to_underlying(data.networkingStatus));
    if (data.networkingStatus != NetworkCommissioning::NetworkCommissioningStatusEnum::kSuccess)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        if (self->mNextStep == Step::kSendRemoveNetwork)
        {
            self->mNextStep = Step::kSendAddOrUpdateNetwork;
        }
        else
        {
            self->mNextStep = Step::kSendConnectNetwork;
        }

        err = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                    &self->mOnDeviceConnectionFailureCallback);
    }
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnConnectNetworkResponse(
    void * context, const NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Success Connect Network callback with null context. Ignoring"));

    ChipLogProgress(Controller, "Received ConnectNetwork response, networkingStatus=%u", to_underlying(data.networkingStatus));
    if (data.networkingStatus != NetworkCommissioning::NetworkCommissioningStatusEnum::kSuccess)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        self->mNextStep = Step::kReleaseSessions;
        err             = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                                &self->mOnDeviceConnectionFailureCallback);
    }
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnCommissioningCompleteResponse(
    void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Commissioning Complete Response callback with null context. Ignoring"));

    ChipLogProgress(Controller, "Received CommissioningComplete response, errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    FinishRecoverNetwork(context, err);
}

void NetworkRecoverBase::OnReadAttributeFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnReadAttributeFailure %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Read Attribute failure callback with null context. Ignoring"));

    FinishRecoverNetwork(context, err);
}

void NetworkRecoverBase::OnCommandFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnCommandFailure %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Send command failure callback with null context. Ignoring"));

    FinishRecoverNetwork(context, err);
}

void NetworkRecoverBase::FinishRecoverNetwork(void * context, CHIP_ERROR err)
{
    auto * self = static_cast<NetworkRecoverBase *>(context);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Network Recovery succeeded.");
    }
    else
    {
        ChipLogError(Controller, "Network Recovery failed : %" CHIP_ERROR_FORMAT, err.Format());
        if (self->mNextStep != Step::kSendDisArmFailSafe && self->mNextStep != Step::kSendCommissioningComplete)
        {
            // disarm
            self->mNextStep = Step::kSendDisArmFailSafe;
            self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                  &self->mOnDeviceConnectionFailureCallback);
            return;
        }
    }
    if (self->mNetworkRecoverCallback != nullptr)
    {
        self->mNetworkRecoverCallback->mCall(self->mNetworkRecoverCallback->mContext, self->mRemoteNodeId, err);
    }
}

AutoNetworkRecover::AutoNetworkRecover(DeviceController * controller) :
    NetworkRecoverBase(controller), mOnNetworkRecoverCompleteCallback(OnNetworkRecoverComplete, this)
{}

CHIP_ERROR AutoNetworkRecover::RecoverNetwork(NetworkRecover * recover, NodeId remoteNodeId, Transport::PeerAddress & addr,
                                              const WiFiCredentials & wiFiCredentials, uint64_t breadcrumb)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * autoRecover = new (std::nothrow) AutoNetworkRecover(recover->GetCommissioner());
    if (autoRecover == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    autoRecover->mNetworkRecover = recover;

    CHIP_ERROR err = autoRecover->NetworkRecoverBase::RecoverNetwork(remoteNodeId, addr, wiFiCredentials, breadcrumb,
                                                                     &autoRecover->mOnNetworkRecoverCompleteCallback);
    if (err != CHIP_NO_ERROR)
    {
        delete autoRecover;
    }
    // Else will clean up when the callback is called.
    return err;
}

void AutoNetworkRecover::OnNetworkRecoverComplete(void * context, NodeId remoteNodeId, CHIP_ERROR status)
{
    auto * self = static_cast<AutoNetworkRecover *>(context);
    self->mNetworkRecover->NetworkRecoverComplete(remoteNodeId, status);
    delete self;
}
} // namespace Controller
} // namespace chip
