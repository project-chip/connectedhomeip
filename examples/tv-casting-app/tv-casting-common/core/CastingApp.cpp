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

#include "CastingApp.h"

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
    VerifyOrReturnError(mState == UNINITIALIZED, CHIP_ERROR_INCORRECT_STATE);
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
    if (uniqueIdProvider != nullptr && uniqueIdProvider->Get() != nullptr)
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConfigurationMgr().SetRotatingDeviceIdUniqueId(*uniqueIdProvider->Get()));
    }
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

    mState = NOT_RUNNING; // initialization done, set state to NOT_RUNNING

    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::Start()
{
    VerifyOrReturnError(mState == NOT_RUNNING, CHIP_ERROR_INCORRECT_STATE);

    // start Matter server
    chip::ServerInitParams * serverInitParams = mAppParameters->GetServerInitParamsProvider()->Get();
    VerifyOrReturnError(serverInitParams != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(*serverInitParams));

    // perform post server startup registrations
    ReturnErrorOnFailure(PostStartRegistrations());

    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::PostStartRegistrations()
{
    VerifyOrReturnError(mState == NOT_RUNNING, CHIP_ERROR_INCORRECT_STATE);
    auto & server = chip::Server::GetInstance();

    // TODO: Set CastingApp as AppDelegate
    // &server.GetCommissioningWindowManager().SetAppDelegate(??);

    // Initialize binding handlers
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });

    // TODO: Set FabricDelegate
    // chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&mPersistenceManager);

    // TODO: Add DeviceEvent Handler
    // ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(DeviceEventCallback, 0));

    mState = RUNNING; // CastingApp started successfully, set state to RUNNING
    return CHIP_NO_ERROR;
}

CHIP_ERROR CastingApp::Stop()
{
    VerifyOrReturnError(mState == RUNNING, CHIP_ERROR_INCORRECT_STATE);

    // TODO: add logic to capture CastingPlayers that we are currently connected to, so we can automatically reconnect with them on
    // Start() again

    // Shutdown the Matter server
    chip::Server::GetInstance().Shutdown();

    mState = NOT_RUNNING; // CastingApp started successfully, set state to RUNNING

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
