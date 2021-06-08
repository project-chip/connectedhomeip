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

#include "network-commissioning.h"

#include <cstring>
#include <type_traits>

#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/common/gen/enums.h>
#include <app/util/af.h>
#include <gen/callback.h>

#include <app/CommandPathParams.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/DeviceControlServer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

// Include DeviceNetworkProvisioningDelegateImpl for WiFi provisioning.
// TODO: Enable wifi network should be done by ConnectivityManager. (Or other platform neutral interfaces)
#if defined(CHIP_DEVICE_LAYER_TARGET)
#define DEVICENETWORKPROVISIONING_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/DeviceNetworkProvisioningDelegateImpl.h>
#include DEVICENETWORKPROVISIONING_HEADER
#endif

// TODO: Configuration should move to build-time configuration
#ifndef CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS
#define CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS 4
#endif // CHIP_CLUSTER_NETWORK_COMMISSIONING_MAX_NETWORKS

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::clusters;
using namespace ::chip::app::clusters::NetworkCommissioning;
using namespace ::chip::app::clusters::NetworkCommissioning::Internal;

namespace chip {
namespace app {
namespace clusters {
namespace NetworkCommissioning {

// Internal structs and helper functions for network commissioning.
namespace Internal {

enum class NetworkType : uint8_t
{
    kUndefined = 0,
    kWiFi      = 1,
    kThread    = 2,
    kEthernet  = 3,
};

struct WiFiNetworkInfo
{
    uint8_t mSSID[kMaxWiFiSSIDLen + 1];
    uint8_t mSSIDLen;
    uint8_t mCredentials[kMaxWiFiCredentialsLen];
    uint8_t mCredentialsLen;
};

struct NetworkInfo
{
    uint8_t mNetworkID[kMaxNetworkIDLen];
    uint8_t mNetworkIDLen;
    uint8_t mEnabled;
    NetworkType mNetworkType;
    union NetworkData
    {
        Thread::OperationalDataset mThread;
        WiFiNetworkInfo mWiFi;
    } mData;
};

enum class ClusterState : uint8_t
{
    kIdle            = 0,
    kEnablingNetwork = 1,
};

} // namespace Internal

namespace {
// The internal network info containing credentials. Need to find some better place to save these info.
NetworkInfo sNetworks[kMaxNetworks];
} // namespace

namespace Internal {
constexpr uint8_t kInvalidNetworkSeq = 0xFF;

// void EncodeAndSendEnableNetworkResponse(CHIP_ERROR err);
// void SetEnablingNetworkSeq(uint8_t seq);
// ClusterState GetClusterState();
// CHIP_ERROR __attribute__((warn_unused_result)) MoveClusterState(ClusterState newState);
// void OnNetworkEnableStatusChangeCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
// void SetCommissionerSecureSessionHandle(const SecureSessionHandle & handle);

NetworkInfo * GetNetworkBySeq(uint8_t seq);

chip::NodeId networkCommissionerNodeId = chip::kAnyNodeId;
ClusterState sClusterState             = ClusterState::kIdle;
uint8_t sCurrentEnablingNetworkSeq     = kInvalidNetworkSeq;
SecureSessionHandle sCommissionerSecureSessionHandle;

NetworkInfo * GetNetworkBySeq(uint8_t seq)
{
    return seq >= kMaxNetworks ? nullptr : &sNetworks[seq];
}

void SetEnablingNetworkSeq(uint8_t seq)
{
    sCurrentEnablingNetworkSeq = seq;
}

ClusterState GetClusterState()
{
    return sClusterState;
}

void SetCommissionerSecureSessionHandle(const SecureSessionHandle & secureSessionHandle)
{
    sCommissionerSecureSessionHandle = secureSessionHandle;
}

void EncodeAndSendEnableNetworkResponse(CHIP_ERROR err)
{
    CHIP_ERROR processError                  = CHIP_NO_ERROR;
    chip::app::CommandSender * commandSender = nullptr;
    EmberAfNetworkCommissioningError commissioningError =
        (err == CHIP_NO_ERROR ? EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS
                              : EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR);
    NodeId commissionerNodeId              = sCommissionerSecureSessionHandle.GetPeerNodeId();
    Transport::AdminId commissionerAdminId = sCommissionerSecureSessionHandle.GetAdminId();

    // From the Network Commissioning cluster spec, we MAY put response command in seperate exchange, and may have a StatusCode
    // before the ResponseCommand.
    app::CommandPathParams cmdParams = { 0, /* group id */ 0, ZCL_NETWORK_COMMISSIONING_CLUSTER_ID,
                                         ZCL_ENABLE_NETWORK_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;
    SuccessOrExit(processError = chip::app::InteractionModelEngine::GetInstance()->NewCommandSender(&commandSender));
    SuccessOrExit(processError = commandSender->PrepareCommand(&cmdParams));
    writer = commandSender->GetCommandDataElementTLVWriter();
    SuccessOrExit(processError = writer->Put(TLV::ContextTag(0), static_cast<uint32_t>(commissioningError)));
    SuccessOrExit(processError = writer->PutString(TLV::ContextTag(1), ""));
    SuccessOrExit(processError = commandSender->FinishCommand());
    SuccessOrExit(processError = commandSender->SendCommandRequest(commissionerNodeId, commissionerAdminId));

    // Interaction model engine will manage the state of command sender when successfully send this command, so we can safely delete
    // this.
    commandSender = nullptr;

exit:
    if (commandSender != nullptr)
    {
        commandSender->Shutdown();
    }
    ChipLogError(Zcl, "Failed to send response command to commissioner: %d", processError);
}

CHIP_ERROR __attribute__((warn_unused_result)) MoveClusterState(ClusterState newState)
{
    if (newState != ClusterState::kIdle)
    {
        // We can transist to some working state only when we are current idle since we does not allow multiple network
        // commissioning cluster transactions in parallel.
        if (sClusterState != ClusterState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }
    else if (newState == ClusterState::kIdle && sClusterState == ClusterState::kIdle)
    {
        ChipLogError(Zcl, "NetworkCommissioning -- Moving Idle to Idle -- Programming error!");
        assert(false);
        // Should not reach this, but it should be OK if we really move from kIdle to kIdle.
        return CHIP_ERROR_INCORRECT_STATE;
    }
    else if (sClusterState != ClusterState::kIdle)
    {
        switch (sClusterState)
        {
        case ClusterState::kEnablingNetwork: {
            SetEnablingNetworkSeq(kInvalidNetworkSeq);
            SetCommissionerSecureSessionHandle(SecureSessionHandle());
        }
        break;
        default:
            ChipLogError(Zcl, "NetworkCommissioning -- Unexpected current state!");
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    // We can only move a Idle state to a working state, or moving from a working state to a idle state, we have checked those cases
    // above, so we can safely set cluster state to the new state now.
    sClusterState = newState;
    return CHIP_NO_ERROR;
}

void OnNetworkEnableStatusChangeCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    NetworkInfo * networkInfo = nullptr;
    CHIP_ERROR err            = CHIP_NO_ERROR;
    // Skip if we received network state change but not during network commissioning.
    VerifyOrReturn(sClusterState == ClusterState::kEnablingNetwork);
    VerifyOrReturn((networkInfo = GetNetworkBySeq(sCurrentEnablingNetworkSeq)) != nullptr);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (networkInfo->mNetworkType == NetworkType::kThread && event->Type == chip::DeviceLayer::DeviceEventType::kThreadStateChange)
    {
        if (DeviceLayer::ThreadStackMgr().IsThreadAttached())
        {
            ChipLogError(Zcl, "OnNetworkEnableStatusChangeCallback -- Connected to Thread");
            EncodeAndSendEnableNetworkResponse(CHIP_NO_ERROR);
            err = MoveClusterState(ClusterState::kIdle);
        }
    }
#endif
    if (networkInfo->mNetworkType == NetworkType::kWiFi &&
        event->Type == chip::DeviceLayer::DeviceEventType::kInternetConnectivityChange &&
        (event->InternetConnectivityChange.IPv4 == chip::DeviceLayer::ConnectivityChange::kConnectivity_Established ||
         event->InternetConnectivityChange.IPv6 == chip::DeviceLayer::ConnectivityChange::kConnectivity_Established))
    {
        ChipLogError(Zcl, "OnNetworkEnableStatusChangeCallback -- Connected to WiFi");
        EncodeAndSendEnableNetworkResponse(CHIP_NO_ERROR);
        err = MoveClusterState(ClusterState::kIdle);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OnNetworkEnableStatusChangeCallback error: %d", err);
    }
}

} // namespace Internal

// Actual command handlers.

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
void OnAddThreadNetworkCommandCallbackInternal(app::Command * apCommandObj, EndpointId endpointId, ByteSpan operationalDataset,
                                               uint64_t breadcrumb, uint32_t timeoutMs)
{
    CHIP_ERROR encodingError             = CHIP_NO_ERROR;
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;

    if (GetClusterState() != ClusterState::kIdle)
    {
        err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
    }
    else
    {
        for (size_t i = 0; i < kMaxNetworks; i++)
        {
            if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
            {
                Thread::OperationalDataset & dataset = sNetworks[i].mData.mThread;
                CHIP_ERROR error                     = dataset.Init(operationalDataset);

                if (error != CHIP_NO_ERROR)
                {
                    ChipLogDetail(Zcl, "Failed to parse Thread operational dataset: %s", ErrorStr(error));
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
                    break;
                }

                uint8_t extendedPanId[Thread::kSizeExtendedPanId];

                static_assert(sizeof(sNetworks[i].mNetworkID) >= sizeof(extendedPanId),
                              "Network ID must be larger than Thread extended PAN ID!");
                if ((error = dataset.GetExtendedPanId(extendedPanId)) != CHIP_NO_ERROR)
                {
                    ChipLogDetail(Zcl, "Failed to get Thread extpanid: %s", ErrorStr(error));
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
                    break;
                }
                memcpy(sNetworks[i].mNetworkID, extendedPanId, sizeof(extendedPanId));
                sNetworks[i].mNetworkIDLen = sizeof(extendedPanId);

                sNetworks[i].mNetworkType = NetworkType::kThread;
                sNetworks[i].mEnabled     = false;

                err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
                break;
            }
        }
    }

    TLV::TLVWriter * writer          = nullptr;
    app::CommandPathParams cmdParams = { /* endpoint id */ endpointId, /* group id */ 0, ZCL_NETWORK_COMMISSIONING_CLUSTER_ID,
                                         ZCL_ADD_THREAD_NETWORK_RESPONSE_COMMAND_ID,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };
    SuccessOrExit(encodingError = apCommandObj->PrepareCommand(&cmdParams));
    VerifyOrExit((writer = apCommandObj->GetCommandDataElementTLVWriter()) != nullptr, encodingError = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(encodingError = writer->Put(TLV::ContextTag(0), static_cast<uint32_t>(err)));
    SuccessOrExit(encodingError = writer->PutString(TLV::ContextTag(1), ""));
    SuccessOrExit(encodingError = apCommandObj->FinishCommand());
exit:
    if (encodingError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command for AddThreadNetwork: %d (Command Error: %d)", encodingError, err);
    }
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
void OnAddWiFiNetworkCommandCallbackInternal(app::Command * apCommandObj, EndpointId endpointId, ByteSpan ssid,
                                             ByteSpan credentials, uint64_t breadcrumb, uint32_t timeoutMs)
{
    CHIP_ERROR encodingError             = CHIP_NO_ERROR;
    EmberAfNetworkCommissioningError err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_BOUNDS_EXCEEDED;
    if (GetClusterState() != ClusterState::kIdle)
    {
        err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_UNKNOWN_ERROR;
    }
    else
    {
        for (size_t i = 0; i < kMaxNetworks; i++)
        {
            if (sNetworks[i].mNetworkType == NetworkType::kUndefined)
            {
                if (!(ssid.size() <= sizeof(sNetworks[i].mData.mWiFi.mSSID)))
                {
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE;
                    break;
                }
                memcpy(sNetworks[i].mData.mWiFi.mSSID, ssid.data(), ssid.size());

                using WiFiSSIDLenType = decltype(sNetworks[i].mData.mWiFi.mSSIDLen);
                if (!chip::CanCastTo<WiFiSSIDLenType>(ssid.size()))
                {
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE;
                    break;
                }
                sNetworks[i].mData.mWiFi.mSSIDLen = static_cast<WiFiSSIDLenType>(ssid.size());

                if (!(credentials.size() <= sizeof(sNetworks[i].mData.mWiFi.mCredentials)))
                {
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE;
                    break;
                }
                memcpy(sNetworks[i].mData.mWiFi.mCredentials, credentials.data(), credentials.size());

                using WiFiCredentialsLenType = decltype(sNetworks[i].mData.mWiFi.mCredentialsLen);
                if (!chip::CanCastTo<WiFiCredentialsLenType>(ssid.size()))
                {
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE;
                    break;
                }
                sNetworks[i].mData.mWiFi.mCredentialsLen = static_cast<WiFiCredentialsLenType>(credentials.size());

                if (!(ssid.size() <= sizeof(sNetworks[i].mNetworkID)))
                {
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE;
                    break;
                }
                memcpy(sNetworks[i].mNetworkID, sNetworks[i].mData.mWiFi.mSSID, ssid.size());

                using NetworkIDLenType = decltype(sNetworks[i].mNetworkIDLen);
                if (!chip::CanCastTo<NetworkIDLenType>(ssid.size()))
                {
                    err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_OUT_OF_RANGE;
                    break;
                }
                sNetworks[i].mNetworkIDLen = static_cast<NetworkIDLenType>(ssid.size());

                sNetworks[i].mNetworkType = NetworkType::kWiFi;
                sNetworks[i].mEnabled     = false;

                err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
                ChipLogDetail(Zcl, "WiFi provisioning data: SSID: %s", ssid);
                break;
            }
        }
    }

    TLV::TLVWriter * writer          = nullptr;
    app::CommandPathParams cmdParams = { /* endpoint id */ endpointId, /* group id */ 0, ZCL_NETWORK_COMMISSIONING_CLUSTER_ID,
                                         ZCL_ADD_WI_FI_NETWORK_RESPONSE_COMMAND_ID,
                                         (chip::app::CommandPathFlags::kEndpointIdValid) };

    SuccessOrExit(encodingError = apCommandObj->PrepareCommand(&cmdParams));
    VerifyOrExit((writer = apCommandObj->GetCommandDataElementTLVWriter()) != nullptr, encodingError = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(encodingError = writer->Put(TLV::ContextTag(0), static_cast<uint32_t>(err)));
    SuccessOrExit(encodingError = writer->PutString(TLV::ContextTag(1), ""));
    SuccessOrExit(encodingError = apCommandObj->FinishCommand());

exit:
    if (encodingError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command for AddWiFiNetwork: %d (Command Error: %d)", encodingError, err);
    }
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

namespace {
void DoEnableNetwork(intptr_t networkPtr)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    NetworkInfo * network = reinterpret_cast<NetworkInfo *>(networkPtr);
    switch (network->mNetworkType)
    {
    case NetworkType::kThread:
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
// TODO: On linux, we are using Reset() instead of Detach() to disable thread network, which is not expected.
// Upstream issue: https://github.com/openthread/ot-br-posix/issues/755
#if !CHIP_DEVICE_LAYER_TARGET_LINUX
        SuccessOrExit(err = DeviceLayer::ThreadStackMgr().SetThreadEnabled(false));
#endif
        SuccessOrExit(err = DeviceLayer::ThreadStackMgr().SetThreadProvision(network->mData.mThread.AsByteSpan()));
        SuccessOrExit(err = DeviceLayer::ThreadStackMgr().SetThreadEnabled(true));
#else
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
#endif
        break;
    case NetworkType::kWiFi:
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    {
        // TODO: Currently, DeviceNetworkProvisioningDelegateImpl assumes that ssid and credentials are null terminated strings,
        // which is not correct, this should be changed once we have better method for commissioning wifi networks.
        DeviceLayer::DeviceNetworkProvisioningDelegateImpl deviceDelegate;
        SuccessOrExit(err = deviceDelegate.ProvisionWiFi(reinterpret_cast<const char *>(network->mData.mWiFi.mSSID),
                                                         reinterpret_cast<const char *>(network->mData.mWiFi.mCredentials)));
    }
#else
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
#endif
    break;
    case NetworkType::kEthernet:
    case NetworkType::kUndefined:
    default:
        ExitNow(err = CHIP_ERROR_NOT_IMPLEMENTED);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        EncodeAndSendEnableNetworkResponse(err);
        err = MoveClusterState(ClusterState::kIdle);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Network Commissioning Cluster -- Failed to move to Idle state: %d", err);
        }
    }
    else
    {
        network->mEnabled = true;
    }
}
} // namespace

void OnEnableNetworkCommandCallbackInternal(app::Command * apCommandObj, EndpointId endpointId, ByteSpan networkID,
                                            uint64_t breadcrumb, uint32_t timeoutMs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t networkSeq;
    EmberAfNetworkCommissioningError emberError = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_NETWORK_ID_NOT_FOUND;

    // TODO(cecille): This is very dangerous - need to check against real netif name, ensure no password.
    constexpr char ethernetNetifMagicCode[] = "ETH0";
    if (networkID.size() == sizeof(ethernetNetifMagicCode) &&
        memcmp(networkID.data(), ethernetNetifMagicCode, networkID.size()) == 0)
    {
        ChipLogProgress(Zcl, "Wired network enabling requested. Assuming success.");
        ExitNow(emberError = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS);
    }

    SuccessOrExit(err = MoveClusterState(ClusterState::kEnablingNetwork));
    if (apCommandObj != nullptr)
    {
        SetCommissionerSecureSessionHandle(apCommandObj->GetExchangeContext()->GetSecureSession());
    }

    for (networkSeq = 0; networkSeq < kMaxNetworks; networkSeq++)
    {
        if (sNetworks[networkSeq].mNetworkIDLen == networkID.size() &&
            sNetworks[networkSeq].mNetworkType != NetworkType::kUndefined &&
            memcmp(sNetworks[networkSeq].mNetworkID, networkID.data(), networkID.size()) == 0)
        {
            // TODO: Currently, we cannot figure out the detailed error from network provisioning on DeviceLayer, we should
            // implement this in device layer.
            SetEnablingNetworkSeq(networkSeq);
            DeviceLayer::PlatformMgr().ScheduleWork(DoEnableNetwork, reinterpret_cast<intptr_t>(&sNetworks[networkSeq]));
            err = EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS;
            break;
        }
    }

    // When DoEnableNetwork success, there is a background task waiting
    if (emberError != EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS)
    {
        SuccessOrExit(err = MoveClusterState(ClusterState::kIdle));
        TLV::TLVWriter * writer          = nullptr;
        app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, ZCL_NETWORK_COMMISSIONING_CLUSTER_ID,
                                             ZCL_ENABLE_NETWORK_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        SuccessOrExit(err = apCommandObj->PrepareCommand(&cmdParams));
        VerifyOrExit((writer = apCommandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), static_cast<uint32_t>(err)));
        SuccessOrExit(err = writer->PutString(TLV::ContextTag(1), ""));
        SuccessOrExit(err = apCommandObj->FinishCommand());
    }
    else
    {
        chip::app::CommandPathParams returnStatusParam = { endpointId,
                                                           0, // GroupId
                                                           ZCL_NETWORK_COMMISSIONING_CLUSTER_ID, ZCL_ENABLE_NETWORK_COMMAND_ID,
                                                           (chip::app::CommandPathFlags::kEndpointIdValid) };

        SuccessOrExit(err = apCommandObj->AddStatusCode(&returnStatusParam,
                                                        chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess,
                                                        chip::Protocols::InteractionModel::Id, 0));
    }
exit:
    if (emberError == EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS)
    {
        DeviceLayer::Internal::DeviceControlServer::DeviceControlSvr().EnableNetworkForOperational(networkID);
        TLV::TLVWriter * writer          = nullptr;
        app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, ZCL_NETWORK_COMMISSIONING_CLUSTER_ID,
                                             ZCL_ENABLE_NETWORK_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        SuccessOrExit(err = apCommandObj->PrepareCommand(&cmdParams));
        VerifyOrExit((writer = apCommandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), static_cast<uint32_t>(EMBER_ZCL_NETWORK_COMMISSIONING_ERROR_SUCCESS)));
        SuccessOrExit(err = writer->PutString(TLV::ContextTag(1), ""));
        SuccessOrExit(err = apCommandObj->FinishCommand());
    }
    else if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to handle EnableNetwork command: %d (Command Error: %d)", err, emberError);
    }
}

} // namespace NetworkCommissioning
} // namespace clusters
} // namespace app
} // namespace chip
