/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file implements objects which provide an abstraction layer between
 *      a platform's WiFiPAF implementation and the CHIP
 *      stack.
 *
 */
#include "WiFiPAFLayer.h"
#include "WiFiPAFConfig.h"
#include "WiFiPAFEndPoint.h"
#include "WiFiPAFError.h"
#include <lib/core/CHIPEncoding.h>

#undef CHIP_WIFIPAF_LAYER_DEBUG_LOGGING_ENABLED
// Magic values expected in first 2 bytes of valid PAF transport capabilities request or response:
// ref: 4.21.3, PAFTP Control Frames
#define CAPABILITIES_MSG_CHECK_BYTE_1 0b01100101
#define CAPABILITIES_MSG_CHECK_BYTE_2 0b01101100

namespace chip {
namespace WiFiPAF {

class WiFiPAFEndPointPool
{
public:
    int Size() const { return WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; }

    WiFiPAFEndPoint * Get(size_t i) const
    {
        static union
        {
            uint8_t Pool[sizeof(WiFiPAFEndPoint) * WIFIPAF_LAYER_NUM_PAF_ENDPOINTS];
            WiFiPAFEndPoint::AlignT ForceAlignment;
        } sEndPointPool;

        if (i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS)
        {
            return reinterpret_cast<WiFiPAFEndPoint *>(sEndPointPool.Pool + (sizeof(WiFiPAFEndPoint) * i));
        }

        return nullptr;
    }

    WiFiPAFEndPoint * Find(WIFIPAF_CONNECTION_OBJECT c) const
    {
        if (c == WIFIPAF_CONNECTION_UNINITIALIZED)
        {
            return nullptr;
        }

        for (size_t i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
        {
            WiFiPAFEndPoint * elem   = Get(i);
            WiFiPAFSession * pInInfo = reinterpret_cast<WiFiPAFSession *>(c);
            if ((elem->mWiFiPafLayer != nullptr) && (elem->mSessionInfo.id == pInInfo->id) &&
                (elem->mSessionInfo.peer_id == pInInfo->peer_id) &&
                !memcmp(elem->mSessionInfo.peer_addr, pInInfo->peer_addr, sizeof(uint8_t) * 6))
            {
                ChipLogProgress(WiFiPAF, "Find: Found WiFiPAFEndPoint[%lu]", i);
                return elem;
            }
#ifdef CHIP_WIFIPAF_LAYER_DEBUG_LOGGING_ENABLED
            {
                const WiFiPAFSession * pElmInfo = &elem->mSessionInfo;
                ChipLogError(WiFiPAF, "EndPoint[%lu]", i);
                ChipLogError(WiFiPAF, "Role: [%d, %d]", pElmInfo->role, pInInfo->role);
                ChipLogError(WiFiPAF, "id: [%u, %u]", pElmInfo->id, pInInfo->id);
                ChipLogError(WiFiPAF, "peer_id: [%d, %d]", pElmInfo->peer_id, pInInfo->peer_id);
                ChipLogError(WiFiPAF, "ElmMac: [%02x:%02x:%02x:%02x:%02x:%02x]", pElmInfo->peer_addr[0], pElmInfo->peer_addr[1],
                             pElmInfo->peer_addr[2], pElmInfo->peer_addr[3], pElmInfo->peer_addr[4], pElmInfo->peer_addr[5]);
                ChipLogError(WiFiPAF, "InMac: [%02x:%02x:%02x:%02x:%02x:%02x]", pInInfo->peer_addr[0], pInInfo->peer_addr[1],
                             pInInfo->peer_addr[2], pInInfo->peer_addr[3], pInInfo->peer_addr[4], pInInfo->peer_addr[5]);
                ChipLogError(WiFiPAF, "nodeId: [%lu, %lu]", pElmInfo->nodeId, pInInfo->nodeId);
                ChipLogError(WiFiPAF, "discriminator: [%d, %d]", pElmInfo->discriminator, pInInfo->discriminator);
            }
#endif
        }

        return nullptr;
    }

    WiFiPAFEndPoint * GetFree() const
    {
        for (size_t i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
        {
            WiFiPAFEndPoint * elem = Get(i);
            if (elem->mWiFiPafLayer == nullptr)
            {
                return elem;
            }
        }
        return nullptr;
    }
};

// EndPoint Pools
static WiFiPAFEndPointPool sWiFiPAFEndPointPool;

/*
 *   PAFTransportCapabilitiesRequestMessage implementation:
 *   ref: 4.21.3.1, PAFTP Handshake Request
 */
void PAFTransportCapabilitiesRequestMessage::SetSupportedProtocolVersion(uint8_t index, uint8_t version)
{
    uint8_t mask;

    // If even-index, store version in lower 4 bits; else, higher 4 bits.
    if (index % 2 == 0)
    {
        mask = 0x0F;
    }
    else
    {
        mask    = 0xF0;
        version = static_cast<uint8_t>(version << 4);
    }

    version &= mask;

    uint8_t & slot = mSupportedProtocolVersions[(index / 2)];
    slot           = static_cast<uint8_t>(slot & ~mask); // Clear version at index; leave other version in same byte alone
    slot |= version;
}

CHIP_ERROR PAFTransportCapabilitiesRequestMessage::Encode(const PacketBufferHandle & msgBuf) const
{
    uint8_t * p = msgBuf->Start();

    // Verify we can write the fixed-length request without running into the end of the buffer.
    VerifyOrReturnError(msgBuf->MaxDataLength() >= kCapabilitiesRequestLength, CHIP_ERROR_NO_MEMORY);

    chip::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_1);
    chip::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_2);

    for (uint8_t version : mSupportedProtocolVersions)
    {
        chip::Encoding::Write8(p, version);
    }

    chip::Encoding::LittleEndian::Write16(p, mMtu);
    chip::Encoding::Write8(p, mWindowSize);

    msgBuf->SetDataLength(kCapabilitiesRequestLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PAFTransportCapabilitiesRequestMessage::Decode(const PacketBufferHandle & msgBuf,
                                                          PAFTransportCapabilitiesRequestMessage & msg)
{
    const uint8_t * p = msgBuf->Start();

    // Verify we can read the fixed-length request without running into the end of the buffer.
    VerifyOrReturnError(msgBuf->DataLength() >= kCapabilitiesRequestLength, CHIP_ERROR_MESSAGE_INCOMPLETE);

    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_1 == chip::Encoding::Read8(p), WIFIPAF_ERROR_INVALID_MESSAGE);
    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_2 == chip::Encoding::Read8(p), WIFIPAF_ERROR_INVALID_MESSAGE);

    static_assert(kCapabilitiesRequestSupportedVersionsLength == sizeof(msg.mSupportedProtocolVersions),
                  "Expected capability sizes and storage must match");
    for (unsigned char & version : msg.mSupportedProtocolVersions)
    {
        version = chip::Encoding::Read8(p);
    }

    msg.mMtu        = chip::Encoding::LittleEndian::Read16(p);
    msg.mWindowSize = chip::Encoding::Read8(p);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PAFTransportCapabilitiesResponseMessage::Encode(const PacketBufferHandle & msgBuf) const
{
    uint8_t * p = msgBuf->Start();

    // Verify we can write the fixed-length request without running into the end of the buffer.
    VerifyOrReturnError(msgBuf->MaxDataLength() >= kCapabilitiesResponseLength, CHIP_ERROR_NO_MEMORY);

    chip::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_1);
    chip::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_2);

    chip::Encoding::Write8(p, mSelectedProtocolVersion);
    chip::Encoding::LittleEndian::Write16(p, mFragmentSize);
    chip::Encoding::Write8(p, mWindowSize);

    msgBuf->SetDataLength(kCapabilitiesResponseLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PAFTransportCapabilitiesResponseMessage::Decode(const PacketBufferHandle & msgBuf,
                                                           PAFTransportCapabilitiesResponseMessage & msg)
{
    const uint8_t * p = msgBuf->Start();

    // Verify we can read the fixed-length response without running into the end of the buffer.
    VerifyOrReturnError(msgBuf->DataLength() >= kCapabilitiesResponseLength, CHIP_ERROR_MESSAGE_INCOMPLETE);

    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_1 == chip::Encoding::Read8(p), WIFIPAF_ERROR_INVALID_MESSAGE);
    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_2 == chip::Encoding::Read8(p), WIFIPAF_ERROR_INVALID_MESSAGE);

    msg.mSelectedProtocolVersion = chip::Encoding::Read8(p);
    msg.mFragmentSize            = chip::Encoding::LittleEndian::Read16(p);
    msg.mWindowSize              = chip::Encoding::Read8(p);

    return CHIP_NO_ERROR;
}

/*
 * WiFiPAFLayer Implementation
 */

WiFiPAFLayer::WiFiPAFLayer()
{
    InitialPafInfo();
}

CHIP_ERROR WiFiPAFLayer::Init(chip::System::Layer * systemLayer)
{
    mSystemLayer = systemLayer;
    memset(&sWiFiPAFEndPointPool, 0, sizeof(sWiFiPAFEndPointPool));
    ChipLogProgress(WiFiPAF, "WiFiPAF: WiFiPAFLayer::Init()");
    return CHIP_NO_ERROR;
}

void WiFiPAFLayer::Shutdown(OnCancelDeviceHandle OnCancelDevice)
{
    ChipLogProgress(WiFiPAF, "WiFiPAF: Closing all WiFiPAF sessions to shutdown");
    uint8_t i;
    WiFiPAFSession * pPafSession;

    for (i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
    {
        pPafSession = &mPafInfoVect[i];
        if (pPafSession->id == UINT32_MAX)
        {
            // Unused session
            continue;
        }
        ChipLogProgress(WiFiPAF, "WiFiPAF: Canceling id: %u", pPafSession->id);
        OnCancelDevice(pPafSession->id, pPafSession->role);
        WiFiPAFEndPoint * endPoint = sWiFiPAFEndPointPool.Find(reinterpret_cast<WIFIPAF_CONNECTION_OBJECT>(pPafSession));
        if (endPoint != nullptr)
        {
            endPoint->DoClose(kWiFiPAFCloseFlag_AbortTransmission, WIFIPAF_ERROR_APP_CLOSED_CONNECTION);
        }
    }
}

bool WiFiPAFLayer::OnWiFiPAFMessageReceived(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg)
{
    WiFiPAFEndPoint * endPoint = sWiFiPAFEndPointPool.Find(reinterpret_cast<WIFIPAF_CONNECTION_OBJECT>(&RxInfo));
    VerifyOrReturnError(endPoint != nullptr, false, ChipLogDetail(WiFiPAF, "No endpoint for received indication"));
    RxInfo.role    = endPoint->mSessionInfo.role;
    CHIP_ERROR err = endPoint->Receive(std::move(msg));
    VerifyOrReturnError(err == CHIP_NO_ERROR, false,
                        ChipLogError(WiFiPAF, "Receive failed, err = %" CHIP_ERROR_FORMAT, err.Format()));

    return true;
}

CHIP_ERROR WiFiPAFLayer::OnWiFiPAFMsgRxComplete(WiFiPAFSession & RxInfo, System::PacketBufferHandle && msg)
{
    if (mWiFiPAFTransport != nullptr)
    {
        return mWiFiPAFTransport->WiFiPAFMessageReceived(RxInfo, std::move(msg));
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

void WiFiPAFLayer::SetWiFiPAFState(State state)
{
    mAppState = state;
}

CHIP_ERROR WiFiPAFLayer::SendMessage(WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msg)
{
    WiFiPAFEndPoint * endPoint = sWiFiPAFEndPointPool.Find(reinterpret_cast<WIFIPAF_CONNECTION_OBJECT>(&TxInfo));
    VerifyOrReturnError(endPoint != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogDetail(WiFiPAF, "No endpoint to send packets"));
    CHIP_ERROR err = endPoint->Send(std::move(msg));
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(WiFiPAF, "Send pakets failed, err = %" CHIP_ERROR_FORMAT, err.Format()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFLayer::HandleWriteConfirmed(WiFiPAF::WiFiPAFSession & TxInfo, bool result)
{
    WiFiPAFEndPoint * endPoint = sWiFiPAFEndPointPool.Find(reinterpret_cast<WIFIPAF_CONNECTION_OBJECT>(&TxInfo));
    VerifyOrReturnError(endPoint != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogDetail(WiFiPAF, "No endpoint to send packets"));
    CHIP_ERROR err = endPoint->HandleSendConfirmationReceived(result);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(WiFiPAF, "Write_Confirm, Send pakets failed, err = %" CHIP_ERROR_FORMAT, err.Format()));

    return CHIP_NO_ERROR;
}

static WiFiPAFLayer sInstance;
WiFiPAFLayer & WiFiPAFLayer::GetWiFiPAFLayer()
{
    return sInstance;
}

CHIP_ERROR WiFiPAFLayer::NewEndPoint(WiFiPAFEndPoint ** retEndPoint, WiFiPAFSession & SessionInfo, WiFiPafRole role)
{
    *retEndPoint = nullptr;

    *retEndPoint = sWiFiPAFEndPointPool.GetFree();
    if (*retEndPoint == nullptr)
    {
        ChipLogError(WiFiPAF, "endpoint pool FULL");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }
    return (*retEndPoint)->Init(this, SessionInfo);
}

CHIP_ERROR WiFiPAFLayer::HandleTransportConnectionInitiated(WiFiPAF::WiFiPAFSession & SessionInfo,
                                                            OnSubscribeCompleteFunct OnSubscribeDoneFunc, void * appState,
                                                            OnSubscribeErrorFunct OnSubscribeErrFunc)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    WiFiPAFEndPoint * newEndPoint = nullptr;

    ChipLogProgress(WiFiPAF, "Creating WiFiPAFEndPoint");
    ReturnErrorOnFailure(NewEndPoint(&newEndPoint, SessionInfo, SessionInfo.role));
    newEndPoint->mOnPafSubscribeComplete = OnSubscribeDoneFunc;
    newEndPoint->mOnPafSubscribeError    = OnSubscribeErrFunc;
    newEndPoint->mAppState               = appState;
    if (SessionInfo.role == kWiFiPafRole_Subscriber)
    {
        err = newEndPoint->StartConnect();
    }

    return err;
}

void WiFiPAFLayer::OnEndPointConnectComplete(WiFiPAFEndPoint * endPoint, CHIP_ERROR err)
{
    VerifyOrDie(endPoint != nullptr);
    if (endPoint->mOnPafSubscribeComplete != nullptr)
    {
        endPoint->mOnPafSubscribeComplete(endPoint->mAppState);
        endPoint->mOnPafSubscribeComplete = nullptr;
    }
}

WiFiPAFTransportProtocolVersion
WiFiPAFLayer::GetHighestSupportedProtocolVersion(const PAFTransportCapabilitiesRequestMessage & reqMsg)
{
    WiFiPAFTransportProtocolVersion retVersion = kWiFiPAFTransportProtocolVersion_None;

    uint8_t shift_width = 4;

    for (int i = 0; i < NUM_PAFTP_SUPPORTED_PROTOCOL_VERSIONS; i++)
    {
        shift_width ^= 4;

        uint8_t version = reqMsg.mSupportedProtocolVersions[(i / 2)];
        version         = static_cast<uint8_t>((version >> shift_width) & 0x0F); // Grab just the nibble we want.

        if ((version >= CHIP_PAF_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION) &&
            (version <= CHIP_PAF_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION) && (version > retVersion))
        {
            retVersion = static_cast<WiFiPAFTransportProtocolVersion>(version);
        }
        else if (version == kWiFiPAFTransportProtocolVersion_None) // Signifies end of supported versions list
        {
            break;
        }
    }

    return retVersion;
}

inline constexpr uint8_t kInvalidActiveWiFiPafSessionId = UINT8_MAX;
void WiFiPAFLayer::InitialPafInfo()
{
    for (uint8_t i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
    {
        CleanPafInfo(mPafInfoVect[i]);
    }
}

void WiFiPAFLayer::CleanPafInfo(WiFiPAFSession & SessionInfo)
{
    memset(&SessionInfo, 0, sizeof(WiFiPAFSession));
    SessionInfo.id            = kUndefinedWiFiPafSessionId;
    SessionInfo.peer_id       = kUndefinedWiFiPafSessionId;
    SessionInfo.nodeId        = kUndefinedNodeId;
    SessionInfo.discriminator = UINT16_MAX;
    return;
}

CHIP_ERROR WiFiPAFLayer::AddPafSession(PafInfoAccess accType, WiFiPAFSession & SessionInfo)
{
    uint8_t i;
    uint8_t eSlotId              = kInvalidActiveWiFiPafSessionId;
    WiFiPAFSession * pPafSession = nullptr;

    // Check if the session has existed
    for (i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
    {
        pPafSession = &mPafInfoVect[i];
        switch (accType)
        {
        case PafInfoAccess::kAccNodeInfo:
            if (pPafSession->nodeId == SessionInfo.nodeId)
            {
                VerifyOrDie(pPafSession->discriminator == SessionInfo.discriminator);
                // Already exist
                return CHIP_NO_ERROR;
            }
            break;
        case PafInfoAccess::kAccSessionId:
            if (pPafSession->id == SessionInfo.id)
            {
                // Already exist
                return CHIP_NO_ERROR;
            }
            break;
        default:
            return CHIP_ERROR_NOT_IMPLEMENTED;
        };
        if ((pPafSession->id == kUndefinedWiFiPafSessionId) && (pPafSession->nodeId == kUndefinedNodeId) &&
            (pPafSession->discriminator == UINT16_MAX))
        {
            eSlotId = i;
        }
    }
    // Add the session if available
    if (eSlotId != kInvalidActiveWiFiPafSessionId)
    {
        pPafSession       = &mPafInfoVect[eSlotId];
        pPafSession->role = SessionInfo.role;
        switch (accType)
        {
        case PafInfoAccess::kAccNodeInfo:
            pPafSession->nodeId        = SessionInfo.nodeId;
            pPafSession->discriminator = SessionInfo.discriminator;
            ChipLogProgress(WiFiPAF, "WiFiPAF: Add session with nodeId: %lu, disc: %x, sessions", SessionInfo.nodeId,
                            SessionInfo.discriminator);
            return CHIP_NO_ERROR;
        case PafInfoAccess::kAccSessionId:
            pPafSession->id = SessionInfo.id;
            ChipLogProgress(WiFiPAF, "WiFiPAF: Add session with id: %u", SessionInfo.id);
            return CHIP_NO_ERROR;
        default:
            return CHIP_ERROR_NOT_IMPLEMENTED;
        };
    }
    ChipLogError(WiFiPAF, "WiFiPAF: No available space for the new sessions");
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR WiFiPAFLayer::RmPafSession(PafInfoAccess accType, WiFiPAFSession & SessionInfo)
{
    uint8_t i;
    WiFiPAFSession * pPafSession;

    for (i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
    {
        pPafSession = &mPafInfoVect[i];
        switch (accType)
        {
        case PafInfoAccess::kAccSessionId:
            if (pPafSession->id == SessionInfo.id)
            {
                ChipLogProgress(WiFiPAF, "Removing session with id: %u", pPafSession->id);
                // Clear the slot
                CleanPafInfo(*pPafSession);
                return CHIP_NO_ERROR;
            }
            break;
        default:
            return CHIP_ERROR_NOT_IMPLEMENTED;
        };
    }
    ChipLogError(WiFiPAF, "No PAF session found");
    return CHIP_ERROR_NOT_FOUND;
}

WiFiPAFSession * WiFiPAFLayer::GetPAFInfo(PafInfoAccess accType, WiFiPAFSession & SessionInfo)
{
    uint8_t i;
    WiFiPAFSession * pPafSession = nullptr;

    for (i = 0; i < WIFIPAF_LAYER_NUM_PAF_ENDPOINTS; i++)
    {
        pPafSession = &mPafInfoVect[i];
        if (pPafSession->role == kWiFiPafRole_Publisher)
        {
            if (pPafSession->id != kUndefinedWiFiPafSessionId)
                return pPafSession;
            else
                continue;
        }
        switch (accType)
        {
        case PafInfoAccess::kAccSessionId:
            if (pPafSession->id == SessionInfo.id)
            {
                return pPafSession;
            }
            break;
        case PafInfoAccess::kAccNodeId:
            if (pPafSession->nodeId == SessionInfo.nodeId)
            {
                return pPafSession;
            }
            break;
        case PafInfoAccess::kAccDisc:
            if (pPafSession->discriminator == SessionInfo.discriminator)
            {
                return pPafSession;
            }
            break;
        default:
            return nullptr;
        };
    }

    return nullptr;
}
} /* namespace WiFiPAF */
} /* namespace chip */
