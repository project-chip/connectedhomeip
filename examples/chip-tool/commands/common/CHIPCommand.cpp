/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "CHIPCommand.h"

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DefaultDeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

constexpr chip::FabricId kIdentityAlphaFabricId = 1;
constexpr chip::FabricId kIdentityBetaFabricId  = 2;
constexpr chip::FabricId kIdentityGammaFabricId = 3;

CHIP_ERROR CHIPCommand::Run()
{
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    ReturnLogErrorOnFailure(chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(0, true));
#endif

    ReturnLogErrorOnFailure(mDefaultStorage.Init());
    ReturnLogErrorOnFailure(mFabricStorage.Initialize(&mDefaultStorage));

    chip::Controller::FactoryInitParams factoryInitParams;
    factoryInitParams.fabricStorage = &mFabricStorage;
    factoryInitParams.listenPort    = static_cast<uint16_t>(mDefaultStorage.GetListenPort() + CurrentCommissionerIndex());
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryInitParams));

    ReturnLogErrorOnFailure(InitializeCommissioner(GetIdentity(), CurrentCommissionerIndex()));

    chip::DeviceLayer::PlatformMgr().ScheduleWork(RunQueuedCommand, reinterpret_cast<intptr_t>(this));
    ReturnLogErrorOnFailure(StartWaiting(GetWaitDuration()));

    Shutdown();

    //
    // We can call DeviceController::Shutdown() safely without grabbing the stack lock
    // since the CHIP thread and event queue have been stopped, preventing any thread
    // races.
    //
    ReturnLogErrorOnFailure(ShutdownCommissioner(GetIdentity()));

    return CHIP_NO_ERROR;
}

void CHIPCommand::SetIdentity(const char * identity)
{
    std::string name = std::string(identity);
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0)
    {
        ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s]", name.c_str(), kIdentityAlpha,
                     kIdentityBeta, kIdentityGamma);
        chipDie();
    }

    mCommissionerName.SetValue(const_cast<char *>(identity));
}

std::string CHIPCommand::GetIdentity()
{
    std::string name = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0)
    {
        ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s]", name.c_str(), kIdentityAlpha,
                     kIdentityBeta, kIdentityGamma);
        chipDie();
    }

    return name;
}

uint16_t CHIPCommand::CurrentCommissionerIndex()
{
    uint16_t index = 0;

    std::string name = GetIdentity();
    if (name.compare(kIdentityAlpha) == 0)
    {
        index = kIdentityAlphaFabricId;
    }
    else if (name.compare(kIdentityBeta) == 0)
    {
        index = kIdentityBetaFabricId;
    }
    else if (name.compare(kIdentityGamma) == 0)
    {
        index = kIdentityGammaFabricId;
    }

    VerifyOrDieWithMsg(index != 0, chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s]", name.c_str(),
                       kIdentityAlpha, kIdentityBeta, kIdentityGamma);
    return index;
}

chip::Controller::DeviceCommissioner & CHIPCommand::CurrentCommissioner()
{
    auto item = mCommissioners.find(GetIdentity());
    return *item->second.get();
}

CHIP_ERROR CHIPCommand::ShutdownCommissioner(std::string key)
{
    return mCommissioners[key].get()->Shutdown();
}

CHIP_ERROR CHIPCommand::InitializeCommissioner(std::string key, chip::FabricId fabricId)
{
    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;

    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());

    // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
    const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
    chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::GetDefaultDACVerifier(testingRootStore));

    VerifyOrReturnError(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);

    chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
    chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
    chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

    chip::Crypto::P256Keypair ephemeralKey;
    ReturnLogErrorOnFailure(ephemeralKey.Initialize());

    // TODO - OpCreds should only be generated for pairing command
    //        store the credentials in persistent storage, and
    //        generate when not available in the storage.
    ReturnLogErrorOnFailure(mCommissionerStorage.Init(key.c_str()));
    ReturnLogErrorOnFailure(mOpCredsIssuer.Initialize(mCommissionerStorage));
    ReturnLogErrorOnFailure(mOpCredsIssuer.GenerateNOCChainAfterValidation(mCommissionerStorage.GetLocalNodeId(), fabricId,
                                                                           ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan));

    std::unique_ptr<ChipDeviceCommissioner> commissioner = std::make_unique<ChipDeviceCommissioner>();
    chip::Controller::SetupParams commissionerParams;
    commissionerParams.storageDelegate                = &mCommissionerStorage;
    commissionerParams.operationalCredentialsDelegate = &mOpCredsIssuer;
    commissionerParams.ephemeralKeypair               = &ephemeralKey;
    commissionerParams.controllerRCAC                 = rcacSpan;
    commissionerParams.controllerICAC                 = icacSpan;
    commissionerParams.controllerNOC                  = nocSpan;
    commissionerParams.controllerVendorId             = chip::VendorId::TestVendor1;

    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, *(commissioner.get())));
    mCommissioners[key] = std::move(commissioner);

    return CHIP_NO_ERROR;
}

void CHIPCommand::RunQueuedCommand(intptr_t commandArg)
{
    auto * command = reinterpret_cast<CHIPCommand *>(commandArg);
    CHIP_ERROR err = command->RunCommand();
    if (err != CHIP_NO_ERROR)
    {
        command->SetCommandExitStatus(err);
    }
}

#if !CONFIG_USE_SEPARATE_EVENTLOOP
static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    (reinterpret_cast<CHIPCommand *>(appState))->SetCommandExitStatus(CHIP_ERROR_TIMEOUT);
}
#endif // !CONFIG_USE_SEPARATE_EVENTLOOP

CHIP_ERROR CHIPCommand::StartWaiting(chip::System::Clock::Timeout duration)
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    // ServiceEvents() calls StartEventLoopTask(), which is paired with the StopEventLoopTask() below.
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().ServiceEvents());
    auto waitingUntil = std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::seconds>(duration);
    {
        std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
        if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; }))
        {
            mCommandExitStatus = CHIP_ERROR_TIMEOUT;
        }
    }
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
#else
    ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, this));
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
#endif // CONFIG_USE_SEPARATE_EVENTLOOP

    return mCommandExitStatus;
}

void CHIPCommand::StopWaiting()
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = false;
    }
    cvWaitingForResponse.notify_all();
#else  // CONFIG_USE_SEPARATE_EVENTLOOP
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
#endif // CONFIG_USE_SEPARATE_EVENTLOOP
}
