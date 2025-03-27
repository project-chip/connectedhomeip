/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "CastingPlayer.h"
#include "CastingPlayerDiscovery.h"

#include "support/CastingStore.h"

#include <app/server/Server.h>

namespace matter {
namespace casting {
namespace core {

memory::Weak<CastingPlayer> CastingPlayer::mTargetCastingPlayer;

void CastingPlayer::VerifyOrEstablishConnection(ConnectionCallbacks connectionCallbacks, uint16_t commissioningWindowTimeoutSec,
                                                IdentificationDeclarationOptions idOptions)
{
    ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() called");

    CastingPlayerDiscovery * castingPlayerDiscovery = CastingPlayerDiscovery::GetInstance();
    std::vector<core::CastingPlayer>::iterator it;
    std::vector<core::CastingPlayer> cachedCastingPlayers = support::CastingStore::GetInstance()->ReadAll();

    CHIP_ERROR err = CHIP_NO_ERROR;

    // ensure the app was not already in the process of connecting to this CastingPlayer
    err = (mConnectionState != CASTING_PLAYER_CONNECTING ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(
        mConnectionState != CASTING_PLAYER_CONNECTING,
        ChipLogError(
            AppServer,
            "CastingPlayer::VerifyOrEstablishConnection() called while already connecting/connected to this CastingPlayer"));
    VerifyOrExit(
        connectionCallbacks.mOnConnectionComplete != nullptr,
        ChipLogError(AppServer,
                     "CastingPlayer::VerifyOrEstablishConnection() ConnectionCallbacks.mOnConnectionComplete was not provided"));
    mConnectionState               = CASTING_PLAYER_CONNECTING;
    mOnCompleted                   = connectionCallbacks.mOnConnectionComplete;
    mCommissioningWindowTimeoutSec = commissioningWindowTimeoutSec;
    mTargetCastingPlayer           = weak_from_this();
    mIdOptions                     = idOptions;
    castingPlayerDiscovery->ClearDisconnectedCastingPlayersInternal();

    // Register the handler for Commissioner's CommissionerDeclaration messages. The CommissionerDeclaration messages provide
    // information indicating the Commissioner's pre-commissioning state.
    if (connectionCallbacks.mCommissionerDeclarationCallback != nullptr)
    {
        ChipLogProgress(AppServer,
                        "CastingPlayer::VerifyOrEstablishConnection() Setting CommissionerDeclarationCallback in "
                        "CommissionerDeclarationHandler");
        // Set the callback for handling CommissionerDeclaration messages.
        matter::casting::core::CommissionerDeclarationHandler::GetInstance()->SetCommissionerDeclarationCallback(
            connectionCallbacks.mCommissionerDeclarationCallback);
        mClientProvidedCommissionerDeclarationCallback = true;
    }
    else
    {
        ChipLogProgress(
            AppServer,
            "CastingPlayer::VerifyOrEstablishConnection() CommissionerDeclarationCallback not provided in ConnectionCallbacks");
        mClientProvidedCommissionerDeclarationCallback = false;
    }

    ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() verifying User Directed Commissioning (UDC) state");
    mIdOptions.LogDetail();
    if (!GetSupportsCommissionerGeneratedPasscode() && mIdOptions.mCommissionerPasscode)
    {
        ChipLogError(AppServer,
                     "CastingPlayer::VerifyOrEstablishConnection() the target CastingPlayer doesn't support Commissioner-Generated "
                     "passcode yet IdentificationDeclarationOptions.mCommissionerPasscode is set to true");
        SuccessOrExit(err = CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (!matter::casting::core::CommissionerDeclarationHandler::GetInstance()->HasCommissionerDeclarationCallback() &&
        mIdOptions.mCommissionerPasscode)
    {
        ChipLogError(
            AppServer,
            "CastingPlayer::VerifyOrEstablishConnection() the CommissionerDeclarationHandler CommissionerDeclaration message "
            "callback has not been set, yet IdentificationDeclarationOptions.mCommissionerPasscode is set to true");
        SuccessOrExit(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // If *this* CastingPlayer was previously connected to, its nodeId, fabricIndex and other attributes should be present
    // in the CastingStore cache. If that is the case, AND, the cached data contains the endpoint desired by the client, if any,
    // as per IdentificationDeclarationOptions.mTargetAppInfos, simply Find or Re-establish the CASE session and return early.
    if (cachedCastingPlayers.size() != 0)
    {
        ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() Re-establishing CASE with cached CastingPlayer");
        it = std::find_if(cachedCastingPlayers.begin(), cachedCastingPlayers.end(),
                          [this](const core::CastingPlayer & castingPlayerParam) { return castingPlayerParam == *this; });

        // found the CastingPlayer in cache
        if (it != cachedCastingPlayers.end())
        {
            ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() found this CastingPlayer in app cache");
            unsigned index = (unsigned int) std::distance(cachedCastingPlayers.begin(), it);
            if (ContainsDesiredTargetApp(&cachedCastingPlayers[index], idOptions.getTargetAppInfoList()))
            {
                ChipLogProgress(
                    AppServer,
                    "CastingPlayer::VerifyOrEstablishConnection() calling FindOrEstablishSession on cached CastingPlayer");
                *this                          = cachedCastingPlayers[index];
                mConnectionState               = CASTING_PLAYER_CONNECTING;
                mOnCompleted                   = connectionCallbacks.mOnConnectionComplete;
                mCommissioningWindowTimeoutSec = commissioningWindowTimeoutSec;

                FindOrEstablishSession(
                    nullptr,
                    [](void * context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
                        ChipLogProgress(AppServer,
                                        "CastingPlayer::VerifyOrEstablishConnection() FindOrEstablishSession Connection to "
                                        "CastingPlayer successful");
                        CastingPlayer::GetTargetCastingPlayer()->mConnectionState = CASTING_PLAYER_CONNECTED;

                        // this async call will Load all the endpoints with their respective attributes into the TargetCastingPlayer
                        // persist the TargetCastingPlayer information into the CastingStore and call mOnCompleted()
                        support::EndpointListLoader::GetInstance()->Initialize(&exchangeMgr, &sessionHandle);
                        support::EndpointListLoader::GetInstance()->Load();
                    },
                    [](void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
                        ChipLogError(AppServer,
                                     "CastingPlayer::VerifyOrEstablishConnection() FindOrEstablishSession Connection to "
                                     "CastingPlayer failed");
                        CastingPlayer::GetTargetCastingPlayer()->mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
                        CHIP_ERROR e = support::CastingStore::GetInstance()->Delete(*CastingPlayer::GetTargetCastingPlayer());
                        if (e != CHIP_NO_ERROR)
                        {
                            ChipLogError(AppServer, "CastingStore::Delete() failed. Err: %" CHIP_ERROR_FORMAT, e.Format());
                        }

                        VerifyOrReturn(CastingPlayer::GetTargetCastingPlayer()->mOnCompleted);
                        CastingPlayer::GetTargetCastingPlayer()->mOnCompleted(error, nullptr);
                        mTargetCastingPlayer.reset();
                    });
                return; // FindOrEstablishSession called. Return early.
            }
        }
    }

    // this CastingPlayer is not in the list of cached CastingPlayers previously connected to or the cached data
    // does not contain the endpoint the client desires to interact with. So, this VerifyOrEstablishConnection call
    // will require User Directed Commissioning.
    if (chip::Server::GetInstance().GetFailSafeContext().IsFailSafeArmed())
    {
        ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() Forcing expiry of armed FailSafe timer");
        // ChipDeviceEventHandler will handle the kFailSafeTimerExpired event by Opening the Basic Commissioning Window and Sending
        // the User Directed Commissioning Request
        support::ChipDeviceEventHandler::SetUdcStatus(false);
        chip::Server::GetInstance().GetFailSafeContext().ForceFailSafeTimerExpiry();
    }
    else
    {
        // We need to call OpenBasicCommissioningWindow() for both the Client/Commissionee-Generated passcode commissioning flow and
        // Casting Player/Commissioner-Generated passcode commissioning flow. Per the Matter spec (UserDirectedCommissioning), even
        // if the Commissionee sends an IdentificationDeclaration with CommissionerPasscode set to true, the Commissioner will first
        // attempt to use AccountLogin in order to obtain Passcode using rotatingID. If no Passcode is obtained, Commissioner
        // displays a Passcode.
        ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() calling OpenBasicCommissioningWindow()");
        SuccessOrExit(err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                          chip::System::Clock::Seconds16(mCommissioningWindowTimeoutSec)));

        ChipLogProgress(AppServer, "CastingPlayer::VerifyOrEstablishConnection() calling SendUserDirectedCommissioningRequest()");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
        SuccessOrExit(err = SendUserDirectedCommissioningRequest());
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingPlayer::VerifyOrEstablishConnection() failed with %" CHIP_ERROR_FORMAT, err.Format());
        resetState(err);
    }
}

CHIP_ERROR CastingPlayer::ContinueConnecting()
{
    ChipLogProgress(AppServer, "CastingPlayer::ContinueConnecting()");
    VerifyOrReturnValue(mOnCompleted != nullptr, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(AppServer, "CastingPlayer::ContinueConnecting() mOnCompleted == nullptr"););
    VerifyOrReturnValue(mConnectionState == CASTING_PLAYER_CONNECTING, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "CastingPlayer::ContinueConnecting() called while not in connecting state"););
    VerifyOrReturnValue(
        mIdOptions.mCommissionerPasscode, CHIP_ERROR_INCORRECT_STATE,
        ChipLogError(AppServer,
                     "CastingPlayer::ContinueConnecting() mIdOptions.mCommissionerPasscode == false, ContinueConnecting() should "
                     "only be called when the CastingPlayer/Commissioner-Generated passcode commissioning flow is in progress."););

    CHIP_ERROR err       = CHIP_NO_ERROR;
    mTargetCastingPlayer = weak_from_this();

    ChipLogProgress(AppServer, "CastingPlayer::ContinueConnecting() calling OpenBasicCommissioningWindow()");
    SuccessOrExit(err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow(
                      chip::System::Clock::Seconds16(mCommissioningWindowTimeoutSec)));

    mIdOptions.mCommissionerPasscodeReady = true;
    ChipLogProgress(AppServer, "CastingPlayer::ContinueConnecting() calling SendUserDirectedCommissioningRequest()");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    SuccessOrExit(err = SendUserDirectedCommissioningRequest());
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingPlayer::ContinueConnecting() failed with %" CHIP_ERROR_FORMAT, err.Format());
        resetState(err);
    }

    return err;
}

CHIP_ERROR CastingPlayer::StopConnecting()
{
    // Calling the internal StopConnecting() API with the shouldSendIdentificationDeclarationMessage set to true to notify the
    // CastingPlayer/Commissioner that the commissioning session was cancelled by the Casting Client/Commissionee user. This will
    // result in the Casting Client/Commissionee sending a CancelPasscode IdentificationDeclaration message to the CastingPlayer.
    // shouldSendIdentificationDeclarationMessage is true when StopConnecting() is called by the Client.
    return this->StopConnecting(true);
}

CHIP_ERROR CastingPlayer::StopConnecting(bool shouldSendIdentificationDeclarationMessage)
{
    ChipLogProgress(AppServer, "CastingPlayer::StopConnecting() called, while ChipDeviceEventHandler.sUdcInProgress: %s",
                    support::ChipDeviceEventHandler::isUdcInProgress() ? "true" : "false");
    VerifyOrReturnValue(mConnectionState == CASTING_PLAYER_CONNECTING, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "CastingPlayer::StopConnecting() called while not in connecting state"););
    CHIP_ERROR err = CHIP_NO_ERROR;
    mIdOptions.resetState();
    mIdOptions.mCancelPasscode     = true;
    mConnectionState               = CASTING_PLAYER_NOT_CONNECTED;
    mCommissioningWindowTimeoutSec = kCommissioningWindowTimeoutSec;
    mTargetCastingPlayer.reset();
    CastingPlayerDiscovery::GetInstance()->ClearCastingPlayersInternal();

    if (!shouldSendIdentificationDeclarationMessage)
    {
        ChipLogProgress(AppServer,
                        "CastingPlayer::StopConnecting() shouldSendIdentificationDeclarationMessage: %d, User Directed "
                        "Commissioning aborted by the CastingPlayer/Commissioner user.",
                        shouldSendIdentificationDeclarationMessage);
        resetState(CHIP_ERROR_CONNECTION_ABORTED);
        return err;
    }

    // If a CastingPlayer::ContinueConnecting() error occurs, StopConnecting() can be called while sUdcInProgress == true.
    // sUdcInProgress should be set to false before sending the CancelPasscode IdentificationDeclaration message to the
    // CastingPlayer/Commissioner.
    if (support::ChipDeviceEventHandler::isUdcInProgress())
    {
        support::ChipDeviceEventHandler::SetUdcStatus(false);
    }

    ChipLogProgress(AppServer,
                    "CastingPlayer::StopConnecting() calling SendUserDirectedCommissioningRequest() to indicate "
                    "Client/Commissionee user canceled passcode entry");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    err = SendUserDirectedCommissioningRequest();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingPlayer::StopConnecting() failed with %" CHIP_ERROR_FORMAT, err.Format());
        resetState(err);
        return err;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    // CastingPlayer::SendUserDirectedCommissioningRequest() calls SetUdcStatus(true) before sending the UDC
    // IdentificationDeclaration message. Since StopConnecting() is attempting to cancel the commissioning process, we need to set
    // the UDC status to false after sending the message.
    support::ChipDeviceEventHandler::SetUdcStatus(false);

    ChipLogProgress(AppServer, "CastingPlayer::StopConnecting() User Directed Commissioning stopped");
    return err;
}

void CastingPlayer::resetState(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "CastingPlayer::resetState()");
    support::ChipDeviceEventHandler::SetUdcStatus(false);
    mConnectionState               = CASTING_PLAYER_NOT_CONNECTED;
    mCommissioningWindowTimeoutSec = kCommissioningWindowTimeoutSec;
    mTargetCastingPlayer.reset();
    if (mOnCompleted)
    {
        mOnCompleted(err, nullptr);
        mOnCompleted = nullptr;
    }
    CastingPlayerDiscovery::GetInstance()->ClearCastingPlayersInternal();
}

void CastingPlayer::Disconnect()
{
    ChipLogProgress(AppServer, "CastingPlayer::Disconnect()");
    mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
    mTargetCastingPlayer.reset();
    CastingPlayerDiscovery::GetInstance()->ClearCastingPlayersInternal();
}

void CastingPlayer::RegisterEndpoint(const memory::Strong<Endpoint> endpoint)
{
    auto it = std::find_if(mEndpoints.begin(), mEndpoints.end(), [endpoint](const memory::Strong<Endpoint> & _endpoint) {
        return _endpoint->GetId() == endpoint->GetId();
    });

    // If existing endpoint, update mEndpoints. If new endpoint, add it to the vector mEndpoints
    if (it != mEndpoints.end())
    {
        unsigned index    = (unsigned int) std::distance(mEndpoints.begin(), it);
        mEndpoints[index] = endpoint;
    }
    else
    {
        mEndpoints.push_back(endpoint);
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
CHIP_ERROR CastingPlayer::SendUserDirectedCommissioningRequest()
{
    ChipLogProgress(AppServer, "CastingPlayer::SendUserDirectedCommissioningRequest()");
    chip::Inet::IPAddress * ipAddressToUse = GetIpAddressForUDCRequest();
    VerifyOrReturnValue(ipAddressToUse != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "No IP Address found to send UDC request to"));

    chip::Protocols::UserDirectedCommissioning::IdentificationDeclaration id = mIdOptions.buildIdentificationDeclarationMessage();

    ReturnErrorOnFailure(support::ChipDeviceEventHandler::SetUdcStatus(true));

    ReturnErrorOnFailure(chip::Server::GetInstance().SendUserDirectedCommissioningRequest(
        chip::Transport::PeerAddress::UDP(*ipAddressToUse, mAttributes.port, mAttributes.interfaceId), id));

    ChipLogProgress(AppServer, "CastingPlayer::SendUserDirectedCommissioningRequest() complete");
    return CHIP_NO_ERROR;
}

chip::Inet::IPAddress * CastingPlayer::GetIpAddressForUDCRequest()
{
    size_t ipIndexToUse = 0;
    for (size_t i = 0; i < mAttributes.numIPs; i++)
    {
        if (mAttributes.ipAddresses[i].IsIPv4())
        {
            ipIndexToUse = i;
            ChipLogProgress(AppServer, "Found IPv4 address at index: %lu - prioritizing use of IPv4",
                            static_cast<long>(ipIndexToUse));
            break;
        }

        if (i == (mAttributes.numIPs - 1))
        {
            ChipLogProgress(AppServer, "Could not find an IPv4 address, defaulting to the first address in IP list");
        }
    }

    return &mAttributes.ipAddresses[ipIndexToUse];
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

void CastingPlayer::FindOrEstablishSession(void * clientContext, chip::OnDeviceConnected onDeviceConnected,
                                           chip::OnDeviceConnectionFailure onDeviceConnectionFailure)
{
    ChipLogProgress(AppServer, "CastingPlayer::FindOrEstablishSession() called on nodeId=0x" ChipLogFormatX64 " fabricIndex=%d",
                    ChipLogValueX64(mAttributes.nodeId), mAttributes.fabricIndex);
    VerifyOrReturn(mAttributes.nodeId != 0 && mAttributes.fabricIndex != 0,
                   ChipLogError(AppServer, "CastingPlayer::FindOrEstablishSession() called on invalid nodeId/fabricIndex"));

    ConnectionContext * connectionContext =
        new ConnectionContext(clientContext, this, onDeviceConnected, onDeviceConnectionFailure);

    chip::Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(
        chip::ScopedNodeId(mAttributes.nodeId, mAttributes.fabricIndex), connectionContext->mOnConnectedCallback,
        connectionContext->mOnConnectionFailureCallback);
}

bool CastingPlayer::ContainsDesiredTargetApp(
    core::CastingPlayer * cachedCastingPlayer,
    std::vector<chip::Protocols::UserDirectedCommissioning::TargetAppInfo> desiredTargetApps)
{
    std::vector<memory::Strong<Endpoint>> cachedEndpoints = cachedCastingPlayer->GetEndpoints();
    for (size_t i = 0; i < desiredTargetApps.size(); i++)
    {
        for (const auto & cachedEndpoint : cachedEndpoints)
        {
            bool match = true;
            match = match && (desiredTargetApps[i].vendorId == 0 || cachedEndpoint->GetVendorId() == desiredTargetApps[i].vendorId);
            match =
                match && (desiredTargetApps[i].productId == 0 || cachedEndpoint->GetProductId() == desiredTargetApps[i].productId);
            if (match)
            {
                ChipLogProgress(AppServer, "CastingPlayer::ContainsDesiredTargetApp() matching cached CastingPlayer found");
                return true;
            }
        }
    }
    ChipLogProgress(AppServer, "CastingPlayer::ContainsDesiredTargetApp() matching cached CastingPlayer not found");
    return false;
}

void CastingPlayer::LogDetail() const
{
    ChipLogProgress(AppServer, "CastingPlayer::LogDetail()");
    if (strlen(mAttributes.id) != 0)
    {
        ChipLogDetail(AppServer, "\tID: %s", mAttributes.id);
    }
    if (strlen(mAttributes.deviceName) != 0)
    {
        ChipLogDetail(AppServer, "\tDevice Name: %s", mAttributes.deviceName);
    }
    if (strlen(mAttributes.hostName) != 0)
    {
        ChipLogDetail(AppServer, "\tHost Name: %s", mAttributes.hostName);
    }
    if (strlen(mAttributes.instanceName) != 0)
    {
        ChipLogDetail(AppServer, "\tInstance Name: %s", mAttributes.instanceName);
    }
    if (mAttributes.numIPs > 0)
    {
        ChipLogDetail(AppServer, "\tNumber of IPs: %u", mAttributes.numIPs);
    }
    char buf[chip::Inet::IPAddress::kMaxStringLength];
    if (strlen(mAttributes.ipAddresses[0].ToString(buf)) != 0)
    {
        for (unsigned j = 0; j < mAttributes.numIPs; j++)
        {
            [[maybe_unused]] char * ipAddressOut = mAttributes.ipAddresses[j].ToString(buf);
            ChipLogDetail(AppServer, "\tIP Address #%d: %s", j + 1, ipAddressOut);
        }
    }
    if (mAttributes.port > 0)
    {
        ChipLogDetail(AppServer, "\tPort: %u", mAttributes.port);
    }
    if (mAttributes.productId > 0)
    {
        ChipLogDetail(AppServer, "\tProduct ID: %u", mAttributes.productId);
    }
    if (mAttributes.vendorId > 0)
    {
        ChipLogDetail(AppServer, "\tVendor ID: %u", mAttributes.vendorId);
    }
    if (mAttributes.deviceType > 0)
    {
        ChipLogDetail(AppServer, "\tDevice Type: %" PRIu32, mAttributes.deviceType);
    }
    ChipLogDetail(AppServer, "\tSupports Commissioner Generated Passcode: %s",
                  mAttributes.supportsCommissionerGeneratedPasscode ? "true" : "false");
    if (mAttributes.nodeId > 0)
    {
        ChipLogDetail(AppServer, "\tNode ID: 0x" ChipLogFormatX64, ChipLogValueX64(mAttributes.nodeId));
    }
    if (mAttributes.fabricIndex > 0)
    {
        ChipLogDetail(AppServer, "\tFabric Index: %u", mAttributes.fabricIndex);
    }
}

CastingPlayer::CastingPlayer(const CastingPlayer & other) :
    std::enable_shared_from_this<CastingPlayer>(other), mEndpoints(other.mEndpoints), mConnectionState(other.mConnectionState),
    mAttributes(other.mAttributes), mIdOptions(other.mIdOptions),
    mCommissioningWindowTimeoutSec(other.mCommissioningWindowTimeoutSec), mOnCompleted(other.mOnCompleted)
{}

CastingPlayer & CastingPlayer::operator=(const CastingPlayer & other)
{
    if (this != &other)
    {
        mAttributes                    = other.mAttributes;
        mEndpoints                     = other.mEndpoints;
        mConnectionState               = other.mConnectionState;
        mIdOptions                     = other.mIdOptions;
        mCommissioningWindowTimeoutSec = other.mCommissioningWindowTimeoutSec;
        mOnCompleted                   = other.mOnCompleted;
    }
    return *this;
}

ConnectionContext::ConnectionContext(void * clientContext, core::CastingPlayer * targetCastingPlayer,
                                     chip::OnDeviceConnected onDeviceConnectedFn,
                                     chip::OnDeviceConnectionFailure onDeviceConnectionFailureFn)
{
    ChipLogProgress(AppServer, "CastingPlayer::ConnectionContext()");
    mClientContext               = clientContext;
    mTargetCastingPlayer         = targetCastingPlayer;
    mOnDeviceConnectedFn         = onDeviceConnectedFn;
    mOnDeviceConnectionFailureFn = onDeviceConnectionFailureFn;

    mOnConnectedCallback = new chip::Callback::Callback<chip::OnDeviceConnected>(
        [](void * context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle) {
            ChipLogProgress(AppServer, "CastingPlayer::ConnectionContext() Device Connection success callback called");
            ConnectionContext * connectionContext = static_cast<ConnectionContext *>(context);
            VerifyOrReturn(
                connectionContext != nullptr && connectionContext->mTargetCastingPlayer != nullptr,
                ChipLogError(
                    AppServer,
                    "CastingPlayer::ConnectionContext() Invalid ConnectionContext received in DeviceConnection success callback"));

            ChipLogProgress(AppServer,
                            "CastingPlayer::ConnectionContext() calling mConnectionState = core::CASTING_PLAYER_CONNECTED");
            connectionContext->mTargetCastingPlayer->mConnectionState = core::CASTING_PLAYER_CONNECTED;
            ChipLogProgress(AppServer, "CastingPlayer::ConnectionContext() calling mOnDeviceConnectedFn");
            connectionContext->mOnDeviceConnectedFn(connectionContext->mClientContext, exchangeMgr, sessionHandle);
            ChipLogProgress(AppServer, "CastingPlayer::ConnectionContext() calling delete connectionContext");
            delete connectionContext;
        },
        this);

    mOnConnectionFailureCallback = new chip::Callback::Callback<chip::OnDeviceConnectionFailure>(
        [](void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error) {
            ChipLogError(AppServer,
                         "CastingPlayer::ConnectionContext() Device Connection failure callback called with %" CHIP_ERROR_FORMAT,
                         error.Format());
            ConnectionContext * connectionContext = static_cast<ConnectionContext *>(context);
            VerifyOrReturn(
                connectionContext != nullptr && connectionContext->mTargetCastingPlayer != nullptr,
                ChipLogError(
                    AppServer,
                    "CastingPlayer::ConnectionContext() Invalid ConnectionContext received in DeviceConnection failure callback"));
            connectionContext->mTargetCastingPlayer->mConnectionState = CASTING_PLAYER_NOT_CONNECTED;
            connectionContext->mOnDeviceConnectionFailureFn(connectionContext->mClientContext, peerId, error);
            delete connectionContext;
        },
        this);
}

ConnectionContext::~ConnectionContext()
{
    if (mOnConnectedCallback != nullptr)
    {
        delete mOnConnectedCallback;
    }

    if (mOnConnectionFailureCallback != nullptr)
    {
        delete mOnConnectionFailureCallback;
    }
}

}; // namespace core
}; // namespace casting
}; // namespace matter
