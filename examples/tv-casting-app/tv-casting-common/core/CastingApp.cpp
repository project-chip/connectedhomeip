/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include "CastingApp.h"

#include "CommissionerDeclarationHandler.h"
#include "ConnectionCallbacks.h"
#include "support/CastingStore.h"
#include "support/ChipDeviceEventHandler.h"

#include <app/InteractionModelEngine.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

namespace matter {
namespace casting {
namespace core {

using namespace matter::casting::support;

CastingApp * CastingApp::_castingApp = nullptr;

CastingApp::CastingApp() {}

CastingApp * CastingApp::GetInstance()
{
    if (_castingApp == nullptr)
    {
        _castingApp = new CastingApp();
    }
    return _castingApp;
}

CHIP_ERROR CastingApp::Initialize(const AppParameters & appParameters)
{
    ChipLogProgress(Discovery, "CastingApp::Initialize() called");
    VerifyOrReturnError(mState == CASTING_APP_UNINITIALIZED, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(appParameters.GetCommissionableDataProvider() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(appParameters.GetDeviceAttestationCredentialsProvider() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(appParameters.GetServerInitParamsProvider() != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(chip::Platform::MemoryInit());
    ReturnErrorOnFailure(chip::DeviceLayer::PlatformMgr().InitChipStack());

    mAppParameters = &appParameters;

    chip::DeviceLayer::SetCommissionableDataProvider(appParameters.GetCommissionableDataProvider());
    chip::Credentials::SetDeviceAttestationCredentialsProvider(appParameters.GetDeviceAttestationCredentialsProvider());
    chip::Credentials::SetDeviceAttestationVerifier(appParameters.GetDeviceAttestationVerifier());

#if CHIP_ENABLE_ROTATING_DEVICE_ID
    MutableByteSpanDataProvider * uniqueIdProvider = appParameters.GetRotatingDeviceIdUniqueIdProvider();
    if (uniqueIdProvider != nullptr)
    {
        chip::MutableByteSpan * uniqueId = uniqueIdProvider->Get();
        if (uniqueId != nullptr)
        {
            ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().SetRotatingDeviceIdUniqueId(*uniqueId));
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

    mState = CASTING_APP_NOT_RUNNING; // initialization done, set state to NOT_RUNNING

    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::UpdateCommissionableDataProvider(chip::DeviceLayer::CommissionableDataProvider * commissionableDataProvider)
{
    ChipLogProgress(Discovery, "CastingApp::UpdateCommissionableDataProvider()");
    chip::DeviceLayer::SetCommissionableDataProvider(commissionableDataProvider);
    return mAppParameters->SetCommissionableDataProvider(commissionableDataProvider);
}

void ReconnectHandler(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer)
{
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingApp::ConnectionHandler() Could not reconnect to CastingPlayer %" CHIP_ERROR_FORMAT,
                     err.Format());
    }
    else
    {
        ChipLogProgress(AppServer, "CastingApp::ConnectionHandler() Reconnected to CastingPlayer(ID: %s)", castingPlayer->GetId());
    }
}

CHIP_ERROR CastingApp::Start()
{
    ChipLogProgress(Discovery, "CastingApp::Start()");
    VerifyOrReturnError(mState == CASTING_APP_NOT_RUNNING, CHIP_ERROR_INCORRECT_STATE);

    // Start Matter server
    chip::ServerInitParams * serverInitParams = mAppParameters->GetServerInitParamsProvider()->Get();
    VerifyOrReturnError(serverInitParams != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(*serverInitParams));

    // Perform post server startup registrations
    ReturnErrorOnFailure(PostStartRegistrations());

    // Reconnect (or verify connection) to the CastingPlayer that the app was connected to before being stopped, if any
    if (CastingPlayer::GetTargetCastingPlayer() != nullptr)
    {
        matter::casting::core::ConnectionCallbacks connectionCallbacks;
        connectionCallbacks.mOnConnectionComplete = ReconnectHandler;
        // Re-connecting to a CastingPLayer does not require a full User Directed Commissioning (UDC) process so
        // CommissionerDeclaration messages are not expected. Leaving ConnectionCallbacks mCommissionerDeclarationCallback as
        // nullptr.
        ChipLogProgress(
            Discovery,
            "CastingApp::Start() calling VerifyOrEstablishConnection() to reconnect (or verify connection) to a CastingPlayer");
        CastingPlayer::GetTargetCastingPlayer()->VerifyOrEstablishConnection(connectionCallbacks);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::PostStartRegistrations()
{
    ChipLogProgress(Discovery, "CastingApp::PostStartRegistrations()");
    VerifyOrReturnError(mState == CASTING_APP_NOT_RUNNING, CHIP_ERROR_INCORRECT_STATE);
    auto & server = chip::Server::GetInstance();

    // TODO: Set CastingApp as AppDelegate
    // &server.GetCommissioningWindowManager().SetAppDelegate(??);

    // Initialize binding handlers
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });

    // Set FabricDelegate
    chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(support::CastingStore::GetInstance());

    // Register DeviceEvent Handler
    ReturnErrorOnFailure(chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(ChipDeviceEventHandler::Handle, 0));

    ChipLogProgress(
        Discovery,
        "CastingApp::PostStartRegistrations() calling GetUserDirectedCommissioningClient()->SetCommissionerDeclarationHandler()");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    // Set a handler for Commissioner's CommissionerDeclaration messages. This is set in
    // connectedhomeip/src/protocols/user_directed_commissioning/UserDirectedCommissioning.h
    chip::Server::GetInstance().GetUserDirectedCommissioningClient()->SetCommissionerDeclarationHandler(
        CommissionerDeclarationHandler::GetInstance());
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    mState = CASTING_APP_RUNNING; // CastingApp started successfully, set state to RUNNING
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::Stop()
{
    ChipLogProgress(Discovery, "CastingApp::Stop()");
    VerifyOrReturnError(mState == CASTING_APP_RUNNING, CHIP_ERROR_INCORRECT_STATE);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    // Remove the handler previously set for Commissioner's CommissionerDeclaration messages.
    chip::Server::GetInstance().GetUserDirectedCommissioningClient()->SetCommissionerDeclarationHandler(nullptr);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    // Shutdown the Matter server
    chip::Server::GetInstance().Shutdown();

    mState = CASTING_APP_NOT_RUNNING; // CastingApp stopped successfully, set state to NOT_RUNNING

    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::ShutdownAllSubscriptions()
{
    chip::app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptions();

    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::ClearCache()
{
    return support::CastingStore::GetInstance()->DeleteAll();
}

}; // namespace core
}; // namespace casting
}; // namespace matter
