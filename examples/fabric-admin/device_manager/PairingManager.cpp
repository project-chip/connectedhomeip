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

Platform::UniquePtr<chip::Controller::CommissioningWindowOpener> PairingManager::mWindowOpener;

PairingManager::PairingManager() :
    mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this),
    mOnOpenCommissioningWindowVerifierCallback(OnOpenCommissioningWindowVerifierResponse, this)
{}

CHIP_ERROR PairingManager::Init(chip::Controller::DeviceCommissioner * commissioner)
{
    mCommissioner = commissioner;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PairingManager::OpenCommissioningWindow(NodeId nodeId, EndpointId endpointId, uint16_t commissioningTimeout,
                                                   uint32_t iterations, uint16_t discriminator, const ByteSpan & salt,
                                                   const ByteSpan & verifier)
{
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);

    auto * params                      = new CommissioningWindowParams();
    params->nodeId                     = nodeId;
    params->endpointId                 = endpointId;
    params->commissioningWindowTimeout = commissioningTimeout;
    params->iteration                  = iterations;
    params->discriminator              = discriminator;

    if (!salt.empty())
    {
        memcpy(params->saltBuffer, salt.data(), salt.size());
        params->salt = ByteSpan(params->saltBuffer, salt.size());
    }

    if (!verifier.empty())
    {
        memcpy(params->verifierBuffer, verifier.data(), verifier.size());
        params->verifier = ByteSpan(params->verifierBuffer, verifier.size());
    }

    // Schedule work on the Matter thread
    chip::DeviceLayer::PlatformMgr().ScheduleWork(OnOpenCommissioningWindow, reinterpret_cast<intptr_t>(params));

    return CHIP_NO_ERROR;
}

void PairingManager::OnOpenCommissioningWindow(intptr_t context)
{
    auto * params         = reinterpret_cast<CommissioningWindowParams *>(context);
    PairingManager & self = PairingManager::Instance();

    if (self.mCommissioner == nullptr)
    {
        ChipLogError(AppServer, "Commissioner is null, cannot open commissioning window");
        return;
    }

    mWindowOpener = Platform::MakeUnique<Controller::CommissioningWindowOpener>(self.mCommissioner);

    if (!params->verifier.empty())
    {
        if (params->salt.empty())
        {
            ChipLogError(AppServer, "Salt is required when verifier is set");
            return;
        }

        CHIP_ERROR err = mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
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
            ChipLogError(AppServer, "Failed to open commissioning window with verifier: %s", ErrorStr(err));
        }
    }
    else
    {
        SetupPayload ignored;
        CHIP_ERROR err = mWindowOpener->OpenCommissioningWindow(Controller::CommissioningWindowPasscodeParams()
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
            ChipLogError(AppServer, "Failed to open commissioning window with passcode: %s", ErrorStr(err));
        }
    }

    // Clean up params
    delete params;
}

void PairingManager::OnOpenCommissioningWindowResponse(void * context, NodeId remoteId, CHIP_ERROR err, chip::SetupPayload payload)
{
    PairingManager * self = static_cast<PairingManager *>(context);
    if (self->mCommissioningWindowDelegate)
    {
        self->mCommissioningWindowDelegate->OnCommissioningWindowOpened(remoteId, err, payload);
        self->UnregisterOpenCommissioningWindowDelegate();
    }

    OnOpenCommissioningWindowVerifierResponse(context, remoteId, err);
}

void PairingManager::OnOpenCommissioningWindowVerifierResponse(void * context, NodeId remoteId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    PairingManager * self = reinterpret_cast<PairingManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(NotSpecified, "OnOpenCommissioningWindowCommand: context is null"));
}
