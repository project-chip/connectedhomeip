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

#pragma once

#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

using chip::NodeId;
using chip::OperationalSessionSetup;
using chip::Protocols::UserDirectedCommissioning::UDCClientState;
using chip::Protocols::UserDirectedCommissioning::UserConfirmationProvider;
using chip::Protocols::UserDirectedCommissioning::UserDirectedCommissioningServer;
using chip::Transport::PeerAddress;

class DLL_EXPORT UserPrompter
{
public:
    /**
     * @brief
     *   Called to prompt the user for consent to allow the given commissioneeName/vendorId/productId to be commissioned.
     * For example "[commissioneeName] is requesting permission to cast to this TV, approve?"
     *
     * If user responds with OK then implementor should call CommissionerRespondOk();
     * If user responds with Cancel then implementor should call CommissionerRespondCancel();
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptForCommissionOKPermission(uint16_t vendorId, uint16_t productId, const char * commissioneeName) = 0;

    /**
     * @brief
     *   Called to prompt the user to enter the setup passcode displayed by the given commissioneeName/vendorId/productId to be
     * commissioned. For example "Please enter passcode displayed in casting app."
     *
     * If user enters passcode then implementor should call CommissionerRespondPasscode(uint32_t passcode);
     * If user responds with Cancel then implementor should call CommissionerRespondCancel();
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    pairingHint        The pairingHint in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    pairingInstruction The pairingInstruction in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptForCommissionPasscode(uint16_t vendorId, uint16_t productId, const char * commissioneeName,
                                             uint16_t pairingHint, const char * pairingInstruction) = 0;

    /**
     * @brief
     *   Called to when CancelCommissioning is received via UDC.
     * Indicates that commissioner can stop showing the passcode entry or display dialog.
     * For example, can show text such as "Commissioning cancelled by client" before hiding dialog.
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void HidePromptsOnCancel(uint16_t vendorId, uint16_t productId, const char * commissioneeName) = 0;

    /**
     * @brief
     *   Return true if this UserPrompter displays QR code along with passcode
     * When PromptWithCommissionerPasscode is called during Commissioner Passcode functionality.
     */
    virtual bool DisplaysPasscodeAndQRCode() = 0;

    /**
     * @brief
     *   Called to display the given setup passcode to the user,
     * for commissioning the given commissioneeName with the given vendorId and productId,
     * and provide instructions for where to enter it in the commissionee (when pairingHint and pairingInstruction are provided).
     * For example "Casting Passcode: [passcode]. For more instructions, click here."
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    passcode           The passcode to display.
     *  @param[in]    pairingHint        The pairingHint in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    pairingInstruction The pairingInstruction in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptWithCommissionerPasscode(uint16_t vendorId, uint16_t productId, const char * commissioneeName,
                                                uint32_t passcode, uint16_t pairingHint, const char * pairingInstruction) = 0;

    /**
     * @brief
     *   Called to alert the user that commissioning has begun."
     *
     *  @param[in]    vendorId           The vendorid from the DAC of the new node.
     *  @param[in]    productId          The productid from the DAC of the new node.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptCommissioningStarted(uint16_t vendorId, uint16_t productId, const char * commissioneeName) = 0;

    /**
     * @brief
     *   Called to prompt the user that commissioning and post-commissioning steps have completed successfully."
     *
     *  @param[in]    vendorId           The vendorid from the DAC of the new node.
     *  @param[in]    productId          The productid from the DAC of the new node.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptCommissioningSucceeded(uint16_t vendorId, uint16_t productId, const char * commissioneeName) = 0;

    /**
     * @brief
     *   Called to prompt the user that commissioning and post-commissioning steps have failed."
     *
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptCommissioningFailed(const char * commissioneeName, CHIP_ERROR error) = 0;

    virtual ~UserPrompter() = default;
};

class DLL_EXPORT PasscodeService
{
public:
    /**
     * @brief
     *   Called to determine if the given target app is available to the commissionee with the given
     * vendorId/productId, and if so, return the passcode.
     *
     * This will be called by the main chip thread so any blocking work should be moved to a separate thread.
     *
     * After lookup and attempting to obtain the passcode, implementor should call HandleContentAppCheck();
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    rotatingId         The rotatingId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    info               App info to look for.
     *
     */
    virtual void LookupTargetContentApp(uint16_t vendorId, uint16_t productId, chip::CharSpan rotatingId,
                                        chip::Protocols::UserDirectedCommissioning::TargetAppInfo & info) = 0;

    /**
     * @brief
     *   Called to get the commissioner-generated setup passcode.
     * Returns 0 if feature is disabled.
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    rotatingId         The rotatingId in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual uint32_t GetCommissionerPasscode(uint16_t vendorId, uint16_t productId, chip::CharSpan rotatingId) = 0;

    /**
     * @brief
     *   Called to get the setup passcode from the content app corresponding to the given vendorId/productId.
     *
     * This will be called by the main chip thread so any blocking work should be moved to a separate thread.
     *
     * After attempting to obtain the passcode, implementor should call HandleContentAppPasscodeResponse();
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    rotatingId         The rotatingId in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void FetchCommissionPasscodeFromContentApp(uint16_t vendorId, uint16_t productId, chip::CharSpan rotatingId) = 0;

    virtual ~PasscodeService() = default;
};

class DLL_EXPORT AppInstallationService
{
public:
    /**
     * @brief
     *   Called to check if the given target app is available to the commissione with th given
     *   vendorId/productId
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual bool LookupTargetContentApp(uint16_t vendorId, uint16_t productId) = 0;

    virtual ~AppInstallationService() = default;
};

class DLL_EXPORT PostCommissioningListener
{
public:
    /**
     * @brief
     *   Called to when commissioning completed to allow the listener to perform additional
     * steps such as binding and ACL creation.
     *
     *  @param[in]    vendorId           The vendorid from the DAC of the new node.
     *  @param[in]    productId          The productid from the DAC of the new node.
     *  @param[in]    nodeId             The node id for the newly commissioned node.
     *  @param[in]    rotatingId         The rotating ID to handle account login.
     *  @param[in]    passcode           The passcode to handle account login.
     *  @param[in]    exchangeMgr        The exchange manager to be used to get an exchange context.
     *  @param[in]    sessionHandle      A reference to an established session.
     *
     */
    virtual void CommissioningCompleted(uint16_t vendorId, uint16_t productId, NodeId nodeId, chip::CharSpan rotatingId,
                                        uint32_t passcode, chip::Messaging::ExchangeManager & exchangeMgr,
                                        const chip::SessionHandle & sessionHandle) = 0;

    virtual ~PostCommissioningListener() = default;
};

class DLL_EXPORT CommissionerCallback
{
public:
    /**
     * @brief
     *   Called to notify the commissioner that commissioning can now proceed for
     * the node identified by the given arguments.
     *
     *  @param[in]    passcode            The passcode to use for the commissionee.
     *  @param[in]    longDiscriminator   The long discriminator for the commissionee.
     *  @param[in]    peerAddress         The peerAddress for the commissionee.
     *
     */
    virtual void ReadyForCommissioning(uint32_t passcode, uint16_t longDiscriminator, PeerAddress peerAddress) = 0;

    virtual ~CommissionerCallback() = default;
};

class CommissionerDiscoveryController : public chip::Protocols::UserDirectedCommissioning::UserConfirmationProvider
{
public:
    /**
     * This controller can only handle one outstanding UDC session at a time and will
     * reject attempts to start a second when one is outstanding.
     *
     * A session ends when post-commissioning completes:
     * - PostCommissioningSucceeded()
     * or when one of the following failure methods is called:
     * - PostCommissioningFailed()
     * - CommissioningFailed()
     *
     * Reset the state of this controller so that a new sessions will be accepted.
     */
    void ResetState();

    /**
     * Check whether we have a valid session (and reset state if not).
     */
    void ValidateSession();

    /**
     * UserConfirmationProvider callback.
     *
     * Notification that a UDC protocol message was received.
     *
     * This code will call the registered UserPrompter's PromptForCommissionOKPermission
     */
    void OnUserDirectedCommissioningRequest(UDCClientState state) override;

    /**
     * UserConfirmationProvider callback.
     *
     * Notification that a Cancel UDC protocol message was received.
     *
     * This code will call the registered UserPrompter's HidePromptsOnCancel
     */
    void OnCancel(UDCClientState state) override;

    /**
     * UserConfirmationProvider callback.
     *
     * Notification that a CommissionerPasscodeReady UDC protocol message was received.
     *
     * This code will trigger the Commissioner to begin commissioning
     */
    void OnCommissionerPasscodeReady(UDCClientState state) override;

    /**
     * This method should be called after the user has given consent for commissioning of the client
     * indicated in the UserPrompter's PromptForCommissionOKPermission callback
     */
    void Ok();
    void InternalOk();

    /**
     * This method should be called after the user has declined to give consent for commissioning of the client
     * indicated in the UserPrompter's PromptForCommissionOKPermission callback
     */
    void Cancel();

    /**
     * @brief
     *   Called with the result of attempting to obtain the passcode from the content app corresponding to the given
     * vendorId/productId.
     *
     *  @param[in]    passcode           Passcode for the given commissionee, or 0 if passcode cannot be obtained.
     *
     */
    void HandleContentAppPasscodeResponse(uint32_t passcode);
    void InternalHandleContentAppPasscodeResponse();

    /**
     * Cache the passcode to use for commissioning
     */
    inline void SetPasscode(uint32_t passcode) { mPasscode = passcode; }

    /**
     * @brief
     *   Called with the result of attempting to lookup and obtain the passcode from the content app corresponding to the given
     * target.
     *
     *  @param[in]    target             Target app info for app check.
     *  @param[in]    passcode           Passcode for the given commissionee, or 0 if passcode cannot be obtained.
     *
     */
    void HandleTargetContentAppCheck(chip::Protocols::UserDirectedCommissioning::TargetAppInfo target, uint32_t passcode);

    /**
     * This method should be called with the passcode for the client
     * indicated in the UserPrompter's PromptForCommissionPasscode callback
     */
    void CommissionWithPasscode(uint32_t passcode);
    void InternalCommissionWithPasscode();

    /**
     * This method should be called by the commissioner to indicate that commissioning succeeded.
     * The PostCommissioningCallback will then be invoked to complete setup
     *
     *  @param[in]    vendorId           The vendorid from the DAC of the new node.
     *  @param[in]    productId          The productid from the DAC of the new node.
     *  @param[in]    nodeId             The node id for the newly commissioned node.
     *  @param[in]    exchangeMgr        The exchange manager to be used to get an exchange context.
     *  @param[in]    sessionHandle      A reference to an established session.
     *
     */
    void CommissioningSucceeded(uint16_t vendorId, uint16_t productId, NodeId nodeId,
                                chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);

    /**
     * This method should be called by the commissioner to indicate that commissioning failed.
     * The UserPrompter will then be invoked to notify the user of the failure.
     */
    void CommissioningFailed(CHIP_ERROR error);

    /**
     * This method should be called by the PostCommissioningListener to indicate that post-commissioning steps completed.
     * The PromptCommissioningSucceeded will then be invoked to notify the user of success.
     */
    void PostCommissioningSucceeded();

    /**
     * This method should be called by the PostCommissioningListener to indicate that post-commissioning steps failed.
     * The PromptCommissioningFailed will then be invoked to notify the user of failure.
     */
    void PostCommissioningFailed(CHIP_ERROR error);

    /**
     * Assign a DeviceCommissioner
     */
    inline void SetUserDirectedCommissioningServer(UserDirectedCommissioningServer * udcServer)
    {
        mUdcServer = udcServer;
        mUdcServer->SetUserConfirmationProvider(this);
    }

    /**
     * Assign a UserPromper
     */
    inline void SetUserPrompter(UserPrompter * userPrompter) { mUserPrompter = userPrompter; }

    /**
     * Assign a PasscodeService
     */
    inline void SetPasscodeService(PasscodeService * passcodeService) { mPasscodeService = passcodeService; }
    inline PasscodeService * GetPasscodeService() { return mPasscodeService; }

    /**
     * Assign an AppInstallationService
     */
    inline void SetAppInstallationService(AppInstallationService * appInstallationService)
    {
        mAppInstallationService = appInstallationService;
    }

    /**
     * Assign a Commissioner Callback to perform commissioning once user consent has been given
     */
    inline void SetCommissionerCallback(CommissionerCallback * commissionerCallback)
    {
        mCommissionerCallback = commissionerCallback;
    }

    /**
     * Assign a PostCommissioning Listener to perform post-commissioning operations
     */
    inline void SetPostCommissioningListener(PostCommissioningListener * postCommissioningListener)
    {
        mPostCommissioningListener = postCommissioningListener;
    }

    /**
     * Get the commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     */
    const char * GetCommissioneeName();

    /**
     * Get the UDCClientState of the requesting commissionee.
     */
    UDCClientState * GetUDCClientState();

protected:
    bool mReady          = true; // ready to start commissioning
    bool mPendingConsent = false;
    char mCurrentInstance[chip::Dnssd::Commission::kInstanceNameMaxLength + 1];
    uint16_t mVendorId  = 0;
    uint16_t mProductId = 0;
    NodeId mNodeId      = 0;
    uint32_t mPasscode  = 0;
    std::string mRotatingId;

    UserDirectedCommissioningServer * mUdcServer           = nullptr;
    UserPrompter * mUserPrompter                           = nullptr;
    PasscodeService * mPasscodeService                     = nullptr;
    AppInstallationService * mAppInstallationService       = nullptr;
    CommissionerCallback * mCommissionerCallback           = nullptr;
    PostCommissioningListener * mPostCommissioningListener = nullptr;
};

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
