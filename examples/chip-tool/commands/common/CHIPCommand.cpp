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
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <credentials/examples/DeviceAttestationVerifierExample.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

CHIP_ERROR CHIPCommand::Run()
{
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    ReturnLogErrorOnFailure(chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(0, true));
#endif

    ReturnLogErrorOnFailure(mStorage.Init());
    ReturnLogErrorOnFailure(mOpCredsIssuer.Initialize(mStorage));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;

    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
    chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::Examples::GetExampleDACVerifier());

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
    ReturnLogErrorOnFailure(mOpCredsIssuer.GenerateNOCChainAfterValidation(mStorage.GetLocalNodeId(), 0, ephemeralKey.Pubkey(),
                                                                           rcacSpan, icacSpan, nocSpan));

    chip::Controller::FactoryInitParams factoryInitParams;
    factoryInitParams.storageDelegate = &mStorage;
    factoryInitParams.listenPort      = mStorage.GetListenPort();

    chip::Controller::SetupParams commissionerParams;
    commissionerParams.operationalCredentialsDelegate = &mOpCredsIssuer;
    commissionerParams.ephemeralKeypair               = &ephemeralKey;
    commissionerParams.controllerRCAC                 = rcacSpan;
    commissionerParams.controllerICAC                 = icacSpan;
    commissionerParams.controllerNOC                  = nocSpan;

    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryInitParams));
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, mController));

    chip::DeviceLayer::PlatformMgr().ScheduleWork(RunQueuedCommand, reinterpret_cast<intptr_t>(this));
    ReturnLogErrorOnFailure(StartWaiting(GetWaitDurationInSeconds()));

    Shutdown();

    //
    // We can call DeviceController::Shutdown() safely without grabbing the stack lock
    // since the CHIP thread and event queue have been stopped, preventing any thread
    // races.
    //
    ReturnLogErrorOnFailure(mController.Shutdown());

    return CHIP_NO_ERROR;
}

void CHIPCommand::RunQueuedCommand(intptr_t commandArg)
{
    auto * command = reinterpret_cast<CHIPCommand *>(commandArg);
    CHIP_ERROR err = command->Run(command->mStorage.GetRemoteNodeId());
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

CHIP_ERROR CHIPCommand::StartWaiting(uint16_t seconds)
{
#if CONFIG_USE_SEPARATE_EVENTLOOP
    // ServiceEvents() calls StartEventLoopTask(), which is paired with the StopEventLoopTask() below.
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().ServiceEvents());
    auto waitingUntil = std::chrono::system_clock::now() + std::chrono::seconds(seconds);
    {
        std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
        if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; }))
        {
            mCommandExitStatus = CHIP_ERROR_TIMEOUT;
        }
    }
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
#else
    ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(seconds * 1000, OnResponseTimeout, this));
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
