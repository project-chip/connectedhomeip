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

#include <iostream>
#include <thread>

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

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
     *   Called to prompt the user to enter the setup pincode displayed by the given commissioneeName/vendorId/productId to be
     * commissioned. For example "Please enter pin displayed in casting app."
     *
     * If user enters with pin then implementor should call CommissionerRespondPincode(uint32_t pincode);
     * If user responds with Cancel then implementor should call CommissionerRespondCancel();
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    commissioneeName   The commissioneeName in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual void PromptForCommissionPincode(uint16_t vendorId, uint16_t productId, const char * commissioneeName) = 0;

    virtual ~UserPrompter() = default;
};

class DLL_EXPORT PincodeService
{
public:
    /**
     * @brief
     *   Called to get the setup pincode from the content app corresponding to the given vendorId/productId
     * Returns 0 if pincode cannot be obtained
     *
     * If user responds with OK then implementor should call CommissionerRespondOk();
     * If user responds with Cancel then implementor should call CommissionerRespondCancel();
     *
     *  @param[in]    vendorId           The vendorId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    productId          The productId in the DNS-SD advertisement of the requesting commissionee.
     *  @param[in]    rotatingId         The rotatingId in the DNS-SD advertisement of the requesting commissionee.
     *
     */
    virtual uint32_t FetchCommissionPincodeFromContentApp(uint16_t vendorId, uint16_t productId, chip::CharSpan rotatingId) = 0;

    virtual ~PincodeService() = default;
};

class DLL_EXPORT CommissionerCallback
{
public:
    /**
     * @brief
     *   Called to notify the commissioner that commissioning can now proceed for
     * the node identified by the given arguments.
     *
     *  @param[in]    pincode             The pin code to use for the commissionee.
     *  @param[in]    longDiscriminator   The long discriminator for the commissionee.
     *  @param[in]    peerAddress         The peerAddress for the commissionee.
     *
     */
    virtual void ReadyForCommissioning(uint32_t pincode, uint16_t longDiscriminator, PeerAddress peerAddress) = 0;

    virtual ~CommissionerCallback() = default;
};

class CommissionerDiscoveryController : public chip::Protocols::UserDirectedCommissioning::UserConfirmationProvider
{
public:
    /**
     * UserConfirmationProvider callback.
     *
     * Notification that a UDC protocol message was received.
     *
     * This code will call the registered UserPrompter's PromptForCommissionOKPermission
     */
    void OnUserDirectedCommissioningRequest(UDCClientState state);

    /**
     * This method should be called after the user has given consent for commissioning of the client
     * indicated in the UserPrompter's PromptForCommissionOKPermission callback
     */
    void Ok();

    /**
     * This method should be called after the user has declined to give consent for commissioning of the client
     * indicated in the UserPrompter's PromptForCommissionOKPermission callback
     */
    void Cancel();

    /**
     * This method should be called with the pincode for the client
     * indicated in the UserPrompter's PromptForCommissionPincode callback
     */
    void CommissionWithPincode(uint32_t pincode);

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
     * Assign a PincodeService
     */
    inline void SetPincodeService(PincodeService * pincodeService) { mPincodeService = pincodeService; }

    /**
     * Assign a Commissioner Callback to perform commissioning once user consent has been given
     */
    inline void SetCommissionerCallback(CommissionerCallback * commissionerCallback)
    {
        mCommissionerCallback = commissionerCallback;
    }

protected:
    bool mPendingConsent = false;
    char mCurrentInstance[chip::Dnssd::Commission::kInstanceNameMaxLength + 1];
    UserDirectedCommissioningServer * mUdcServer = nullptr;
    UserPrompter * mUserPrompter                 = nullptr;
    PincodeService * mPincodeService             = nullptr;
    CommissionerCallback * mCommissionerCallback = nullptr;
};
