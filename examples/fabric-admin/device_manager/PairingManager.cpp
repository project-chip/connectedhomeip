/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "PairingManager.h"

#include <controller/CommissioningDelegate.h>
#include <controller/CurrentFabricRemover.h>
#include <lib/support/Span.h>

using namespace ::chip;

PairingManager::PairingManager() :
    mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this),
    mOnOpenCommissioningWindowVerifierCallback(OnOpenCommissioningWindowVerifierResponse, this)
{}

void PairingManager::Init(Controller::DeviceCommissioner * commissioner)
{
    VerifyOrDie(mCommissioner == nullptr);
    mCommissioner = commissioner;
}

CHIP_ERROR PairingManager::OpenCommissioningWindow(NodeId nodeId, EndpointId endpointId, uint16_t commissioningTimeoutSec,
                                                   uint32_t iterations, uint16_t discriminator, const ByteSpan & salt,
                                                   const ByteSpan & verifier)
{
    if (mCommissioner == nullptr)
    {
        ChipLogError(NotSpecified, "Commissioner is null, cannot open commissioning window");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Check if a window is already open
    if (mWindowOpener != nullptr)
    {
        ChipLogError(NotSpecified, "A commissioning window is already open");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto params                        = Platform::MakeUnique<CommissioningWindowParams>();
    params->nodeId                     = nodeId;
    params->endpointId                 = endpointId;
    params->commissioningWindowTimeout = commissioningTimeoutSec;
    params->iteration                  = iterations;
    params->discriminator              = discriminator;

    if (!salt.empty())
    {
        if (salt.size() > sizeof(params->saltBuffer))
        {
            ChipLogError(NotSpecified, "Salt size exceeds buffer capacity");
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(params->saltBuffer, salt.data(), salt.size());
        params->salt = ByteSpan(params->saltBuffer, salt.size());
    }

    if (!verifier.empty())
    {
        if (verifier.size() > sizeof(params->verifierBuffer))
        {
            ChipLogError(NotSpecified, "Verifier size exceeds buffer capacity");
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(params->verifierBuffer, verifier.data(), verifier.size());
        params->verifier = ByteSpan(params->verifierBuffer, verifier.size());
    }

    // Schedule work on the Matter thread
    return DeviceLayer::PlatformMgr().ScheduleWork(OnOpenCommissioningWindow, reinterpret_cast<intptr_t>(params.release()));
}

void PairingManager::OnOpenCommissioningWindow(intptr_t context)
{
    Platform::UniquePtr<CommissioningWindowParams> params(reinterpret_cast<CommissioningWindowParams *>(context));
    PairingManager & self = PairingManager::Instance();

    if (self.mCommissioner == nullptr)
    {
        ChipLogError(NotSpecified, "Commissioner is null, cannot open commissioning window");
        return;
    }

    self.mWindowOpener = Platform::MakeUnique<Controller::CommissioningWindowOpener>(self.mCommissioner);

    if (!params->verifier.empty())
    {
        if (params->salt.empty())
        {
            ChipLogError(NotSpecified, "Salt is required when verifier is set");
            self.mWindowOpener.reset();
            return;
        }

        CHIP_ERROR err =
            self.mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                                            .SetNodeId(params->nodeId)
                                                            .SetEndpointId(params->endpointId)
                                                            .SetTimeout(params->commissioningWindowTimeout)
                                                            .SetIteration(params->iteration)
                                                            .SetDiscriminator(params->discriminator)
                                                            .SetVerifier(params->verifier)
                                                            .SetSalt(params->salt)
                                                            .SetCallback(&self.mOnOpenCommissioningWindowVerifierCallback));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to open commissioning window with verifier: %s", ErrorStr(err));
            self.mWindowOpener.reset();
        }
    }
    else
    {
        SetupPayload ignored;
        CHIP_ERROR err = self.mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
                                                                         .SetNodeId(params->nodeId)
                                                                         .SetEndpointId(params->endpointId)
                                                                         .SetTimeout(params->commissioningWindowTimeout)
                                                                         .SetIteration(params->iteration)
                                                                         .SetDiscriminator(params->discriminator)
                                                                         .SetSetupPIN(NullOptional)
                                                                         .SetSalt(NullOptional)
                                                                         .SetCallback(&self.mOnOpenCommissioningWindowCallback),
                                                                     ignored);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to open commissioning window with passcode: %s", ErrorStr(err));
            self.mWindowOpener.reset();
        }
    }
}

void PairingManager::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err, SetupPayload payload)
{
    VerifyOrDie(context != nullptr);
    PairingManager * self = static_cast<PairingManager *>(context);
    if (self->mCommissioningWindowDelegate)
    {
        self->mCommissioningWindowDelegate->OnCommissioningWindowOpened(remoteId, err, payload);
        self->SetOpenCommissioningWindowDelegate(nullptr);
    }

    OnOpenCommissioningWindowVerifierResponse(context, remoteId, err);
}

void PairingManager::OnOpenCommissioningWindowVerifierResponse(void * context, NodeId remoteId, CHIP_ERROR err)
{
    VerifyOrDie(context != nullptr);
    PairingManager * self = static_cast<PairingManager *>(context);
    LogErrorOnFailure(err);

    // Reset the window opener once the window operation is complete
    self->mWindowOpener.reset();
}
