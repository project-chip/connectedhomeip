/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Declaration of Commissioner Discovery Controller,
 *      a common class that manages state and callbacks
 *      for handling the Commissioner Discovery
 *      and User Directed Commissioning workflow
 *
 */
#include <iostream>
#include <thread>

#include <controller/CommissionerDiscoveryController.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace chip::Protocols::UserDirectedCommissioning;

void CommissionerDiscoveryController::OnUserDirectedCommissioningRequest(UDCClientState state)
{
    strncpy(mCurrentInstance, state.GetInstanceName(), sizeof(mCurrentInstance));
    mPendingConsent = true;
    ChipLogDetail(Controller,
                  "------PROMPT USER: %s is requesting permission to cast to this TV, approve? [" ChipLogFormatMEI
                  "," ChipLogFormatMEI ",%s]",
                  state.GetDeviceName(), ChipLogValueMEI(state.GetVendorId()), ChipLogValueMEI(state.GetProductId()),
                  state.GetInstanceName());
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->PromptForCommissionOKPermission(state.GetVendorId(), state.GetProductId(), state.GetDeviceName());
    }
    ChipLogDetail(Controller, "------Via Shell Enter: controller ux ok|cancel");
}

void CommissionerDiscoveryController::Ok()
{
    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX Cancel: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Ok: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok: could not find instance=%s", mCurrentInstance);
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kPromptingUser)
    {
        ChipLogError(AppServer, "UX Ok: invalid state for ok");
        return;
    }
    client->SetUDCClientProcessingState(UDCClientProcessingState::kObtainingOnboardingPayload);

    if (mPincodeService != nullptr)
    {
        char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
        Encoding::BytesToUppercaseHexString(client->GetRotatingId(), client->GetRotatingIdLength(), rotatingIdString,
                                            sizeof(rotatingIdString));
        // Encoding::BytesToUppercaseHexString(client->GetRotatingId(), chip::Dnssd::kMaxRotatingIdLen, rotatingIdString,
        //                                     sizeof(rotatingIdString));

        CharSpan rotatingIdSpan = chip::CharSpan(rotatingIdString, sizeof(rotatingIdString));
        uint32_t pincode =
            mPincodeService->FetchCommissionPincodeFromContentApp(client->GetVendorId(), client->GetProductId(), rotatingIdSpan);
        if (pincode != 0)
        {
            CommissionWithPincode(pincode);
            return;
        }
    }

    ChipLogDetail(Controller, "------PROMPT USER: please enter pin displayed in casting app ");
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->PromptForCommissionPincode(client->GetVendorId(), client->GetProductId(), client->GetDeviceName());
    }
    ChipLogDetail(Controller, "------Via Shell Enter: controller ux ok [pincode]");
    return;
}

void CommissionerDiscoveryController::CommissionWithPincode(uint32_t pincode)
{
    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX Cancel: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX CommissionWithPincode: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok: could not find instance=%s", mCurrentInstance);
        return;
    }
    // state needs to be either kPromptingUser or kObtainingOnboardingPayload
    if (!(client->GetUDCClientProcessingState() == UDCClientProcessingState::kPromptingUser ||
          client->GetUDCClientProcessingState() == UDCClientProcessingState::kObtainingOnboardingPayload))
    {
        ChipLogError(AppServer, "UX CommissionWithPincode: invalid state for CommissionWithPincode");
        return;
    }
    Transport::PeerAddress peerAddress = client->GetPeerAddress();
    client->SetUDCClientProcessingState(UDCClientProcessingState::kCommissioningNode);
    if (mCommissionerCallback != nullptr)
    {
        mCommissionerCallback->ReadyForCommissioning(pincode, client->GetLongDiscriminator(), peerAddress);
    }
}

void CommissionerDiscoveryController::Cancel()
{
    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX Cancel: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Cancel: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr || client->GetUDCClientProcessingState() != UDCClientProcessingState::kPromptingUser)
    {
        ChipLogError(AppServer, "UX Cancel: invalid state for cancel");
        return;
    }
    client->SetUDCClientProcessingState(UDCClientProcessingState::kUserDeclined);
    mPendingConsent = false;
    return;
}
