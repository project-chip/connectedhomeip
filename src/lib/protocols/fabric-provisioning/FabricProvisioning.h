/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the Fabric Provisioning Profile, used to
 *      manage membership to CHIP Fabrics.
 *
 *      The Fabric Provisioning Profile facilitates client-server operations
 *      such that the client (the controlling device) can trigger specific
 *      functionality on the server (the device undergoing provisioning),
 *      to allow it to create, join, and leave CHIP Fabrics.  This includes
 *      communicating Fabric configuration information such as identifiers,
 *      keys, security schemes, and related data.
 */

#ifndef FABRICPROVISIONING_H_
#define FABRICPROVISIONING_H_

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <message/CHIPServerBase.h>
#include <support/DLLUtil.h>

/**
 *   @namespace chip::Protocols::FabricProvisioning
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP Fabric Provisioning profile, the second of the three
 *     CHIP provisioning profiles.
 */

namespace chip {
namespace Protocols {
namespace FabricProvisioning {

using Inet::IPAddress;

/**
 * Fabric Provisioning Status Codes
 */
enum
{
    kStatusCode_AlreadyMemberOfFabric = 1, /**< The recipient is already a member of a fabric. */
    kStatusCode_NotMemberOfFabric     = 2, /**< The recipient is not a member of a fabric. */
    kStatusCode_InvalidFabricConfig   = 3  /**< The specified fabric configuration was invalid. */
};

/**
 * Fabric Provisioning Message Types
 */
enum
{
    // Application/Device Messages
    kMsgType_CreateFabric            = 1,
    kMsgType_LeaveFabric             = 2,
    kMsgType_GetFabricConfig         = 3,
    kMsgType_GetFabricConfigComplete = 4,
    kMsgType_JoinExistingFabric      = 5
};

/**
 * Fabric Provisioning Data Element Tags
 */
enum
{
    // ---- Top-level Data Elements ----
    kTag_FabricConfig = 1, /**< [ structure ] Contains provisioning information for an existing fabric.
                                  IMPORTANT NOTE: As a convenience to readers, all elements in a FabricConfig
                                  must be encoded in numeric tag order, at all levels. */

    // ---- Context-specific Tags for FabricConfig Structure ----
    kTag_FabricId   = 1, /**< [ uint ] Fabric ID. */
    kTag_FabricKeys = 2, /**< [ array ] List of FabricKey structures. */

    // ---- Context-specific Tags for FabricKey Structure ----
    kTag_FabricKeyId    = 1,       /**< [ uint ] CHIP key ID for fabric key. */
    kTag_EncryptionType = 2,       /**< [ uint ] CHIP encryption type supported by the key. */
    kTag_DataKey        = 3,       /**< [ byte-string ] Data encryption key. */
    kTag_IntegrityKey   = 4,       /**< [ byte-string ] Data integrity key. */
    kTag_KeyScope       = 5,       /**< [ uint ] Enumerated value identifying the category of devices that can possess
                                                 the fabric key. */
    kTag_RotationScheme    = 6,    /**< [ uint ] Enumerated value identifying the rotation scheme for the key. */
    kTag_RemainingLifeTime = 7,    /**< [ uint ] Remaining time (in seconds) until key expiration. Absent if lifetime
                                                 is indefinite or doesn't apply. */
    kTag_RemainingReservedTime = 8 /**< [ uint ] Remaining time (in seconds) until key is eligible for use. Absent if
                                                 key can be used right away. */
};

/**
 * Delegate class for implementing additional actions corresponding to Fabric Provisioning operations.
 */
class FabricProvisioningDelegate : public ChipServerDelegateBase
{
public:
    /**
     * Indicates that the device has created a new Fabric.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Other CHIP or platform-specific error codes indicating that an error
     *                         occurred preventing the device from creating a fabric.
     */
    virtual CHIP_ERROR HandleCreateFabric(void) = 0;

    /**
     * Indicates that the device has joined an existing Fabric.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Other CHIP or platform-specific error codes indicating that an error
     *                         occurred preventing the device from joining the fabric.
     */
    virtual CHIP_ERROR HandleJoinExistingFabric(void) = 0;

    /**
     * Indicates that the device has left a Fabric.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Other CHIP or platform-specific error codes indicating that an error
     *                         occurred preventing the device from leaving the fabric.
     */
    virtual CHIP_ERROR HandleLeaveFabric(void) = 0;

    /**
     * Indicates that the configuration of the current CHIP Fabric has been
     * requested.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Other CHIP or platform-specific error codes indicating that an error
     *                         occurred preventing the device from returning the fabric config.
     */
    virtual CHIP_ERROR HandleGetFabricConfig(void) = 0;

    /**
     * Enforce message-level access control for an incoming Fabric Provisioning request message.
     *
     * @param[in] ec            The ExchangeContext over which the message was received.
     * @param[in] msgProfileId  The profile id of the received message.
     * @param[in] msgType       The message type of the received message.
     * @param[in] msgInfo       A ChipMessageInfo structure containing information about the received message.
     * @param[inout] result     An enumerated value describing the result of access control policy evaluation for
     *                          the received message. Upon entry to the method, the value represents the tentative
     *                          result at the current point in the evaluation process.  Upon return, the result
     *                          is expected to represent the final assessment of access control policy for the
     *                          message.
     */
    virtual void EnforceAccessControl(ExchangeContext * ec, uint32_t msgProfileId, uint8_t msgType, const ChipMessageInfo * msgInfo,
                                      AccessControlResult & result);

    /**
     * Called to determine if the device is currently paired to an account.
     */
    // TODO: make this pure virtual when product code provides appropriate implementations.
    virtual bool IsPairedToAccount() const;
};

/**
 * Server class for implementing the Fabric Provisioning profile.
 */
// TODO: Additional documentation detail required (i.e. expected class usage, number in the system, instantiation requirements,
// lifetime).
class DLL_EXPORT FabricProvisioningServer : public ChipServerBase
{
public:
    FabricProvisioningServer(void);
    FabricProvisioningServer(const FabricProvisioningServer &) = delete;
    FabricProvisioningServer & operator=(const FabricProvisioningServer &) = delete;

    CHIP_ERROR Init(ChipExchangeManager * exchangeMgr);
    CHIP_ERROR Shutdown(void);

    void SetDelegate(FabricProvisioningDelegate * delegate);

    // Check if the session is marked as privileged to retrieve fabric config information.
    bool SessionHasFabricConfigAccessPrivilege(uint16_t keyId, uint64_t peerNodeId) const;

    virtual CHIP_ERROR SendSuccessResponse(void);
    virtual CHIP_ERROR SendStatusReport(uint32_t statusProfileId, uint16_t statusCode, CHIP_ERROR sysError = CHIP_NO_ERROR);

protected:
    FabricProvisioningDelegate * mDelegate;
    ExchangeContext * mCurClientOp;

private:
    static void HandleClientRequest(ExchangeContext * ec, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo,
                                    uint32_t profileId, uint8_t msgType, PacketBuffer * payload);

    // Utility functions for managing registration with/notification from ChipFabricState
    // about whether the current security session is privileged to
    // access fabric config information.
    void GrantFabricConfigAccessPrivilege(uint16_t keyId, uint64_t peerNodeId);
    void ClearFabricConfigAccessPrivilege(void);
    static void HandleSessionEnd(uint16_t keyId, uint64_t peerNodeId, void * context);
    CHIP_ERROR RegisterSessionEndCallbackWithFabricState(void);

    // Indicates the session that is privileged to
    // retrieve fabric config information.
    struct FabricConfigAccessSession
    {
        uint64_t PeerNodeId;
        uint16_t SessionKeyId;
    };
    FabricConfigAccessSession mFabricConfigAccessSession;

    chip::ChipFabricState::SessionEndCbCtxt mSessionEndCbCtxt;
};

} // namespace FabricProvisioning
} // namespace Protocols
} // namespace chip

#endif /* FABRICPROVISIONING_H_ */
