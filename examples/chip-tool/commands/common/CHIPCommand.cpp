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
#include <core/CHIPBuildConfig.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

constexpr chip::FabricId kIdentityNullFabricId  = chip::kUndefinedFabricId;
constexpr chip::FabricId kIdentityAlphaFabricId = 1;
constexpr chip::FabricId kIdentityBetaFabricId  = 2;
constexpr chip::FabricId kIdentityGammaFabricId = 3;

CHIP_ERROR CHIPCommand::Run()
{
    StartTracing();

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    ReturnLogErrorOnFailure(chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(0, true));
#endif

    ReturnLogErrorOnFailure(mDefaultStorage.Init());
    ReturnLogErrorOnFailure(mFabricStorage.Initialize(&mDefaultStorage));

    chip::Controller::FactoryInitParams factoryInitParams;
    factoryInitParams.fabricStorage = &mFabricStorage;
    factoryInitParams.listenPort    = static_cast<uint16_t>(mDefaultStorage.GetListenPort() + CurrentControllerId());
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryInitParams));

    ReturnLogErrorOnFailure(InitializeController(kIdentityNull, kIdentityNullFabricId));
    ReturnLogErrorOnFailure(InitializeController(kIdentityAlpha, kIdentityAlphaFabricId));
    ReturnLogErrorOnFailure(InitializeController(kIdentityBeta, kIdentityBetaFabricId));
    ReturnLogErrorOnFailure(InitializeController(kIdentityGamma, kIdentityGammaFabricId));

    chip::DeviceLayer::PlatformMgr().ScheduleWork(RunQueuedCommand, reinterpret_cast<intptr_t>(this));
    ReturnLogErrorOnFailure(StartWaiting(GetWaitDuration()));

    Shutdown();

    //
    // We can call DeviceController::Shutdown() safely without grabbing the stack lock
    // since the CHIP thread and event queue have been stopped, preventing any thread
    // races.
    //
    ReturnLogErrorOnFailure(ShutdownController(kIdentityNull));
    ReturnLogErrorOnFailure(ShutdownController(kIdentityAlpha));
    ReturnLogErrorOnFailure(ShutdownController(kIdentityBeta));
    ReturnLogErrorOnFailure(ShutdownController(kIdentityGamma));

    StopTracing();
    return CHIP_NO_ERROR;
}

void CHIPCommand::StartTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::InitTrace();

    if (mTraceFile.HasValue())
    {
        chip::trace::SetTraceStream(new chip::trace::TraceStreamFile(mTraceFile.Value()));
    }
    else if (mTraceLog.HasValue() && mTraceLog.Value() == true)
    {
        chip::trace::SetTraceStream(new chip::trace::TraceStreamLog());
    }
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommand::StopTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommand::SetIdentity(const char * identity)
{
    std::string name = std::string(identity);
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0 &&
        name.compare(kIdentityNull) != 0)
    {
        ChipLogError(chipTool, "Unknown controller name: %s. Supported names are [%s, %s, %s]", name.c_str(), kIdentityAlpha,
                     kIdentityBeta, kIdentityGamma);
        chipDie();
    }

    mControllerName.SetValue(const_cast<char *>(identity));
}

std::string CHIPCommand::GetIdentity()
{
    std::string name = mControllerName.HasValue() ? mControllerName.Value() : kIdentityAlpha;
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0 &&
        name.compare(kIdentityNull) != 0)
    {
        ChipLogError(chipTool, "Unknown controller name: %s. Supported names are [%s, %s, %s]", name.c_str(), kIdentityAlpha,
                     kIdentityBeta, kIdentityGamma);
        chipDie();
    }

    return name;
}

chip::FabricId CHIPCommand::CurrentControllerId()
{
    chip::FabricId id;

    std::string name = GetIdentity();
    if (name.compare(kIdentityAlpha) == 0)
    {
        id = kIdentityAlphaFabricId;
    }
    else if (name.compare(kIdentityBeta) == 0)
    {
        id = kIdentityBetaFabricId;
    }
    else if (name.compare(kIdentityGamma) == 0)
    {
        id = kIdentityGammaFabricId;
    }
    else if (name.compare(kIdentityNull) == 0)
    {
        id = kIdentityNullFabricId;
    }
    else
    {
        VerifyOrDieWithMsg(false, chipTool, "Unknown controller name: %s. Supported names are [%s, %s, %s]", name.c_str(),
                           kIdentityAlpha, kIdentityBeta, kIdentityGamma);
    }

    return id;
}

chip::Controller::DeviceController & CHIPCommand::CurrentController()
{
    auto item = mControllers.find(GetIdentity());
    return *item->second.get();
}

CHIP_ERROR CHIPCommand::ShutdownController(std::string key)
{
    return mControllers[key].get()->Shutdown();
}

CHIP_ERROR CHIPCommand::InitializeController(std::string key, chip::FabricId fabricId)
{
    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;

    std::unique_ptr<ChipDeviceController> controller = std::make_unique<ChipDeviceController>();
    chip::Controller::SetupParams controllerParams;

    ReturnLogErrorOnFailure(mCredIssuerCmds->SetupDeviceAttestation(controllerParams));
    chip::Credentials::SetDeviceAttestationVerifier(controllerParams.deviceAttestationVerifier);

    VerifyOrReturnError(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);

    chip::Crypto::P256Keypair ephemeralKey;

    if (fabricId != chip::kUndefinedFabricId)
    {

        // TODO - OpCreds should only be generated for pairing command
        //        store the credentials in persistent storage, and
        //        generate when not available in the storage.
        ReturnLogErrorOnFailure(mControllerStorage.Init(key.c_str()));
        ReturnLogErrorOnFailure(mCredIssuerCmds->InitializeCredentialsIssuer(mControllerStorage));

        chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
        chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

        ReturnLogErrorOnFailure(ephemeralKey.Initialize());
        ReturnLogErrorOnFailure(mCredIssuerCmds->GenerateControllerNOCChain(mControllerStorage.GetLocalNodeId(), fabricId,
                                                                            ephemeralKey, rcacSpan, icacSpan, nocSpan));
        controllerParams.operationalKeypair = &ephemeralKey;
        controllerParams.controllerRCAC     = rcacSpan;
        controllerParams.controllerICAC     = icacSpan;
        controllerParams.controllerNOC      = nocSpan;
    }

    controllerParams.storageDelegate                = &mControllerStorage;
    controllerParams.operationalCredentialsDelegate = mCredIssuerCmds->GetCredentialIssuer();
    controllerParams.controllerVendorId             = chip::VendorId::TestVendor1;

    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().SetupController(controllerParams, *(controller.get())));
    mControllers[key] = std::move(controller);

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
