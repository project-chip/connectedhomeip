/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file implements objects which provide an abstraction layer between
 *      a platform's Bluetooth Low Energy (BLE) implementation and the Weave
 *      stack.
 *
 *      The BleLayer obect accepts BLE data and control input from the
 *      application via a functional interface. It performs the fragmentation
 *      and reassembly required to transmit Weave message via a BLE GATT
 *      characteristic interface, and drives incoming messages up the Weave
 *      stack.
 *
 *      During initialization, the BleLayer object requires a pointer to the
 *      platform's implementation of the BlePlatformDelegate and
 *      BleApplicationDelegate objects.
 *
 *      The BlePlatformDelegate provides the Weave stack with an interface
 *      by which to form and cancel GATT subscriptions, read and write
 *      GATT characteristic values, send GATT characteristic notifications,
 *      respond to GATT read requests, and close BLE connections.
 *
 *      The BleApplicationDelegate provides a mechanism for Weave to inform
 *      the application when it has finished using a given BLE connection,
 *      i.e when the WeaveConnection object wrapping this connection has
 *      closed. This allows the application to either close the BLE connection
 *      or continue to keep it open for non-Weave purposes.
 *
 *      To enable Weave over BLE for a new platform, the application developer
 *      must provide an implementation for both delegates, provides points to
 *      instances of these delegates on startup, and ensure that the
 *      application calls the necessary BleLayer functions when appropriate to
 *      drive BLE data and control input up the stack.
 */

#include <BleLayer/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include <string.h>

#include <BleLayer/BlePlatformDelegate.h>
#include <BleLayer/BleApplicationDelegate.h>
#include <BleLayer/BleLayer.h>
#include <BleLayer/BleUUID.h>
#include <BleLayer/BLEEndPoint.h>

#include <Weave/Core/WeaveEncoding.h>
#include <Weave/Support/logging/WeaveLogging.h>
#include <Weave/Support/CodeUtils.h>

// clang-format off

#define CAPABILITIES_REQUEST_MAGICNUM_LEN 2
#define CAPABILITIES_REQUEST_L2CAP_MTU_LEN 2
#define CAPABILITIES_REQUEST_SUPPORTED_VERSIONS_LEN 4
#define CAPABILITIES_REQUEST_WINDOW_SIZE_LEN 1
#define CAPABILITIES_REQUEST_LEN (CAPABILITIES_REQUEST_MAGICNUM_LEN + \
                                  CAPABILITIES_REQUEST_L2CAP_MTU_LEN + \
                                  CAPABILITIES_REQUEST_SUPPORTED_VERSIONS_LEN + \
                                  CAPABILITIES_REQUEST_WINDOW_SIZE_LEN)

#define CAPABILITIES_RESPONSE_MAGICNUM_LEN 2
#define CAPABILITIES_RESPONSE_L2CAP_MTU_LEN 2
#define CAPABILITIES_RESPONSE_SELECTED_PROTOCOL_VERSION_LEN 1
#define CAPABILITIES_RESPONSE_WINDOW_SIZE_LEN 1
#define CAPABILITIES_RESPONSE_LEN (CAPABILITIES_RESPONSE_MAGICNUM_LEN + \
                                   CAPABILITIES_RESPONSE_L2CAP_MTU_LEN + \
                                   CAPABILITIES_RESPONSE_SELECTED_PROTOCOL_VERSION_LEN + \
                                   CAPABILITIES_RESPONSE_WINDOW_SIZE_LEN)

// Magic values expected in first 2 bytes of valid BLE transport capabilities request or response:
#define CAPABILITIES_MSG_CHECK_BYTE_1 'n'
#define CAPABILITIES_MSG_CHECK_BYTE_2 'l'

// clang-format on

namespace nl {
namespace Ble {

class BleEndPointPool
{
public:
    int Size() const { return BLE_LAYER_NUM_BLE_ENDPOINTS; }

    BLEEndPoint * Get(int i) const
    {
        static union
        {
            uint8_t Pool[sizeof(BLEEndPoint) * BLE_LAYER_NUM_BLE_ENDPOINTS];
            BLEEndPoint::AlignT ForceAlignment;
        } sEndPointPool;

        if (i < BLE_LAYER_NUM_BLE_ENDPOINTS)
        {
            return (BLEEndPoint *) (sEndPointPool.Pool + (sizeof(BLEEndPoint) * i));
        }
        else
        {
            return NULL;
        }
    }

    BLEEndPoint * Find(BLE_CONNECTION_OBJECT c)
    {
        if (c == BLE_CONNECTION_UNINITIALIZED)
        {
            return NULL;
        }

        for (int i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
        {
            BLEEndPoint * elem = Get(i);
            if (elem->mBle != NULL && elem->mConnObj == c)
            {
                return elem;
            }
        }

        return NULL;
    }

    BLEEndPoint * GetFree() const
    {
        for (int i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
        {
            BLEEndPoint * elem = Get(i);
            if (elem->mBle == NULL)
            {
                return elem;
            }
        }
        return NULL;
    }
};

// EndPoint Pools
//
static BleEndPointPool sBLEEndPointPool;

// UUIDs used internally by BleLayer:

const WeaveBleUUID BleLayer::WEAVE_BLE_CHAR_1_ID = { { // 18EE2EF5-263D-4559-959F-4F9C429F9D11
                                                       0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42,
                                                       0x9F, 0x9D, 0x11 } };

const WeaveBleUUID BleLayer::WEAVE_BLE_CHAR_2_ID = { { // 18EE2EF5-263D-4559-959F-4F9C429F9D12
                                                       0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42,
                                                       0x9F, 0x9D, 0x12 } };

void BleLayerObject::Release()
{
    // Decrement the ref count.  When it reaches zero, NULL out the pointer to the Weave::System::Layer
    // object. This effectively declared the object free and ready for re-allocation.
    mRefCount--;
    if (mRefCount == 0)
    {
        mBle = NULL;
    }
}

// BleTransportCapabilitiesRequestMessage implementation:

void BleTransportCapabilitiesRequestMessage::SetSupportedProtocolVersion(uint8_t index, uint8_t version)
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
        version = version << 4;
    }

    version &= mask;

    mSupportedProtocolVersions[(index / 2)] &= ~mask; // Clear version at index; leave other version in same byte alone
    mSupportedProtocolVersions[(index / 2)] |= version;
}

BLE_ERROR BleTransportCapabilitiesRequestMessage::Encode(PacketBuffer * msgBuf) const
{
    uint8_t * p   = msgBuf->Start();
    BLE_ERROR err = BLE_NO_ERROR;

    // Verify we can write the fixed-length request without running into the end of the buffer.
    VerifyOrExit(msgBuf->MaxDataLength() >= CAPABILITIES_REQUEST_LEN, err = BLE_ERROR_NO_MEMORY);

    nl::Weave::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_1);
    nl::Weave::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_2);

    for (int i = 0; i < CAPABILITIES_REQUEST_SUPPORTED_VERSIONS_LEN; i++)
    {
        nl::Weave::Encoding::Write8(p, mSupportedProtocolVersions[i]);
    }

    nl::Weave::Encoding::LittleEndian::Write16(p, mMtu);
    nl::Weave::Encoding::Write8(p, mWindowSize);

    msgBuf->SetDataLength(CAPABILITIES_REQUEST_LEN);

exit:
    return err;
}

BLE_ERROR BleTransportCapabilitiesRequestMessage::Decode(const PacketBuffer & msgBuf, BleTransportCapabilitiesRequestMessage & msg)
{
    const uint8_t * p = msgBuf.Start();
    BLE_ERROR err     = BLE_NO_ERROR;

    // Verify we can read the fixed-length request without running into the end of the buffer.
    VerifyOrExit(msgBuf.DataLength() >= CAPABILITIES_REQUEST_LEN, err = BLE_ERROR_MESSAGE_INCOMPLETE);

    VerifyOrExit(CAPABILITIES_MSG_CHECK_BYTE_1 == nl::Weave::Encoding::Read8(p), err = BLE_ERROR_INVALID_MESSAGE);
    VerifyOrExit(CAPABILITIES_MSG_CHECK_BYTE_2 == nl::Weave::Encoding::Read8(p), err = BLE_ERROR_INVALID_MESSAGE);

    for (int i = 0; i < CAPABILITIES_REQUEST_SUPPORTED_VERSIONS_LEN; i++)
    {
        msg.mSupportedProtocolVersions[i] = nl::Weave::Encoding::Read8(p);
    }

    msg.mMtu        = nl::Weave::Encoding::LittleEndian::Read16(p);
    msg.mWindowSize = nl::Weave::Encoding::Read8(p);

exit:
    return err;
}

// BleTransportCapabilitiesResponseMessage implementation:

BLE_ERROR BleTransportCapabilitiesResponseMessage::Encode(PacketBuffer * msgBuf) const
{
    uint8_t * p   = msgBuf->Start();
    BLE_ERROR err = BLE_NO_ERROR;

    // Verify we can write the fixed-length request without running into the end of the buffer.
    VerifyOrExit(msgBuf->MaxDataLength() >= CAPABILITIES_RESPONSE_LEN, err = BLE_ERROR_NO_MEMORY);

    nl::Weave::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_1);
    nl::Weave::Encoding::Write8(p, CAPABILITIES_MSG_CHECK_BYTE_2);

    nl::Weave::Encoding::Write8(p, mSelectedProtocolVersion);
    nl::Weave::Encoding::LittleEndian::Write16(p, mFragmentSize);
    nl::Weave::Encoding::Write8(p, mWindowSize);

    msgBuf->SetDataLength(CAPABILITIES_RESPONSE_LEN);

exit:
    return err;
}

BLE_ERROR BleTransportCapabilitiesResponseMessage::Decode(const PacketBuffer & msgBuf,
                                                          BleTransportCapabilitiesResponseMessage & msg)
{
    const uint8_t * p = msgBuf.Start();
    BLE_ERROR err     = BLE_NO_ERROR;

    // Verify we can read the fixed-length response without running into the end of the buffer.
    VerifyOrExit(msgBuf.DataLength() >= CAPABILITIES_RESPONSE_LEN, err = BLE_ERROR_MESSAGE_INCOMPLETE);

    VerifyOrExit(CAPABILITIES_MSG_CHECK_BYTE_1 == nl::Weave::Encoding::Read8(p), err = BLE_ERROR_INVALID_MESSAGE);
    VerifyOrExit(CAPABILITIES_MSG_CHECK_BYTE_2 == nl::Weave::Encoding::Read8(p), err = BLE_ERROR_INVALID_MESSAGE);

    msg.mSelectedProtocolVersion = nl::Weave::Encoding::Read8(p);
    msg.mFragmentSize            = nl::Weave::Encoding::LittleEndian::Read16(p);
    msg.mWindowSize              = nl::Weave::Encoding::Read8(p);

exit:
    return err;
}

// BleLayer implementation:

BleLayer::BleLayer()
{
    mState = kState_NotInitialized;
}

BLE_ERROR BleLayer::Init(BlePlatformDelegate * platformDelegate, BleApplicationDelegate * appDelegate,
                         Weave::System::Layer * systemLayer)
{
    BLE_ERROR err = BLE_NO_ERROR;

    RegisterBleLayerErrorFormatter();

    VerifyOrExit(platformDelegate != NULL, err = BLE_ERROR_BAD_ARGS);
    VerifyOrExit(appDelegate != NULL, err = BLE_ERROR_BAD_ARGS);
    VerifyOrExit(systemLayer != NULL, err = BLE_ERROR_BAD_ARGS);

    if (mState != kState_NotInitialized)
    {
        return BLE_ERROR_INCORRECT_STATE;
    }

    mPlatformDelegate    = platformDelegate;
    mApplicationDelegate = appDelegate;
    mSystemLayer         = systemLayer;

    memset(&sBLEEndPointPool, 0, sizeof(sBLEEndPointPool));

    mState = kState_Initialized;

#if WEAVE_ENABLE_WOBLE_TEST
    mTestBleEndPoint = NULL;
#endif

exit:
    return err;
}

BLE_ERROR BleLayer::Shutdown()
{
    mState = kState_NotInitialized;

    // Close and free all BLE end points.
    for (int i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
    {
        BLEEndPoint * elem = sBLEEndPointPool.Get(i);

        // If end point was initialized, and has not since been freed...
        if (elem->mBle != NULL)
        {
            // If end point hasn't already been closed...
            if (elem->mState != BLEEndPoint::kState_Closed)
            {
                // Close end point such that callbacks are suppressed and pending transmissions aborted.
                elem->Abort();
            }

            // If end point was closed, but is still waiting for GATT unsubscribe to complete, free it anyway.
            // This cancels the unsubscribe timer (plus all the end point's other timers).
            if (elem->IsUnsubscribePending())
            {
                elem->Free();
            }
        }
    }

    return BLE_NO_ERROR;
}

BLE_ERROR BleLayer::NewBleEndPoint(BLEEndPoint ** retEndPoint, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose)
{
    *retEndPoint = NULL;

    if (mState != kState_Initialized)
    {
        return BLE_ERROR_INCORRECT_STATE;
    }

    if (connObj == BLE_CONNECTION_UNINITIALIZED)
    {
        return BLE_ERROR_BAD_ARGS;
    }

    *retEndPoint = sBLEEndPointPool.GetFree();
    if (*retEndPoint == NULL)
    {
        WeaveLogError(Ble, "%s endpoint pool FULL", "Ble");
        return BLE_ERROR_NO_ENDPOINTS;
    }

    (*retEndPoint)->Init(this, connObj, role, autoClose);

#if WEAVE_ENABLE_WOBLE_TEST
    mTestBleEndPoint = *retEndPoint;
#endif

    return BLE_NO_ERROR;
}

// Handle remote central's initiation of Weave over BLE protocol handshake.
BLE_ERROR BleLayer::HandleBleTransportConnectionInitiated(BLE_CONNECTION_OBJECT connObj, PacketBuffer * pBuf)
{
    BLE_ERROR err             = BLE_NO_ERROR;
    BLEEndPoint * newEndPoint = NULL;

    // Only BLE peripherals can receive GATT writes, so specify this role in our creation of the BLEEndPoint.
    // Set autoClose = false. Peripherals only notify the application when an end point releases a BLE connection.
    err = NewBleEndPoint(&newEndPoint, connObj, kBleRole_Peripheral, false);
    SuccessOrExit(err);

    newEndPoint->mAppState = mAppState;

    err  = newEndPoint->Receive(pBuf);
    pBuf = NULL;
    SuccessOrExit(err); // If we fail here, end point will have already released connection and freed itself.

exit:
    if (pBuf != NULL)
    {
        PacketBuffer::Free(pBuf);
    }

    // If we failed to allocate a new end point, release underlying BLE connection. Central's handshake will time out
    // if the application decides to keep the BLE connection open.
    if (newEndPoint == NULL)
    {
        mApplicationDelegate->NotifyWeaveConnectionClosed(connObj);
    }

    if (err != BLE_NO_ERROR)
    {
        WeaveLogError(Ble, "HandleWeaveConnectionReceived failed, err = %d", err);
    }

    return err;
}

bool BleLayer::HandleWriteReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                                   PacketBuffer * pBuf)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        WeaveLogError(Ble, "ble write rcvd on unknown svc id");
        ExitNow();
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_1_ID, charId))
    {
        if (pBuf == NULL)
        {
            WeaveLogError(Ble, "rcvd null ble write");
            ExitNow();
        }

        // Find matching connection end point.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != NULL)
        {
            BLE_ERROR status = endPoint->Receive(pBuf);
            pBuf             = NULL;
            if (status != BLE_NO_ERROR)
            {
                WeaveLogError(Ble, "BLEEndPoint rcv failed, err = %d", status);
            }
        }
        else
        {
            BLE_ERROR status = HandleBleTransportConnectionInitiated(connObj, pBuf);
            pBuf             = NULL;
            if (status != BLE_NO_ERROR)
            {
                WeaveLogError(Ble, "failed handle new Weave BLE connection, status = %d", status);
            }
        }
    }
    else
    {
        WeaveLogError(Ble, "ble write rcvd on unknown char");
    }

exit:
    if (pBuf != NULL)
    {
        PacketBuffer::Free(pBuf);
    }

    return true;
}

bool BleLayer::HandleIndicationReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId,
                                        PacketBuffer * pBuf)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_2_ID, charId))
    {
        if (pBuf == NULL)
        {
            WeaveLogError(Ble, "rcvd null ble indication");
            ExitNow();
        }

        // find matching connection end point.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != NULL)
        {
            BLE_ERROR status = endPoint->Receive(pBuf);
            pBuf             = NULL;
            if (status != BLE_NO_ERROR)
            {
                WeaveLogError(Ble, "BLEEndPoint rcv failed, err = %d", status);
            }
        }
        else
        {
            WeaveLogDetail(Ble, "no endpoint for rcvd indication");
        }
    }
    else
    {
        WeaveLogError(Ble, "ble ind rcvd on unknown char");
    }

exit:
    if (pBuf != NULL)
    {
        PacketBuffer::Free(pBuf);
    }

    return true;
}

bool BleLayer::HandleWriteConfirmation(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_1_ID, charId))
    {
        HandleAckReceived(connObj);
    }
    else
    {
        WeaveLogError(Ble, "ble write con rcvd on unknown char");
    }

    return true;
}

bool BleLayer::HandleIndicationConfirmation(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_2_ID, charId))
    {
        HandleAckReceived(connObj);
    }
    else
    {
        WeaveLogError(Ble, "ble ind con rcvd on unknown char");
    }

    return true;
}

void BleLayer::HandleAckReceived(BLE_CONNECTION_OBJECT connObj)
{
    // find matching connection end point.
    BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

    if (endPoint != NULL)
    {
        BLE_ERROR status = endPoint->HandleGattSendConfirmationReceived();

        if (status != BLE_NO_ERROR)
        {
            WeaveLogError(Ble, "endpoint conf recvd failed, err = %d", status);
        }
    }
    else
    {
        WeaveLogError(Ble, "no endpoint for BLE sent data ack");
    }
}

bool BleLayer::HandleSubscribeReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_2_ID, charId))
    {
        // Find end point already associated with BLE connection, if any.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != NULL)
        {
            endPoint->HandleSubscribeReceived();
        }
        else
        {
            WeaveLogError(Ble, "no endpoint for sub recvd");
        }
    }

    return true;
}

bool BleLayer::HandleSubscribeComplete(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_2_ID, charId))
    {
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != NULL)
        {
            endPoint->HandleSubscribeComplete();
        }
        else
        {
            WeaveLogError(Ble, "no endpoint for sub complete");
        }
    }

    return true;
}

bool BleLayer::HandleUnsubscribeReceived(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_2_ID, charId))
    {
        // Find end point already associated with BLE connection, if any.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != NULL)
        {
            endPoint->DoClose(kBleCloseFlag_AbortTransmission, BLE_ERROR_CENTRAL_UNSUBSCRIBED);
        }
        else
        {
            WeaveLogError(Ble, "no endpoint for unsub recvd");
        }
    }

    return true;
}

bool BleLayer::HandleUnsubscribeComplete(BLE_CONNECTION_OBJECT connObj, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    if (!UUIDsMatch(&WEAVE_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&WEAVE_BLE_CHAR_2_ID, charId))
    {
        // Find end point already associated with BLE connection, if any.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != NULL)
        {
            endPoint->HandleUnsubscribeComplete();
        }
        else
        {
            WeaveLogError(Ble, "no endpoint for unsub complete");
        }
    }

    return true;
}

void BleLayer::HandleConnectionError(BLE_CONNECTION_OBJECT connObj, BLE_ERROR err)
{
    // BLE connection has failed somehow, we must find and abort matching connection end point.
    BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

    if (endPoint != NULL)
    {
        if (err == BLE_ERROR_GATT_UNSUBSCRIBE_FAILED && endPoint->IsUnsubscribePending())
        {
            // If end point was already closed and just waiting for unsubscribe to complete, free it. Call to Free()
            // stops unsubscribe timer.
            endPoint->Free();
        }
        else
        {
            endPoint->DoClose(kBleCloseFlag_AbortTransmission, err);
        }
    }
}

BleTransportProtocolVersion BleLayer::GetHighestSupportedProtocolVersion(const BleTransportCapabilitiesRequestMessage & reqMsg)
{
    BleTransportProtocolVersion retVersion = kBleTransportProtocolVersion_None;

    uint8_t shift_width = 4;

    for (int i = 0; i < NUM_SUPPORTED_PROTOCOL_VERSIONS; i++)
    {
        shift_width ^= 4;

        uint8_t version = reqMsg.mSupportedProtocolVersions[(i / 2)];
        version         = (version >> shift_width) & 0x0F; // Grab just the nibble we want.

        if ((version >= NL_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION) &&
            (version <= NL_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION) && (version > retVersion))
        {
            retVersion = static_cast<BleTransportProtocolVersion>(version);
        }
        else if (version == kBleTransportProtocolVersion_None) // Signifies end of supported versions list
        {
            break;
        }
    }

    return retVersion;
}

} /* namespace Ble */
} /* namespace nl */

#endif /* CONFIG_NETWORK_LAYER_BLE */
