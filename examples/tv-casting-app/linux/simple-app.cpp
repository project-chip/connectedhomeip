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

#include "simple-app-helper.h"

#include "core/CastingPlayer.h"
#include "core/CastingPlayerDiscovery.h"
#include "core/Types.h"

#include <LinuxCommissionableDataProvider.h>
#include <Options.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#if defined(ENABLE_CHIP_SHELL)
#include <lib/shell/Engine.h> // nogncheck
#include <thread>
using chip::Shell::Engine;
#endif

using namespace matter::casting::core;
using namespace matter::casting::support;

// To hold SPAKE2+ verifier, discriminator, passcode
LinuxCommissionableDataProvider gCommissionableDataProvider;

/**
 * @brief Provides the unique ID that is used by the SDK to generate the Rotating Device ID.
 */
class RotatingDeviceIdUniqueIdProvider : public MutableByteSpanDataProvider
{
private:
    chip::MutableByteSpan rotatingDeviceIdUniqueIdSpan;
    uint8_t rotatingDeviceIdUniqueId[chip::DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];

public:
    RotatingDeviceIdUniqueIdProvider()
    {
        // generate a random Unique ID for this example app
        for (size_t i = 0; i < sizeof(rotatingDeviceIdUniqueId); i++)
        {
            rotatingDeviceIdUniqueId[i] = chip::Crypto::GetRandU8();
        }
        rotatingDeviceIdUniqueIdSpan = chip::MutableByteSpan(rotatingDeviceIdUniqueId);
    }

    chip::MutableByteSpan * Get() { return &rotatingDeviceIdUniqueIdSpan; }
};

/**
 * @brief Provides the ServerInitParams required to start the CastingApp, which in turn starts the Matter server
 */
class CommonCaseDeviceServerInitParamsProvider : public ServerInitParamsProvider
{
private:
    // For this example, we'll use CommonCaseDeviceServerInitParams
    chip::CommonCaseDeviceServerInitParams serverInitParams;

public:
    chip::ServerInitParams * Get()
    {
        CHIP_ERROR err = serverInitParams.InitializeStaticResourcesBeforeServerInit();
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(AppServer, "Initialization of ServerInitParams failed %" CHIP_ERROR_FORMAT, err.Format()));
        return &serverInitParams;
    }
};

int main(int argc, char * argv[])
{
    ChipLogProgress(AppServer, "chip_casting_simplified = 1"); // this file is built/run only if chip_casting_simplified = 1
    // Create AppParameters that need to be passed to CastingApp.Initialize()
    AppParameters appParameters;
    RotatingDeviceIdUniqueIdProvider rotatingDeviceIdUniqueIdProvider;
    CommonCaseDeviceServerInitParamsProvider serverInitParamsProvider;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = InitCommissionableDataProvider(gCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
    VerifyOrReturnValue(
        err == CHIP_NO_ERROR, 0,
        ChipLogError(AppServer, "Initialization of CommissionableDataProvider failed %" CHIP_ERROR_FORMAT, err.Format()));
    err = appParameters.Create(&rotatingDeviceIdUniqueIdProvider, &gCommissionableDataProvider,
                               chip::Credentials::Examples::GetExampleDACProvider(),
                               GetDefaultDACVerifier(chip::Credentials::GetTestAttestationTrustStore()), &serverInitParamsProvider);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                        ChipLogError(AppServer, "Creation of AppParameters failed %" CHIP_ERROR_FORMAT, err.Format()));

    // Initialize the CastingApp
    err = CastingApp::GetInstance()->Initialize(appParameters);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                        ChipLogError(AppServer, "Initialization of CastingApp failed %" CHIP_ERROR_FORMAT, err.Format()));

    // Initialize Linux KeyValueStoreMgr
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);

    // Start the CastingApp
    err = CastingApp::GetInstance()->Start();
    VerifyOrReturnValue(err == CHIP_NO_ERROR, 0,
                        ChipLogError(AppServer, "CastingApp::Start failed %" CHIP_ERROR_FORMAT, err.Format()));

#if defined(ENABLE_CHIP_SHELL)
    chip::Shell::Engine::Root().Init();
    std::thread shellThread([]() { chip::Shell::Engine::Root().RunMainLoop(); });
    RegisterCommands();
#endif

    CastingPlayerDiscovery::GetInstance()->SetDelegate(DiscoveryDelegateImpl::GetInstance());

    // Discover CastingPlayers
    err = CastingPlayerDiscovery::GetInstance()->StartDiscovery(kTargetPlayerDeviceType);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, -1,
                        ChipLogError(AppServer, "CastingPlayerDiscovery::StartDiscovery failed %" CHIP_ERROR_FORMAT, err.Format()));

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
