/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

namespace chip {
namespace Controller {

CHIP_ERROR NetworkRecoverBase::RecoverNetwork(NodeId remoteNodeId, RendezvousParameters params, 
                                              const WiFiCredentials & wiFiCredentials, uint64_t breadcrumb, OnNetworkRecover callback)
{
    mRemoteNodeId           = remoteNodeId;
    mNetworkType            = NetworkType::kWiFi;
    mWiFiCredentials.SetValue(wiFiCredentials);
    mBreadcrumb             = breadcrumb;
    // print ssid and password
    printf("ByteSpan: ");
    for (size_t i = 0; i < mWiFiCredentials.Value().ssid.size(); ++i)
    {
        printf("%c ", mWiFiCredentials.Value().ssid.data()[i]); // Print each byte in hexadecimal format
    }
    printf("\n");
    mOnSuccessNetworkRecoverCallback = callback;
    ChipLogDetail(Controller, "Start recovering the network");
    return mController->GetConnectedDevice(remoteNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback, params);
}

// CHIP_ERROR NetworkRecoverBase::RecoverNetwork(NodeId remoteNodeId, ByteSpan threadOperationalDataset, uint64_t breadcrumb, Callback::Callback<OnNetworkRecover> * callback)
// {
//     mRemoteNodeId               = remoteNodeId;
//     mNetworkType                = NetworkType::kThread;
//     mThreadOperationalDataset   = threadOperationalDataset;
//     mBreadcrumb                  = breadcrumb;
//     mOnSuccessNetworkRecoverCallback = callback;

//     return mController->GetConnectedDevice(remoteNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
// }

CHIP_ERROR NetworkRecoverBase::SendArmFailSafe(Messaging::ExchangeManager & exchangeMgr,
                                                        const SessionHandle & sessionHandle)
{
    GeneralCommissioning::Commands::ArmFailSafe::Type request;
    request.expiryLengthSeconds = kDefaultFailsafeTimeout;
    request.breadcrumb          = 0;

    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    return cluster.InvokeCommand(request, this, OnArmFailSafeResponse, OnCommandFailure);
}

CHIP_ERROR NetworkRecoverBase::SendRemoveNetwork(Messaging::ExchangeManager & exchangeMgr,
    const SessionHandle & sessionHandle)
{
    return CHIP_NO_ERROR;
    // const char ssid[] = "ldp";
    // auto oldssid = chip::ByteSpan(reinterpret_cast<const uint8_t*>(ssid), sizeof(ssid) - 1);
    // NetworkCommissioning::Commands::RemoveNetwork::Type request;
    // request.networkID = oldssid;

    // ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    // return cluster.InvokeCommand(request, this, OnNetworkConfigResponse, OnCommandFailure);
}

CHIP_ERROR NetworkRecoverBase::SendAddOrUpdateNetwork(Messaging::ExchangeManager & exchangeMgr,
    const SessionHandle & sessionHandle)
{
    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);
    if(mNetworkType == NetworkType::kWiFi)
    {
        NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type request;
        // print ssid and password
        printf("ByteSpan: ");
        for (size_t i = 0; i < mWiFiCredentials.Value().ssid.size(); ++i)
        {
            printf("%c ", mWiFiCredentials.Value().ssid.data()[i]); // Print each byte in hexadecimal format
        }
        printf("\n");
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

CHIP_ERROR NetworkRecoverBase::SendConnectNetwork(Messaging::ExchangeManager & exchangeMgr,
    const SessionHandle & sessionHandle)
{
    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    NetworkCommissioning::Commands::ConnectNetwork::Type request;
    if(mNetworkType == NetworkType::kWiFi)
    {
        // print ssid and password
        printf("ByteSpan: ");
        for (size_t i = 0; i < mWiFiCredentials.Value().ssid.size(); ++i)
        {
            printf("%c ", mWiFiCredentials.Value().ssid.data()[i]); // Print each byte in hexadecimal format
        }
        printf("\n");
        // const char ssid[] = "ldp";
        // auto newssid = chip::ByteSpan(reinterpret_cast<const uint8_t*>(ssid), sizeof(ssid) - 1);
        
        request.networkID = mWiFiCredentials.Value().ssid;
        request.breadcrumb.Emplace(mBreadcrumb);
    }
    else
    {
        ByteSpan extendedPanId;
        chip::Thread::OperationalDataset operationalDataset;
        if(operationalDataset.Init(mThreadOperationalDataset) != CHIP_NO_ERROR ||
           operationalDataset.GetExtendedPanIdAsByteSpan(extendedPanId) != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Unable to get extended pan ID for thread operational dataset\n");
            FinishRecoverNetwork(this, CHIP_ERROR_INVALID_ARGUMENT);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        request.networkID = extendedPanId;
        request.breadcrumb.Emplace(mBreadcrumb);
    }

    return cluster.InvokeCommand(request, this, OnConnectNetworkResponse, OnCommandFailure);
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
    case Step::kSendArmFailSafe: 
    {
        err = self->SendArmFailSafe(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendRemoveNetwork:
    {
        err = self->SendRemoveNetwork(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendAddOrUpdateNetwork: 
    {
        err = self->SendAddOrUpdateNetwork(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendConnectNetwork: 
    {
        err = self->SendConnectNetwork(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kClearCASESessions:
    {
        // TODO: clear all CASE Sessions for this device
        self->mController->ReleaseSession(self->mRemoteNodeId);
        break;
    }
    case Step::kSendCommissioningComplete: 
    {
        err = self->SendCommissioningComplete(exchangeMgr, sessionHandle);
        break;
    }
    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Current Fabric Remover failure : %" CHIP_ERROR_FORMAT, err.Format());
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
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Success Arm Fail Safe Response callback with null context. Ignoring"));

    ChipLogProgress(Controller, "Received ArmFailSafe response errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        // self->mNextStep = Step::kSendRemoveNetwork;
        self->mNextStep = Step::kSendAddOrUpdateNetwork;
        err             = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                                &self->mOnDeviceConnectionFailureCallback);
    }
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
    
}

void NetworkRecoverBase::OnNetworkConfigResponse(void * context, const NetworkCommissioning::Commands::NetworkConfigResponse::DecodableType & data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Newtork Configure callback with null context. Ignoring"));
    
    ChipLogProgress(Controller, "Received NetworkConfig response, networkingStatus=%u", to_underlying(data.networkingStatus));
    if (data.networkingStatus == NetworkCommissioning::NetworkCommissioningStatusEnum::kSuccess)
    {
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        if(self->mNextStep == Step::kSendRemoveNetwork)
        {
            self->mNextStep = Step::kSendAddOrUpdateNetwork;
        }
        else
        {
            self->mNextStep  =Step::kSendConnectNetwork;
        }
        
        err = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                                &self->mOnDeviceConnectionFailureCallback);
    }
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnConnectNetworkResponse(void * context, const NetworkCommissioning::Commands::ConnectNetworkResponse::DecodableType & data)
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
        self->mNextStep = Step::kSendCommissioningComplete;
        err             = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                                &self->mOnDeviceConnectionFailureCallback);
    }
    if (err != CHIP_NO_ERROR)
    {
        FinishRecoverNetwork(context, err);
    }
}

void NetworkRecoverBase::OnCommissioningCompleteResponse(void * context, const GeneralCommissioning::Commands::CommissioningCompleteResponse::DecodableType & data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    
    auto * self = static_cast<NetworkRecoverBase *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Success Commissioning Complete Response callback with null context. Ignoring"));

    ChipLogProgress(Controller, "Received CommissioningComplete response, errorCode=%u", to_underlying(data.errorCode));
    if (data.errorCode != GeneralCommissioning::CommissioningErrorEnum::kOk)
    {
        err = CHIP_ERROR_INTERNAL;
    }
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
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Network Recovery succeeded.");
    }
    else
    {
        ChipLogError(Controller, "Network Recovery failed : %" CHIP_ERROR_FORMAT, err.Format());
    }
    auto * self = static_cast<NetworkRecoverBase *>(context);
    if (self->mOnSuccessNetworkRecoverCallback != nullptr)
    {
        self->mOnSuccessNetworkRecoverCallback(self->mController, self->mRemoteNodeId, err);
    }
}

AutoNetworkRecover::AutoNetworkRecover(DeviceController * controller) :
    NetworkRecoverBase(controller), mOnNetworkRecoverCallback(OnNetworkRecoverHandler, this)
{}

CHIP_ERROR AutoNetworkRecover::RecoverNetwork(DeviceController * controller, NodeId remoteNodeId, RendezvousParameters params, 
                                              const WiFiCredentials & wiFiCredentials, uint64_t breadcrumb, OnNetworkRecover callback)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * remover = new (std::nothrow) AutoNetworkRecover(controller);
    if (remover == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    CHIP_ERROR err = remover->NetworkRecoverBase::RecoverNetwork(remoteNodeId, params, wiFiCredentials, breadcrumb, callback);
    if (err != CHIP_NO_ERROR)
    {
        delete remover;
    }
    // Else will clean up when the callback is called.
    return err;
}

// CHIP_ERROR AutoNetworkRecoverBase::RecoverNetwork(DeviceController * controller, NodeId remoteNodeId, chip::ByteSpan threadOperationalDataset, uint64_t breadcrumb, OnNetworkRecover callback)
// {
//     // Not using Platform::New because we want to keep our constructor private.
//     auto * remover = new (std::nothrow) AutoNetworkRecover(controller);
//     if (remover == nullptr)
//     {
//         return CHIP_ERROR_NO_MEMORY;
//     }

//     CHIP_ERROR err = remover->NetworkRecoverBase::RecoverNetwork(remoteNodeId, threadOperationalDataset, breadcrumb, callback);
//     if (err != CHIP_NO_ERROR)
//     {
//         delete remover;
//     }
//     // Else will clean up when the callback is called.
//     return err;
// }

void AutoNetworkRecover::OnNetworkRecoverHandler(void * context, NodeId remoteNodeId, CHIP_ERROR status)
{
    ChipLogDetail(Controller, "AutoNetworkRecover::OnNetworkRecoverHandler: %" CHIP_ERROR_FORMAT, status.Format());
    auto * self = static_cast<AutoNetworkRecover *>(context);
    delete self;
}
} // namespace Controller
} // namespace chip
