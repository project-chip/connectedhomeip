/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines objects for a User-Directed Commissioning unsolicited
 *      initiator (client) and recipient (server).
 *
 */

#pragma once

#include "UDCClients.h"
#include <lib/core/CHIPCore.h>
#include <lib/dnssd/Resolver.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <transport/TransportMgr.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

inline constexpr char kProtocolName[] = "UserDirectedCommissioning";

// Cache contains 16 clients. This may need to be tweaked.
inline constexpr uint8_t kMaxUDCClients = 16;

/**
 * User Directed Commissioning Protocol Message Types
 */
enum class MsgType : uint8_t
{
    IdentificationDeclaration = 0x00,
};

/**
 * Represents the Identification Delaration message
 * sent by a UDC client to a UDC server.
 *
 * ### IdentificationDeclaration format
 *
 * <pre>
 *  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━┓
 *  ┃ instance name '\n'        ┃ ignore   ┃ additional data TLV ┃
 *  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━┛
 *  │← · · kInstanceNameMaxLength + 1 · · →│← TLV DataLength()  →│
 *
 * Commissioning kInstanceNameMaxLength is 16
 * </pre>
 *
 */
class DLL_EXPORT IdentificationDeclaration
{
public:
    constexpr static size_t kUdcTLVDataMaxBytes = 500;

    const char * GetInstanceName() const { return mInstanceName; }
    void SetInstanceName(const char * instanceName) { Platform::CopyString(mInstanceName, instanceName); }

    bool HasDiscoveryInfo() { return mVendorId != 0 && mProductId != 0 && mCdPort != 0 && strlen(mDeviceName) > 0; }

    const char * GetDeviceName() const { return mDeviceName; }
    void SetDeviceName(const char * deviceName) { Platform::CopyString(mDeviceName, deviceName); }

    uint16_t GetCdPort() const { return mCdPort; }
    void SetCdPort(uint16_t port) { mCdPort = port; }

    uint16_t GetVendorId() const { return mVendorId; }
    void SetVendorId(uint16_t vendorId) { mVendorId = vendorId; }

    uint16_t GetProductId() const { return mProductId; }
    void SetProductId(uint16_t productId) { mProductId = productId; }

    const uint8_t * GetRotatingId() const { return mRotatingId; }
    size_t GetRotatingIdLength() const { return mRotatingIdLen; }
    void SetRotatingId(const uint8_t * rotatingId, size_t rotatingIdLen)
    {
        size_t maxSize = ArraySize(mRotatingId);
        mRotatingIdLen = (maxSize < rotatingIdLen) ? maxSize : rotatingIdLen;
        memcpy(mRotatingId, rotatingId, mRotatingIdLen);
    }

    bool GetTargetAppInfo(uint8_t index, TargetAppInfo & info) const
    {
        if (index < mNumTargetAppInfos)
        {
            info.vendorId  = mTargetAppInfos[index].vendorId;
            info.productId = mTargetAppInfos[index].productId;
            return true;
        }
        return false;
    }
    uint8_t GetNumTargetAppInfos() const { return mNumTargetAppInfos; }

    bool AddTargetAppInfo(TargetAppInfo vid)
    {
        if (mNumTargetAppInfos >= sizeof(mTargetAppInfos))
        {
            // already at max
            return false;
        }
        mTargetAppInfos[mNumTargetAppInfos].vendorId  = vid.vendorId;
        mTargetAppInfos[mNumTargetAppInfos].productId = vid.productId;
        mNumTargetAppInfos++;
        return true;
    }

    const char * GetPairingInst() const { return mPairingInst; }
    void SetPairingInst(const char * pairingInst) { Platform::CopyString(mPairingInst, pairingInst); }

    uint16_t GetPairingHint() const { return mPairingHint; }
    void SetPairingHint(uint16_t pairingHint) { mPairingHint = pairingHint; }

    void SetNoPasscode(bool newValue) { mNoPasscode = newValue; };
    bool GetNoPasscode() const { return mNoPasscode; };

    void SetCdUponPasscodeDialog(bool newValue) { mCdUponPasscodeDialog = newValue; };
    bool GetCdUponPasscodeDialog() const { return mCdUponPasscodeDialog; };

    void SetCommissionerPasscode(bool newValue) { mCommissionerPasscode = newValue; };
    bool GetCommissionerPasscode() const { return mCommissionerPasscode; };

    void SetCommissionerPasscodeReady(bool newValue) { mCommissionerPasscodeReady = newValue; };
    bool GetCommissionerPasscodeReady() const { return mCommissionerPasscodeReady; };

    void SetCancelPasscode(bool newValue) { mCancelPasscode = newValue; };
    bool GetCancelPasscode() const { return mCancelPasscode; };

    /**
     *  Writes the IdentificationDeclaration message to the given buffer.
     *
     * @return Total number of bytes written or 0 if an error occurred.
     */
    uint32_t WritePayload(uint8_t * payloadBuffer, size_t payloadBufferSize);

    /**
     *  Reads the IdentificationDeclaration message from the given buffer.
     */
    CHIP_ERROR ReadPayload(uint8_t * payloadBuffer, size_t payloadBufferSize);

    /**
     *  Assigns fields from this Identification Declaration to the given UDC client state.
     */
    void UpdateClientState(UDCClientState * client)
    {
        client->SetDeviceName(GetDeviceName());
        client->SetVendorId(GetVendorId());
        client->SetProductId(GetProductId());
        client->SetRotatingId(GetRotatingId(), GetRotatingIdLength());
        client->SetPairingInst(GetPairingInst());
        client->SetPairingHint(GetPairingHint());
        for (uint8_t i = 0; i < GetNumTargetAppInfos(); i++)
        {
            TargetAppInfo info;
            if (GetTargetAppInfo(i, info))
            {
                client->AddTargetAppInfo(info);
            }
        }

        client->SetCdPort(GetCdPort());
        client->SetNoPasscode(GetNoPasscode());
        client->SetCdUponPasscodeDialog(GetCdUponPasscodeDialog());
        client->SetCommissionerPasscode(GetCommissionerPasscode());
        client->SetCommissionerPasscodeReady(GetCommissionerPasscodeReady());
        client->SetCancelPasscode(GetCancelPasscode());
    }

    void DebugLog()
    {
        ChipLogDetail(AppServer, "---- Identification Declaration Start ----");

        ChipLogDetail(AppServer, "\tinstance: %s", mInstanceName);
        if (strlen(mDeviceName) != 0)
        {
            ChipLogDetail(AppServer, "\tdevice Name: %s", mDeviceName);
        }
        if (mVendorId != 0)
        {
            ChipLogDetail(AppServer, "\tvendor id: %d", mVendorId);
        }
        if (mProductId != 0)
        {
            ChipLogDetail(AppServer, "\tproduct id: %d", mProductId);
        }
        if (mCdPort != 0)
        {
            ChipLogDetail(AppServer, "\tcd port: %d", mCdPort);
        }
        if (mRotatingIdLen > 0)
        {
            char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(mRotatingId, mRotatingIdLen, rotatingIdString, sizeof(rotatingIdString));
            ChipLogDetail(AppServer, "\trotating id: %s", rotatingIdString);
        }
        for (uint8_t i = 0; i < mNumTargetAppInfos; i++)
        {
            ChipLogDetail(AppServer, "\tapp vendor id / product id [%d]: %u/%u", i, mTargetAppInfos[i].vendorId,
                          mTargetAppInfos[i].productId);
        }
        if (strlen(mPairingInst) != 0)
        {
            ChipLogDetail(AppServer, "\tpairing instruction: %s", mPairingInst);
        }
        if (mPairingHint != 0)
        {
            ChipLogDetail(AppServer, "\tpairing hint: %d", mPairingHint);
        }
        if (mNoPasscode)
        {
            ChipLogDetail(AppServer, "\tno passcode: true");
        }
        if (mCdUponPasscodeDialog)
        {
            ChipLogDetail(AppServer, "\tcd upon passcode dialog: true");
        }
        if (mCommissionerPasscode)
        {
            ChipLogDetail(AppServer, "\tcommissioner passcode: true");
        }
        if (mCommissionerPasscodeReady)
        {
            ChipLogDetail(AppServer, "\tcommissioner passcode ready: true");
        }
        if (mCancelPasscode)
        {
            ChipLogDetail(AppServer, "\tcancel passcode: true");
        }
        ChipLogDetail(AppServer, "---- Identification Declaration End ----");
    }

private:
    // TODO: update spec per the latest tags
    enum IdentificationDeclarationTLVTag
    {
        kVendorIdTag = 1,
        kProductIdTag,
        kDeviceNameTag,
        kDeviceTypeTag,
        kPairingInstTag,
        kPairingHintTag,
        kRotatingIdTag,
        kCdPortTag,
        kTargetAppListTag,
        kTargetAppTag,
        kAppVendorIdTag,
        kAppProductIdTag,
        kNoPasscodeTag,
        kCdUponPasscodeDialogTag,
        kCommissionerPasscodeTag,
        kCommissionerPasscodeReadyTag,
        kCancelPasscodeTag,

        kMaxNum = UINT8_MAX
    };

    char mInstanceName[Dnssd::Commission::kInstanceNameMaxLength + 1] = {};
    char mDeviceName[Dnssd::kMaxDeviceNameLen + 1]                    = {};
    uint16_t mCdPort                                                  = 0;

    uint16_t mVendorId  = 0;
    uint16_t mProductId = 0;
    uint8_t mRotatingId[chip::Dnssd::kMaxRotatingIdLen];
    size_t mRotatingIdLen = 0;

    constexpr static size_t kMaxTargetAppInfos = 10;
    uint8_t mNumTargetAppInfos                 = 0; // number of vendor Ids
    TargetAppInfo mTargetAppInfos[kMaxTargetAppInfos];

    char mPairingInst[chip::Dnssd::kMaxPairingInstructionLen + 1] = {};
    uint16_t mPairingHint                                         = 0;

    bool mNoPasscode                = false;
    bool mCdUponPasscodeDialog      = false;
    bool mCommissionerPasscode      = false;
    bool mCommissionerPasscodeReady = false;
    bool mCancelPasscode            = false;
};

/**
 * Represents the Commissioner Delaration message
 * sent by a UDC server to a UDC client.
 */
class DLL_EXPORT CommissionerDeclaration
{
public:
    enum class CdError : uint16_t
    {
        kNoError                                = 0,
        kCommissionableDiscoveryFailed          = 1,
        kPaseConnectionFailed                   = 2,
        kPaseAuthFailed                         = 3,
        kDacValidationFailed                    = 4,
        kAlreadyOnFabric                        = 5,
        kOperationalDiscoveryFailed             = 6,
        kCaseConnectionFailed                   = 7,
        kCaseAuthFailed                         = 8,
        kConfigurationFailed                    = 9,
        kBindingConfigurationFailed             = 10,
        kCommissionerPasscodeNotSupported       = 11,
        kInvalidIdentificationDeclarationParams = 12,
        kAppInstallConsentPending               = 13,
        kAppInstalling                          = 14,
        kAppInstallFailed                       = 15,
        kAppInstalledRetryNeeded                = 16,
        kCommissionerPasscodeDisabled           = 17,
        kUnexpectedCommissionerPasscodeReady    = 18
    };

    constexpr static size_t kUdcTLVDataMaxBytes = 500;

    void SetErrorCode(CdError newValue) { mErrorCode = newValue; };
    CdError GetErrorCode() const { return mErrorCode; };

    void SetNeedsPasscode(bool newValue) { mNeedsPasscode = newValue; };
    bool GetNeedsPasscode() const { return mNeedsPasscode; };

    void SetNoAppsFound(bool newValue) { mNoAppsFound = newValue; };
    bool GetNoAppsFound() const { return mNoAppsFound; };

    void SetPasscodeDialogDisplayed(bool newValue) { mPasscodeDialogDisplayed = newValue; };
    bool GetPasscodeDialogDisplayed() const { return mPasscodeDialogDisplayed; };

    void SetCommissionerPasscode(bool newValue) { mCommissionerPasscode = newValue; };
    bool GetCommissionerPasscode() const { return mCommissionerPasscode; };

    void SetQRCodeDisplayed(bool newValue) { mQRCodeDisplayed = newValue; };
    bool GetQRCodeDisplayed() const { return mQRCodeDisplayed; };

    void SetCancelPasscode(bool newValue) { mCancelPasscode = newValue; };
    bool GetCancelPasscode() const { return mCancelPasscode; };

    /**
     *  Writes the CommissionerDeclaration message to the given buffer.
     *
     * @return Total number of bytes written or 0 if an error occurred.
     */
    uint32_t WritePayload(uint8_t * payloadBuffer, size_t payloadBufferSize);

    /**
     *  Reads the CommissionerDeclaration message from the given buffer.
     */
    CHIP_ERROR ReadPayload(uint8_t * payloadBuffer, size_t payloadBufferSize);

    void DebugLog()
    {
        ChipLogDetail(AppServer, "---- Commissioner Declaration Start ----");

        if (mErrorCode != CdError::kNoError)
        {
            ChipLogDetail(AppServer, "\terror code: %d", static_cast<uint16_t>(mErrorCode));
        }

        if (mNeedsPasscode)
        {
            ChipLogDetail(AppServer, "\tneeds passcode: true");
        }
        if (mNoAppsFound)
        {
            ChipLogDetail(AppServer, "\tno apps found: true");
        }
        if (mPasscodeDialogDisplayed)
        {
            ChipLogDetail(AppServer, "\tpasscode dialog displayed: true");
        }
        if (mCommissionerPasscode)
        {
            ChipLogDetail(AppServer, "\tcommissioner passcode: true");
        }
        if (mQRCodeDisplayed)
        {
            ChipLogDetail(AppServer, "\tQR code displayed: true");
        }
        if (mCancelPasscode)
        {
            ChipLogDetail(AppServer, "\tPasscode cancelled: true");
        }
        ChipLogDetail(AppServer, "---- Commissioner Declaration End ----");
    }

private:
    // TODO: update spec per the latest tags
    enum CommissionerDeclarationTLVTag
    {
        kErrorCodeTag = 1,
        kNeedsPasscodeTag,
        kNoAppsFoundTag,
        kPasscodeDialogDisplayedTag,
        kCommissionerPasscodeTag,
        kQRCodeDisplayedTag,
        kCancelPasscodeTag,

        kMaxNum = UINT8_MAX
    };

    CdError mErrorCode            = CdError::kNoError;
    bool mNeedsPasscode           = false;
    bool mNoAppsFound             = false;
    bool mPasscodeDialogDisplayed = false;
    bool mCommissionerPasscode    = false;
    bool mQRCodeDisplayed         = false;
    bool mCancelPasscode          = false;
};

class DLL_EXPORT InstanceNameResolver
{
public:
    /**
     * @brief
     *   Called when a UDC message is received specifying the given instanceName
     * This method indicates that UDC Server needs the Commissionable Node corresponding to
     * the given instance name to be found. UDC Server will wait for OnCommissionableNodeFound.
     *
     * @param instanceName DNS-SD instance name for the client requesting commissioning
     *
     */
    virtual void FindCommissionableNode(char * instanceName) = 0;

    virtual ~InstanceNameResolver() = default;
};

class DLL_EXPORT UserConfirmationProvider
{
public:
    /**
     * @brief
     *   Called when an Identification Declaration UDC message has been received
     * and corresponding nodeData has been found.
     * It is expected that the implementer will prompt the user to confirm their intention to
     * commission the given node, and obtain the setup code to allow commissioning to proceed,
     * and then invoke commissioning on the given Node (using CHIP Device Controller, for example)
     *
     *  @param[in]    state           The state for the UDC Client.
     *
     */
    virtual void OnUserDirectedCommissioningRequest(UDCClientState state) = 0;

    /**
     * @brief
     *   Called when an Identification Declaration UDC message has been received
     * with the cancel flag set.
     * It is expected that the implementer will tear down any dialog prompts for the
     * commissionee instance (identified in the UDC client state argument).
     *
     *  @param[in]    state           The state for the UDC Client.
     *
     */
    virtual void OnCancel(UDCClientState state) = 0;

    /**
     * @brief
     *   Called when an Identification Declaration UDC message has been received
     * with the commissioner passcode ready flag set.
     * It is expected that the implementer will invoke commissioning on the
     * commissionee instance (identified in the UDC client state argument).
     *
     *  @param[in]    state           The state for the UDC Client.
     *
     */
    virtual void OnCommissionerPasscodeReady(UDCClientState state) = 0;

    virtual ~UserConfirmationProvider() = default;
};

class DLL_EXPORT CommissionerDeclarationHandler
{
public:
    /**
     * @brief
     *   Called when a Commissioner Declaration UDC message has been received.
     * It is expected that the implementer will de-dup messages received from the
     * same source within a short (1 second) time window.
     *
     *  @param[in]    source       The source of the Commissioner Declaration Message.
     *  @param[in]    cd           The Commissioner Declaration Message.
     *
     */
    virtual void OnCommissionerDeclarationMessage(const chip::Transport::PeerAddress & source, CommissionerDeclaration cd) = 0;

    virtual ~CommissionerDeclarationHandler() = default;
};

/**
 * TODO:
 * - add processing of Commissioner Declaration flags
 */
class DLL_EXPORT UserDirectedCommissioningClient : public TransportMgrDelegate
{
public:
    /**
     * Send a User Directed Commissioning message to a CHIP node.
     *
     * @param transportMgr  A transport to use for sending the message.
     * @param idMessage     The Identification Declaration message.
     * @param peerAddress   Address of destination.
     *
     * @return CHIP_ERROR_NO_MEMORY if allocation fails.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */

    CHIP_ERROR SendUDCMessage(TransportMgrBase * transportMgr, IdentificationDeclaration idMessage,
                              chip::Transport::PeerAddress peerAddress);

    /**
     * Encode a User Directed Commissioning message.
     *
     * @param payload       A PacketBufferHandle with the payload.
     *
     * @return CHIP_ERROR_NO_MEMORY if allocation fails.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */

    CHIP_ERROR EncodeUDCMessage(const System::PacketBufferHandle & payload);

    /**
     * Set the listener to be called when a Commissioner Declaration UDC request is received.
     *
     *  @param[in]    commissionerDeclarationHandler    The callback function to handle the message.
     *
     */
    void SetCommissionerDeclarationHandler(CommissionerDeclarationHandler * commissionerDeclarationHandler)
    {
        ChipLogProgress(AppServer, "UserDirectedCommissioningClient::SetCommissionerDeclarationHandler()");
        mCommissionerDeclarationHandler = commissionerDeclarationHandler;
    }

private:
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                           Transport::MessageTransportContext * ctxt = nullptr) override;

    CommissionerDeclarationHandler * mCommissionerDeclarationHandler = nullptr;
};

/**
 * TODO:
 * - add processing of Identification Declaration flags
 */
class DLL_EXPORT UserDirectedCommissioningServer : public TransportMgrDelegate
{
public:
    /**
     * Set the listener to be called when a UDC request is received
     * and the Instance Name provided needs to be resolved.
     *
     * The resolver should call OnCommissionableNodeFound when the instance is found
     *
     *  @param[in]    instanceNameResolver    The callback function to receive UDC request instance name.
     *
     */
    void SetInstanceNameResolver(InstanceNameResolver * instanceNameResolver) { mInstanceNameResolver = instanceNameResolver; }

    /**
     * Set the listener to be called when a UDC request is received
     * and the Instance Name has been resolved.
     *
     * The provider should prompt the user to allow commissioning of the node and provide the setup code.
     *
     *  @param[in]    userConfirmationProvider    The callback function to obtain user confirmation.
     *
     */
    void SetUserConfirmationProvider(UserConfirmationProvider * userConfirmationProvider)
    {
        mUserConfirmationProvider = userConfirmationProvider;
    }

    /**
     * Update the processing state for a UDC Client based upon instance name.
     *
     * This can be used by the UX to set the state to one of the following values:
     * - kUserDeclined
     * - kObtainingOnboardingPayload
     * - kCommissioningNode
     * - kCommissioningFailed
     *
     *  @param[in]    instanceName    The instance name for the UDC Client.
     *  @param[in]    state           The state for the UDC Client.
     *
     */
    void SetUDCClientProcessingState(char * instanceName, UDCClientProcessingState state);

    /**
     * Reset the processing states for all UDC Clients
     *
     */
    void ResetUDCClientProcessingStates() { mUdcClients.ResetUDCClientStates(); }

    /**
     * Called when a CHIP Node in commissioning mode is found.
     *
     * Lookup instanceName from nodeData in the active UDC Client states
     * and if current state is kDiscoveringNode then change to kPromptingUser and
     * call UX Prompt callback
     *
     *  @param[in]    nodeData        DNS-SD response data.
     *
     */
    void OnCommissionableNodeFound(const Dnssd::DiscoveredNodeData & nodeData);

    /**
     * Get the cache of UDC Clients
     *
     */
    UDCClients<kMaxUDCClients> & GetUDCClients() { return mUdcClients; }

    /**
     * Print the cache of UDC Clients
     *
     */
    void PrintUDCClients();

    /**
     * Send a Commissioner Declaration message to the given peer address
     *
     * Only one message will be sent.
     * Clients should follow spec and send up to 5 times with 100ms sleep between each call.
     */
    CHIP_ERROR SendCDCMessage(CommissionerDeclaration cdMessage, chip::Transport::PeerAddress peerAddress);

    /**
     * Encode a User Directed Commissioning message.
     *
     * @param payload       A PacketBufferHandle with the payload.
     *
     * @return CHIP_ERROR_NO_MEMORY if allocation fails.
     *         Other CHIP_ERROR codes as returned by the lower layers.
     *
     */
    CHIP_ERROR EncodeUDCMessage(const System::PacketBufferHandle & payload);

    /**
     * Assign the transport manager to use for Commissioner Declaration messages
     */
    void SetTransportManager(TransportMgrBase * transportMgr) { mTransportMgr = transportMgr; }

private:
    InstanceNameResolver * mInstanceNameResolver         = nullptr;
    UserConfirmationProvider * mUserConfirmationProvider = nullptr;

    void HandleNewUDC(const Transport::PeerAddress & source, IdentificationDeclaration & id);
    void HandleUDCCancel(IdentificationDeclaration & id);
    void HandleUDCCommissionerPasscodeReady(IdentificationDeclaration & id);
    void OnMessageReceived(const Transport::PeerAddress & source, System::PacketBufferHandle && msgBuf,
                           Transport::MessageTransportContext * ctxt = nullptr) override;

    UDCClients<kMaxUDCClients> mUdcClients; // < Active UDC clients

    TransportMgrBase * mTransportMgr = nullptr;
};

} // namespace UserDirectedCommissioning

template <>
struct MessageTypeTraits<UserDirectedCommissioning::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return UserDirectedCommissioning::Id; }

    static auto GetTypeToNameTable()
    {
        static const std::array<MessageTypeNameLookup, 1> typeToNameTable = {
            {
                { UserDirectedCommissioning::MsgType::IdentificationDeclaration, "IdentificationDeclaration" },
            },
        };

        return &typeToNameTable;
    }
};

} // namespace Protocols
} // namespace chip
