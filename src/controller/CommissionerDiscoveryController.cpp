/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
#include <controller/CommissionerDiscoveryController.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

using namespace ::chip;
using namespace chip::Protocols::UserDirectedCommissioning;

void CommissionerDiscoveryController::ResetState()
{
    mCurrentInstance[0] = '\0';
    mVendorId           = 0;
    mProductId          = 0;
    mNodeId             = 0;
    mReady              = true;
}

void CommissionerDiscoveryController::ValidateSession()
{
    if (mReady)
    {
        return;
    }
    if (mUdcServer != nullptr)
    {
        UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
        if (client != nullptr)
        {
            // everything looks good
            return;
        }
    }
    ResetState();
}

void CommissionerDiscoveryController::OnCancel(UDCClientState state)
{
    if (mReady)
    {
        // if state was ready for a new session,
        // then we have lost our discovery controller context and can't perform the commissioning request
        ChipLogDetail(Controller, "CommissionerDiscoveryController::OnCancel received when no current instance.");
        return;
    }

    if (strncmp(mCurrentInstance, state.GetInstanceName(), sizeof(mCurrentInstance)) != 0)
    {
        // if the instance doesn't match the one in our discovery controller context,
        // then we can't perform the commissioning request
        ChipLogDetail(Controller, "CommissionerDiscoveryController::OnCancel received mismatched instance. Current instance=%s",
                      mCurrentInstance);
        return;
    }

    ChipLogDetail(Controller, "------PROMPT USER: %s cancelled commissioning [" ChipLogFormatMEI "," ChipLogFormatMEI ",%s]",
                  state.GetDeviceName(), ChipLogValueMEI(state.GetVendorId()), ChipLogValueMEI(state.GetProductId()),
                  state.GetInstanceName());
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->HidePromptsOnCancel(state.GetVendorId(), state.GetProductId(), state.GetDeviceName());
    }
    return;
}

void CommissionerDiscoveryController::OnCommissionerPasscodeReady(UDCClientState state)
{
    if (mReady)
    {
        // if state was ready for a new session,
        // then we have lost our discovery controller context and can't perform the commissioning request
        ChipLogDetail(Controller,
                      "CommissionerDiscoveryController::OnCommissionerPasscodeReady received when no current instance.");
        return;
    }

    if (strncmp(mCurrentInstance, state.GetInstanceName(), sizeof(mCurrentInstance)) != 0)
    {
        // if the instance doesn't match the one in our discovery controller context,
        // then we can't perform the commissioning request
        ChipLogDetail(
            Controller,
            "CommissionerDiscoveryController::OnCommissionerPasscodeReady received mismatched instance. Current instance=%s",
            mCurrentInstance);
        return;
    }

    if (state.GetCdPort() == 0)
    {
        ChipLogDetail(Controller, "CommissionerDiscoveryController::OnCommissionerPasscodeReady no port");
        return;
    }

    uint32_t passcode = state.GetCachedCommissionerPasscode();
    if (passcode == 0)
    {
        ChipLogError(AppServer, "On UDC: commissioner passcode ready but no passcode");
        CommissionerDeclaration cd;
        cd.SetErrorCode(CommissionerDeclaration::CdError::kUnexpectedCommissionerPasscodeReady);

        if (mUdcServer == nullptr)
        {
            ChipLogError(AppServer, "On UDC: no udc server");
            return;
        }
        mUdcServer->SendCDCMessage(cd, Transport::PeerAddress::UDP(state.GetPeerAddress().GetIPAddress(), state.GetCdPort()));
        return;
    }
    else
    {
        // can only get here is ok() has already been called
        ChipLogDetail(AppServer, "On UDC: commissioner passcode ready with passcode - commissioning");

        // start commissioning using the cached passcode
        CommissionWithPasscode(passcode);
        return;
    }
}

void CommissionerDiscoveryController::OnUserDirectedCommissioningRequest(UDCClientState state)
{
    ValidateSession();

    if (!mReady)
    {
        // we must currently have discovery controller context (a UDC prompt under way)
        ChipLogDetail(Controller, "CommissionerDiscoveryController not ready. Current instance=%s", mCurrentInstance);
        return;
    }

    if (state.GetProductId() == 0 && state.GetVendorId() == 0)
    {
        // this is an invalid request and should be ignored
        ChipLogDetail(Controller, "Ignoring the request as it's invalid. product and vendor id cannot be 0");
        return;
    }

    mReady = false;
    Platform::CopyString(mCurrentInstance, state.GetInstanceName());
    mPendingConsent = true;
    char rotatingIdString[Dnssd::kMaxRotatingIdLen * 2 + 1];
    CHIP_ERROR err = Encoding::BytesToUppercaseHexString(state.GetRotatingId(), state.GetRotatingIdLength(), rotatingIdString,
                                                         sizeof(rotatingIdString));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "On UDC: could not convert rotating id to hex");
        rotatingIdString[0] = '\0';
    }
    else
    {
        // Store rotating ID string. Don't include null terminator character.
        mRotatingId = std::string{ rotatingIdString, state.GetRotatingIdLength() * 2 };
    }

    ChipLogDetail(Controller,
                  "------PROMPT USER: %s is requesting permission to cast to this TV, approve? [" ChipLogFormatMEI
                  "," ChipLogFormatMEI ",%s,%s]",
                  state.GetDeviceName(), ChipLogValueMEI(state.GetVendorId()), ChipLogValueMEI(state.GetProductId()),
                  state.GetInstanceName(), rotatingIdString);
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->PromptForCommissionOKPermission(state.GetVendorId(), state.GetProductId(), state.GetDeviceName());
    }
    ChipLogDetail(Controller, "------Via Shell Enter: controller ux ok|cancel");
}

/// Callback for getting execution into the main chip thread
void CallbackOk(System::Layer * aSystemLayer, void * aAppState)
{
    ChipLogDetail(AppServer, "UX Ok: now on main thread");
    CommissionerDiscoveryController * cdc = static_cast<CommissionerDiscoveryController *>(aAppState);
    cdc->InternalOk();
}

void CommissionerDiscoveryController::Ok()
{
    ChipLogDetail(AppServer, "UX Ok: moving to main thread");
    assertChipStackLockedByCurrentThread();
    if (CHIP_NO_ERROR != DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(0), CallbackOk, this))
    {
        ChipLogError(AppServer, "UX Ok: StartTimer returned error");
    }
}

void CommissionerDiscoveryController::InternalOk()
{
    ChipLogDetail(AppServer, "UX InternalOk");
    assertChipStackLockedByCurrentThread();
    ValidateSession();

    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX InternalOk: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX InternalOk: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX InternalOk: could not find instance=%s", mCurrentInstance);
        return;
    }

    if (mAppInstallationService == nullptr)
    {
        ChipLogError(AppServer, "UX InternalOk: no app installation service");
        return;
    }

    if (!mAppInstallationService->LookupTargetContentApp(client->GetVendorId(), client->GetProductId()))
    {
        ChipLogDetail(AppServer, "UX InternalOk: app not installed.");
    }

    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kPromptingUser)
    {
        ChipLogError(AppServer, "UX InternalOk: invalid state for ok");
        return;
    }
    client->SetUDCClientProcessingState(UDCClientProcessingState::kObtainingOnboardingPayload);

    if (mPasscodeService == nullptr)
    {
        HandleContentAppPasscodeResponse(0);
        return;
    }

    char rotatingIdBuffer[Dnssd::kMaxRotatingIdLen * 2];
    size_t rotatingIdLength = client->GetRotatingIdLength();
    CHIP_ERROR err =
        Encoding::BytesToUppercaseHexBuffer(client->GetRotatingId(), rotatingIdLength, rotatingIdBuffer, sizeof(rotatingIdBuffer));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "UX InternalOk: could not convert rotating id to hex");
        return;
    }
    CharSpan rotatingIdSpan(rotatingIdBuffer, 2 * rotatingIdLength);

    uint8_t targetAppCount = client->GetNumTargetAppInfos();

    if (targetAppCount > 0)
    {
        ChipLogDetail(AppServer, "UX InternalOk: checking for each target app specified");

        for (uint8_t i = 0; i < targetAppCount; i++)
        {
            TargetAppInfo info;
            if (client->GetTargetAppInfo(i, info))
            {
                if (mPasscodeService != nullptr)
                {
                    mPasscodeService->LookupTargetContentApp(client->GetVendorId(), client->GetProductId(), rotatingIdSpan, info);
                }
            }
        }
        return;
    }
    ChipLogDetail(AppServer, "UX InternalOk: checking target app associated with client");

    if (mPasscodeService != nullptr)
    {
        mPasscodeService->FetchCommissionPasscodeFromContentApp(client->GetVendorId(), client->GetProductId(), rotatingIdSpan);
    }

    ChipLogDetail(AppServer, "UX Ok: done moving out of main thread");
}

void CommissionerDiscoveryController::HandleTargetContentAppCheck(TargetAppInfo target, uint32_t passcode)
{
    assertChipStackLockedByCurrentThread();
    ValidateSession();

    bool foundTargetApp      = false;
    bool foundPendingTargets = false;

    /**
     * Update our target app list with the status from this target.
     *
     * If we are the first callback to receive a passcode,
     *  then complete commissioning with it.
     * If we are the last expected callback and none has completed commissioning,
     *  then advance to the next step for trying to obtain a passcode.
     * When iterating through the list of targets, keep track of whether any apps have been found,
     *  so that if we advance we can do so with that information (may need to send a CDC).
     */

    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: could not find instance=%s", mCurrentInstance);
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kObtainingOnboardingPayload)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppCheck: invalid state for HandleContentAppPasscodeResponse");
        return;
    }

    uint8_t targetAppCount = client->GetNumTargetAppInfos();
    for (uint8_t i = 0; i < targetAppCount; i++)
    {
        TargetAppInfo info;
        if (client->GetTargetAppInfo(i, info))
        {
            if (info.checkState == TargetAppCheckState::kAppFoundPasscodeReturned)
            {
                // nothing else to do, complete commissioning has been called
                return;
            }
            else if (info.checkState == TargetAppCheckState::kAppFoundNoPasscode)
            {
                foundTargetApp = true;
            }
            else if (info.checkState == TargetAppCheckState::kNotInitialized)
            {
                if (target.vendorId == info.vendorId && target.productId == info.productId)
                {
                    client->SetTargetAppInfoState(i, target.checkState);
                    if (target.checkState != TargetAppCheckState::kAppNotFound)
                    {
                        foundTargetApp = true;
                    }
                }
                else
                {
                    foundPendingTargets = true;
                }
            }
        }
    }
    if (passcode != 0)
    {
        ChipLogDetail(AppServer, "UX Ok - HandleContentAppCheck: found a passcode");
        // we found a passcode and complete commissioning has not been called
        CommissionWithPasscode(passcode);
        return;
    }
    if (foundPendingTargets)
    {
        ChipLogDetail(AppServer, "UX Ok - HandleContentAppCheck: have not heard from all apps");
        // have not heard from all targets so don't do anything
        return;
    }
    if (!foundTargetApp && client->GetNoPasscode())
    {
        // finished iterating through all apps and found none, send CDC
        ChipLogDetail(AppServer, "UX Ok - HandleContentAppCheck: target apps specified but none found, sending CDC");
        CommissionerDeclaration cd;
        cd.SetNoAppsFound(true);
        mUdcServer->SendCDCMessage(cd, Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
    }
    ChipLogDetail(AppServer, "UX Ok - HandleContentAppCheck: advancing");
    // otherwise, advance to the next step for trying to obtain a passcode.
    HandleContentAppPasscodeResponse(0);
    return;
}

/// Callback for getting execution into the main chip thread
void CallbackHandleContentAppPasscodeResponse(System::Layer * aSystemLayer, void * aAppState)
{
    ChipLogDetail(AppServer, "HandleContentAppPasscodeResponse: now on main thread");
    CommissionerDiscoveryController * cdc = static_cast<CommissionerDiscoveryController *>(aAppState);
    cdc->InternalHandleContentAppPasscodeResponse();
}

void CommissionerDiscoveryController::HandleContentAppPasscodeResponse(uint32_t passcode)
{
    assertChipStackLockedByCurrentThread();

    SetPasscode(passcode);
    if (CHIP_NO_ERROR !=
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(0), CallbackHandleContentAppPasscodeResponse, this))
    {
        ChipLogError(AppServer, "HandleContentAppPasscodeResponse: StartTimer returned error");
    }
}

void CommissionerDiscoveryController::InternalHandleContentAppPasscodeResponse()
{
    assertChipStackLockedByCurrentThread();
    ValidateSession();
    uint32_t passcode = mPasscode;

    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: could not find instance=%s", mCurrentInstance);
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kObtainingOnboardingPayload)
    {
        ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: invalid state for HandleContentAppPasscodeResponse");
        return;
    }

    if (mPasscodeService != nullptr)
    {
        // if CommissionerPasscode
        //    - if CommissionerPasscodeReady, then start commissioning
        //    - if CommissionerPasscode, then call new UX method to show passcode, send CDC
        if (passcode == 0 && client->GetCommissionerPasscode() && client->GetCdPort() != 0)
        {
            char rotatingIdBuffer[Dnssd::kMaxRotatingIdLen * 2];
            size_t rotatingIdLength = client->GetRotatingIdLength();
            CHIP_ERROR err = Encoding::BytesToUppercaseHexBuffer(client->GetRotatingId(), rotatingIdLength, rotatingIdBuffer,
                                                                 sizeof(rotatingIdBuffer));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "UX Ok - HandleContentAppPasscodeResponse: could not convert rotating id to hex");
                return;
            }
            CharSpan rotatingIdSpan(rotatingIdBuffer, 2 * rotatingIdLength);

            // first step of commissioner passcode
            ChipLogError(AppServer, "UX Ok: commissioner passcode, sending CDC");
            // generate a passcode
            passcode = mPasscodeService->GetCommissionerPasscode(client->GetVendorId(), client->GetProductId(), rotatingIdSpan);
            if (passcode == 0)
            {
                // passcode feature disabled
                ChipLogError(AppServer, "UX Ok: commissioner passcode disabled, sending CDC with error");
                CommissionerDeclaration cd;
                cd.SetErrorCode(CommissionerDeclaration::CdError::kCommissionerPasscodeDisabled);
                cd.SetNeedsPasscode(true);
                mUdcServer->SendCDCMessage(
                    cd, Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
                return;
            }

            client->SetCachedCommissionerPasscode(passcode);
            client->SetUDCClientProcessingState(UDCClientProcessingState::kWaitingForCommissionerPasscodeReady);

            CommissionerDeclaration cd;
            cd.SetCommissionerPasscode(true);
            if (mUserPrompter->DisplaysPasscodeAndQRCode())
            {
                cd.SetQRCodeDisplayed(true);
            }
            mUdcServer->SendCDCMessage(cd,
                                       Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));

            // dialog
            ChipLogDetail(Controller,
                          "------PROMPT USER: %s is requesting permission to cast to this TV. Casting passcode: [" ChipLogFormatMEI
                          "]. Additional instructions [" ChipLogFormatMEI "] [%s]. [" ChipLogFormatMEI "," ChipLogFormatMEI ",%s]",
                          client->GetDeviceName(), ChipLogValueMEI(passcode), ChipLogValueMEI(client->GetPairingHint()),
                          client->GetPairingInst(), ChipLogValueMEI(client->GetVendorId()), ChipLogValueMEI(client->GetProductId()),
                          client->GetInstanceName());
            mUserPrompter->PromptWithCommissionerPasscode(client->GetVendorId(), client->GetProductId(), client->GetDeviceName(),
                                                          passcode, client->GetPairingHint(), client->GetPairingInst());
            return;
        }
        if (passcode != 0)
        {
            CommissionWithPasscode(passcode);
            return;
        }
    }

    // if NoPasscode, send CDC
    if (client->GetNoPasscode() && client->GetCdPort() != 0)
    {
        ChipLogDetail(AppServer, "UX Ok: no app passcode and NoPasscode in UDC, sending CDC");
        CommissionerDeclaration cd;
        cd.SetNeedsPasscode(true);
        mUdcServer->SendCDCMessage(cd, Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
        return;
    }

    // if CdUponPasscodeDialog, send CDC
    if (client->GetCdUponPasscodeDialog() && client->GetCdPort() != 0)
    {
        ChipLogDetail(AppServer, "UX Ok: no app passcode and GetCdUponPasscodeDialog in UDC, sending CDC");
        CommissionerDeclaration cd;
        cd.SetNeedsPasscode(true); // TODO: should this be set?
        cd.SetPasscodeDialogDisplayed(true);
        mUdcServer->SendCDCMessage(cd, Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
    }

    ChipLogDetail(Controller, "------PROMPT USER: please enter passcode displayed in casting app ");
    if (mUserPrompter != nullptr)
    {
        mUserPrompter->PromptForCommissionPasscode(client->GetVendorId(), client->GetProductId(), client->GetDeviceName(),
                                                   client->GetPairingHint(), client->GetPairingInst());
    }
    ChipLogDetail(Controller, "------Via Shell Enter: controller ux ok [passcode]");
}

/// Callback for getting execution into the main chip thread
void CallbackCommissionWithPasscode(System::Layer * aSystemLayer, void * aAppState)
{
    ChipLogDetail(AppServer, "CallbackCommissionWithPasscode: now on main thread");
    CommissionerDiscoveryController * cdc = static_cast<CommissionerDiscoveryController *>(aAppState);
    cdc->InternalCommissionWithPasscode();
}

void CommissionerDiscoveryController::CommissionWithPasscode(uint32_t passcode)
{
    assertChipStackLockedByCurrentThread();

    SetPasscode(passcode);
    if (CHIP_NO_ERROR != DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(0), CallbackCommissionWithPasscode, this))
    {
        ChipLogError(AppServer, "CommissionWithPasscode: StartTimer returned error");
    }
}

void CommissionerDiscoveryController::InternalCommissionWithPasscode()
{
    assertChipStackLockedByCurrentThread();
    ValidateSession();
    uint32_t passcode = mPasscode;

    if (!mPendingConsent)
    {
        ChipLogError(AppServer, "UX CommissionWithPasscode: no current instance");
        return;
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX CommissionWithPasscode: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX CommissionWithPasscode: could not find instance=%s", mCurrentInstance);
        return;
    }
    // state needs to be either kPromptingUser or kObtainingOnboardingPayload
    if (!(client->GetUDCClientProcessingState() == UDCClientProcessingState::kPromptingUser ||
          client->GetUDCClientProcessingState() == UDCClientProcessingState::kObtainingOnboardingPayload))
    {
        ChipLogError(AppServer, "UX CommissionWithPasscode: invalid state for CommissionWithPasscode");
        return;
    }
    Transport::PeerAddress peerAddress = client->GetPeerAddress();
    client->SetUDCClientProcessingState(UDCClientProcessingState::kCommissioningNode);
    if (mCommissionerCallback != nullptr)
    {
        if (mUserPrompter != nullptr)
        {
            mUserPrompter->PromptCommissioningStarted(client->GetVendorId(), client->GetProductId(), client->GetDeviceName());
        }
        mCommissionerCallback->ReadyForCommissioning(passcode, client->GetLongDiscriminator(), peerAddress);
    }
}

void CommissionerDiscoveryController::Cancel()
{
    ValidateSession();

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

    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX Cancel: client not found");
        return;
    }

    auto state = client->GetUDCClientProcessingState();

    bool isCancelableState =
        (state == UDCClientProcessingState::kPromptingUser || state == UDCClientProcessingState::kObtainingOnboardingPayload ||
         state == UDCClientProcessingState::kWaitingForCommissionerPasscodeReady);

    if (!isCancelableState)
    {
        ChipLogError(AppServer, "UX Cancel: invalid state for cancel, state: %hhu", static_cast<uint8_t>(state));
        return;
    }

    client->SetUDCClientProcessingState(UDCClientProcessingState::kUserDeclined);

    if (state == UDCClientProcessingState::kObtainingOnboardingPayload ||
        state == UDCClientProcessingState::kWaitingForCommissionerPasscodeReady)
    {
        ChipLogDetail(AppServer, "UX Cancel: user cancelled entering PIN code, sending CDC");
        CommissionerDeclaration cd;
        cd.SetCancelPasscode(true);
        mUdcServer->SendCDCMessage(cd, Transport::PeerAddress::UDP(client->GetPeerAddress().GetIPAddress(), client->GetCdPort()));
    }

    mPendingConsent = false;
    ResetState();
}

void CommissionerDiscoveryController::CommissioningSucceeded(uint16_t vendorId, uint16_t productId, NodeId nodeId,
                                                             Messaging::ExchangeManager & exchangeMgr,
                                                             const SessionHandle & sessionHandle)
{
    mVendorId  = vendorId;
    mProductId = productId;
    mNodeId    = nodeId;

    if (mPostCommissioningListener != nullptr)
    {
        ChipLogDetail(Controller, "CommissionerDiscoveryController calling listener");
        auto rotatingIdSpan = CharSpan{ mRotatingId.data(), mRotatingId.size() };
        mPostCommissioningListener->CommissioningCompleted(vendorId, productId, nodeId, rotatingIdSpan, mPasscode, exchangeMgr,
                                                           sessionHandle);
    }
    else
    {
        PostCommissioningSucceeded();
    }
}

void CommissionerDiscoveryController::CommissioningFailed(CHIP_ERROR error)
{
    if (mUserPrompter != nullptr)
    {
        ChipLogDetail(Controller, "------PROMPT USER: commissioning failed ");
        mUserPrompter->PromptCommissioningFailed(GetCommissioneeName(), error);
    }
    if (mUdcServer == nullptr)
    {
        ChipLogError(AppServer, "UX CommissioningFailed: no udc server");
        return;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "UX CommissioningFailed: no client");
        return;
    }
    if (client->GetUDCClientProcessingState() != UDCClientProcessingState::kCommissioningNode)
    {
        ChipLogError(AppServer, "UX CommissioningFailed: invalid state");
        return;
    }
    client->SetUDCClientProcessingState(UDCClientProcessingState::kCommissioningFailed);
    client->Reset();

    ResetState();
}

void CommissionerDiscoveryController::PostCommissioningSucceeded()
{
    if (mUserPrompter != nullptr)
    {
        ChipLogDetail(Controller, "------PROMPT USER: commissioning success ");
        mUserPrompter->PromptCommissioningSucceeded(mVendorId, mProductId, GetCommissioneeName());
    }
    ResetState();
}

void CommissionerDiscoveryController::PostCommissioningFailed(CHIP_ERROR error)
{
    if (mUserPrompter != nullptr)
    {
        ChipLogDetail(Controller, "------PROMPT USER: post-commissioning failed ");
        mUserPrompter->PromptCommissioningFailed(GetCommissioneeName(), error);
    }
    ResetState();
}

const char * CommissionerDiscoveryController::GetCommissioneeName()
{
    if (mReady)
    {
        // no current commissionee
        ChipLogError(AppServer, "CommissionerDiscoveryController no current commissionee");
        return nullptr;
    }
    UDCClientState * client = mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
    if (client == nullptr)
    {
        ChipLogError(AppServer, "CommissionerDiscoveryController no UDCState for instance=%s", mCurrentInstance);
        return nullptr;
    }
    return client->GetDeviceName();
}

UDCClientState * CommissionerDiscoveryController::GetUDCClientState()
{
    if (mReady)
    {
        // no current commissionee
        ChipLogError(AppServer, "CommissionerDiscoveryController no current commissionee");
        return nullptr;
    }
    return mUdcServer->GetUDCClients().FindUDCClientState(mCurrentInstance);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
