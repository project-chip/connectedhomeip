/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      a platform's Bluetooth Low Energy (BLE) implementation and the CHIP
 *      stack.
 *
 *      The BleLayer obect accepts BLE data and control input from the
 *      application via a functional interface. It performs the fragmentation
 *      and reassembly required to transmit CHIP message via a BLE GATT
 *      characteristic interface, and drives incoming messages up the CHIP
 *      stack.
 *
 *      During initialization, the BleLayer object requires a pointer to the
 *      platform's implementation of the BlePlatformDelegate and
 *      BleApplicationDelegate objects.
 *
 *      The BlePlatformDelegate provides the CHIP stack with an interface
 *      by which to form and cancel GATT subscriptions, read and write
 *      GATT characteristic values, send GATT characteristic notifications,
 *      respond to GATT read requests, and close BLE connections.
 *
 *      The BleApplicationDelegate provides a mechanism for CHIP to inform
 *      the application when it has finished using a given BLE connection,
 *      i.e when the chipConnection object wrapping this connection has
 *      closed. This allows the application to either close the BLE connection
 *      or continue to keep it open for non-CHIP purposes.
 *
 *      To enable CHIP over BLE for a new platform, the application developer
 *      must provide an implementation for both delegates, provides points to
 *      instances of these delegates on startup, and ensure that the
 *      application calls the necessary BleLayer functions when appropriate to
 *      drive BLE data and control input up the stack.
 */

#include <ble/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE

#include <string.h>

#include <ble/BLEEndPoint.h>
#include <ble/BleApplicationDelegate.h>
#include <ble/BleLayer.h>
#include <ble/BlePlatformDelegate.h>
#include <ble/BleUUID.h>

#include <core/CHIPEncoding.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

// Magic values expected in first 2 bytes of valid BLE transport capabilities request or response:
#define CAPABILITIES_MSG_CHECK_BYTE_1 'n'
#define CAPABILITIES_MSG_CHECK_BYTE_2 'l'

namespace chip {
namespace Ble {

class BleEndPointPool
{
public:
    int Size() const { return BLE_LAYER_NUM_BLE_ENDPOINTS; }

    BLEEndPoint * Get(size_t i) const
    {
        static union
        {
            uint8_t Pool[sizeof(BLEEndPoint) * BLE_LAYER_NUM_BLE_ENDPOINTS];
            BLEEndPoint::AlignT ForceAlignment;
        } sEndPointPool;

        if (i < BLE_LAYER_NUM_BLE_ENDPOINTS)
        {
            return reinterpret_cast<BLEEndPoint *>(sEndPointPool.Pool + (sizeof(BLEEndPoint) * i));
        }

        return nullptr;
    }

    BLEEndPoint * Find(BLE_CONNECTION_OBJECT c)
    {
        if (c == BLE_CONNECTION_UNINITIALIZED)
        {
            return nullptr;
        }

        for (size_t i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
        {
            BLEEndPoint * elem = Get(i);
            if (elem->mBle != nullptr && elem->mConnObj == c)
            {
                return elem;
            }
        }

        return nullptr;
    }

    BLEEndPoint * GetFree() const
    {
        for (size_t i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
        {
            BLEEndPoint * elem = Get(i);
            if (elem->mBle == nullptr)
            {
                return elem;
            }
        }
        return nullptr;
    }
};

// EndPoint Pools
//
static BleEndPointPool sBLEEndPointPool;

// UUIDs used internally by BleLayer:

const ChipBleUUID BleLayer::CHIP_BLE_CHAR_1_ID = { { // 18EE2EF5-263D-4559-959F-4F9C429F9D11
                                                     0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42,
                                                     0x9F, 0x9D, 0x11 } };

const ChipBleUUID BleLayer::CHIP_BLE_CHAR_2_ID = { { // 18EE2EF5-263D-4559-959F-4F9C429F9D12
                                                     0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42,
                                                     0x9F, 0x9D, 0x12 } };

const ChipBleUUID BleLayer::CHIP_BLE_CHAR_3_ID = { { // 64630238-8772-45F2-B87D-748A83218F04
                                                     0x64, 0x63, 0x02, 0x38, 0x87, 0x72, 0x45, 0xF2, 0xB8, 0x7D, 0x74, 0x8A, 0x83,
                                                     0x21, 0x8F, 0x04 } };

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
        version = static_cast<uint8_t>(version << 4);
    }

    version &= mask;

    uint8_t & slot = mSupportedProtocolVersions[(index / 2)];
    slot           = static_cast<uint8_t>(slot & ~mask); // Clear version at index; leave other version in same byte alone
    slot |= version;
}

CHIP_ERROR BleTransportCapabilitiesRequestMessage::Encode(const PacketBufferHandle & msgBuf) const
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

CHIP_ERROR BleTransportCapabilitiesRequestMessage::Decode(const PacketBufferHandle & msgBuf,
                                                          BleTransportCapabilitiesRequestMessage & msg)
{
    const uint8_t * p = msgBuf->Start();

    // Verify we can read the fixed-length request without running into the end of the buffer.
    VerifyOrReturnError(msgBuf->DataLength() >= kCapabilitiesRequestLength, CHIP_ERROR_MESSAGE_INCOMPLETE);

    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_1 == chip::Encoding::Read8(p), BLE_ERROR_INVALID_MESSAGE);
    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_2 == chip::Encoding::Read8(p), BLE_ERROR_INVALID_MESSAGE);

    for (size_t i = 0; i < kCapabilitiesRequestSupportedVersionsLength; i++)
    {
        msg.mSupportedProtocolVersions[i] = chip::Encoding::Read8(p);
    }

    msg.mMtu        = chip::Encoding::LittleEndian::Read16(p);
    msg.mWindowSize = chip::Encoding::Read8(p);

    return CHIP_NO_ERROR;
}

// BleTransportCapabilitiesResponseMessage implementation:

CHIP_ERROR BleTransportCapabilitiesResponseMessage::Encode(const PacketBufferHandle & msgBuf) const
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

CHIP_ERROR BleTransportCapabilitiesResponseMessage::Decode(const PacketBufferHandle & msgBuf,
                                                           BleTransportCapabilitiesResponseMessage & msg)
{
    const uint8_t * p = msgBuf->Start();

    // Verify we can read the fixed-length response without running into the end of the buffer.
    VerifyOrReturnError(msgBuf->DataLength() >= kCapabilitiesResponseLength, CHIP_ERROR_MESSAGE_INCOMPLETE);

    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_1 == chip::Encoding::Read8(p), BLE_ERROR_INVALID_MESSAGE);
    VerifyOrReturnError(CAPABILITIES_MSG_CHECK_BYTE_2 == chip::Encoding::Read8(p), BLE_ERROR_INVALID_MESSAGE);

    msg.mSelectedProtocolVersion = chip::Encoding::Read8(p);
    msg.mFragmentSize            = chip::Encoding::LittleEndian::Read16(p);
    msg.mWindowSize              = chip::Encoding::Read8(p);

    return CHIP_NO_ERROR;
}

// BleLayer implementation:

BleLayer::BleLayer()
{
    mState = kState_NotInitialized;
}

CHIP_ERROR BleLayer::Init(BlePlatformDelegate * platformDelegate, BleConnectionDelegate * connDelegate,
                          BleApplicationDelegate * appDelegate, chip::System::Layer * systemLayer)
{
    Ble::RegisterLayerErrorFormatter();

    // It is totally valid to not have a connDelegate. In this case the client application
    // will take care of the connection steps.
    VerifyOrReturnError(platformDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(appDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (mState != kState_NotInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mConnectionDelegate  = connDelegate;
    mPlatformDelegate    = platformDelegate;
    mApplicationDelegate = appDelegate;
    mSystemLayer         = systemLayer;

    memset(&sBLEEndPointPool, 0, sizeof(sBLEEndPointPool));

    mState = kState_Initialized;

#if CHIP_ENABLE_CHIPOBLE_TEST
    mTestBleEndPoint = NULL;
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR BleLayer::Init(BlePlatformDelegate * platformDelegate, BleApplicationDelegate * appDelegate,
                          chip::System::Layer * systemLayer)
{
    return Init(platformDelegate, nullptr, appDelegate, systemLayer);
}

CHIP_ERROR BleLayer::Shutdown()
{
    mState = kState_NotInitialized;
    return CloseAllBleConnections();
}

CHIP_ERROR BleLayer::CloseAllBleConnections()
{
    // Close and free all BLE end points.
    for (size_t i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
    {
        BLEEndPoint * elem = sBLEEndPointPool.Get(i);

        // If end point was initialized, and has not since been freed...
        if (elem->mBle != nullptr)
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
    return CHIP_NO_ERROR;
}

CHIP_ERROR BleLayer::CloseBleConnection(BLE_CONNECTION_OBJECT connObj)
{
    // Close and free all BLE endpoints.
    for (size_t i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
    {
        BLEEndPoint * elem = sBLEEndPointPool.Get(i);

        // If end point was initialized, and has not since been freed...
        if (elem->mBle != nullptr && elem->ConnectionObjectIs(connObj))
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
    return CHIP_NO_ERROR;
}

CHIP_ERROR BleLayer::CancelBleIncompleteConnection()
{
    VerifyOrReturnError(mState == kState_Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = mConnectionDelegate->CancelConnection();
    if (err == CHIP_ERROR_NOT_IMPLEMENTED)
    {
        ChipLogError(Ble, "BleConnectionDelegate::CancelConnection is not implemented.");
    }
    return err;
}

CHIP_ERROR BleLayer::NewBleConnectionByDiscriminator(uint16_t connDiscriminator)
{

    VerifyOrReturnError(mState == kState_Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mBleTransport != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mConnectionDelegate->OnConnectionComplete = OnConnectionComplete;
    mConnectionDelegate->OnConnectionError    = OnConnectionError;
    // TODO: We are passing the same parameter two times, should take a look at it to see if we can remove one of them.
    mConnectionDelegate->NewConnection(this, this, connDiscriminator);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BleLayer::NewBleConnectionByObject(BLE_CONNECTION_OBJECT connObj)
{
    VerifyOrReturnError(mState == kState_Initialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mBleTransport != nullptr, CHIP_ERROR_INCORRECT_STATE);

    OnConnectionComplete(this, connObj);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BleLayer::NewBleEndPoint(BLEEndPoint ** retEndPoint, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose)
{
    *retEndPoint = nullptr;

    if (mState != kState_Initialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (connObj == BLE_CONNECTION_UNINITIALIZED)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *retEndPoint = sBLEEndPointPool.GetFree();
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Ble, "%s endpoint pool FULL", "Ble");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    (*retEndPoint)->Init(this, connObj, role, autoClose);
    (*retEndPoint)->mBleTransport = mBleTransport;

#if CHIP_ENABLE_CHIPOBLE_TEST
    mTestBleEndPoint = *retEndPoint;
#endif

    return CHIP_NO_ERROR;
}

// Handle remote central's initiation of CHIP over BLE protocol handshake.
CHIP_ERROR BleLayer::HandleBleTransportConnectionInitiated(BLE_CONNECTION_OBJECT connObj, PacketBufferHandle && pBuf)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    BLEEndPoint * newEndPoint = nullptr;

    // Only BLE peripherals can receive GATT writes, so specify this role in our creation of the BLEEndPoint.
    // Set autoClose = false. Peripherals only notify the application when an end point releases a BLE connection.
    err = NewBleEndPoint(&newEndPoint, connObj, kBleRole_Peripheral, false);
    SuccessOrExit(err);

    newEndPoint->mBleTransport = mBleTransport;

    err = newEndPoint->Receive(std::move(pBuf));
    SuccessOrExit(err); // If we fail here, end point will have already released connection and freed itself.

exit:
    // If we failed to allocate a new end point, release underlying BLE connection. Central's handshake will time out
    // if the application decides to keep the BLE connection open.
    if (newEndPoint == nullptr)
    {
        mApplicationDelegate->NotifyChipConnectionClosed(connObj);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "HandleChipConnectionReceived failed, err = %" CHIP_ERROR_FORMAT, err);
    }

    return err;
}

bool BleLayer::HandleWriteReceived(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                   PacketBufferHandle && pBuf)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        ChipLogError(Ble, "ble write rcvd on unknown svc id");
        return true;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_1_ID, charId))
    {
        if (pBuf.IsNull())
        {
            ChipLogError(Ble, "rcvd null ble write");
            return true;
        }

        // Find matching connection end point.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != nullptr)
        {
            CHIP_ERROR status = endPoint->Receive(std::move(pBuf));
            if (status != CHIP_NO_ERROR)
            {
                ChipLogError(Ble, "BLEEndPoint rcv failed, err = %" CHIP_ERROR_FORMAT, status);
            }
        }
        else
        {
            CHIP_ERROR status = HandleBleTransportConnectionInitiated(connObj, std::move(pBuf));
            if (status != CHIP_NO_ERROR)
            {
                ChipLogError(Ble, "failed handle new chip BLE connection, status = %" CHIP_ERROR_FORMAT, status);
            }
        }
    }
    else
    {
        ChipLogError(Ble, "ble write rcvd on unknown char");
    }

    return true;
}

bool BleLayer::HandleIndicationReceived(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                        PacketBufferHandle && pBuf)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_2_ID, charId))
    {
        if (pBuf.IsNull())
        {
            ChipLogError(Ble, "rcvd null ble indication");
            return true;
        }

        // find matching connection end point.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != nullptr)
        {
            CHIP_ERROR status = endPoint->Receive(std::move(pBuf));
            if (status != CHIP_NO_ERROR)
            {
                ChipLogError(Ble, "BLEEndPoint rcv failed, err = %" CHIP_ERROR_FORMAT, status);
            }
        }
        else
        {
            ChipLogDetail(Ble, "no endpoint for rcvd indication");
        }
    }
    else
    {
        ChipLogError(Ble, "ble ind rcvd on unknown char");
    }

    return true;
}

bool BleLayer::HandleWriteConfirmation(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_1_ID, charId))
    {
        HandleAckReceived(connObj);
    }
    else
    {
        ChipLogError(Ble, "ble write con rcvd on unknown char");
    }

    return true;
}

bool BleLayer::HandleIndicationConfirmation(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_2_ID, charId))
    {
        HandleAckReceived(connObj);
    }
    else
    {
        ChipLogError(Ble, "ble ind con rcvd on unknown char");
    }

    return true;
}

void BleLayer::HandleAckReceived(BLE_CONNECTION_OBJECT connObj)
{
    // find matching connection end point.
    BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

    if (endPoint != nullptr)
    {
        CHIP_ERROR status = endPoint->HandleGattSendConfirmationReceived();

        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(Ble, "endpoint conf recvd failed, err = %" CHIP_ERROR_FORMAT, status);
        }
    }
    else
    {
        ChipLogError(Ble, "no endpoint for BLE sent data ack");
    }
}

bool BleLayer::HandleSubscribeReceived(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_2_ID, charId) || UUIDsMatch(&CHIP_BLE_CHAR_3_ID, charId))
    {
        // Find end point already associated with BLE connection, if any.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != nullptr)
        {
            endPoint->HandleSubscribeReceived();
        }
        else
        {
            ChipLogError(Ble, "no endpoint for sub recvd");
        }
    }

    return true;
}

bool BleLayer::HandleSubscribeComplete(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_2_ID, charId) || UUIDsMatch(&CHIP_BLE_CHAR_3_ID, charId))
    {
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != nullptr)
        {
            endPoint->HandleSubscribeComplete();
        }
        else
        {
            ChipLogError(Ble, "no endpoint for sub complete");
        }
    }

    return true;
}

bool BleLayer::HandleUnsubscribeReceived(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_2_ID, charId) || UUIDsMatch(&CHIP_BLE_CHAR_3_ID, charId))
    {
        // Find end point already associated with BLE connection, if any.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != nullptr)
        {
            endPoint->DoClose(kBleCloseFlag_AbortTransmission, BLE_ERROR_CENTRAL_UNSUBSCRIBED);
        }
        else
        {
            ChipLogError(Ble, "no endpoint for unsub recvd");
        }
    }

    return true;
}

bool BleLayer::HandleUnsubscribeComplete(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    if (!UUIDsMatch(&CHIP_BLE_SVC_ID, svcId))
    {
        return false;
    }

    if (UUIDsMatch(&CHIP_BLE_CHAR_2_ID, charId) || UUIDsMatch(&CHIP_BLE_CHAR_3_ID, charId))
    {
        // Find end point already associated with BLE connection, if any.
        BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

        if (endPoint != nullptr)
        {
            endPoint->HandleUnsubscribeComplete();
        }
        else
        {
            ChipLogError(Ble, "no endpoint for unsub complete");
        }
    }

    return true;
}

void BleLayer::HandleConnectionError(BLE_CONNECTION_OBJECT connObj, CHIP_ERROR err)
{
    // BLE connection has failed somehow, we must find and abort matching connection end point.
    BLEEndPoint * endPoint = sBLEEndPointPool.Find(connObj);

    if (endPoint != nullptr)
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

        if ((version >= CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION) &&
            (version <= CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION) && (version > retVersion))
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

void BleLayer::OnConnectionComplete(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    BleLayer * layer       = reinterpret_cast<BleLayer *>(appState);
    BLEEndPoint * endPoint = nullptr;
    CHIP_ERROR err         = CHIP_NO_ERROR;

    SuccessOrExit(err = layer->NewBleEndPoint(&endPoint, connObj, kBleRole_Central, true));
    layer->mBleTransport->OnBleConnectionComplete(endPoint);

exit:
    if (err != CHIP_NO_ERROR)
    {
        OnConnectionError(layer, err);
    }
}

void BleLayer::OnConnectionError(void * appState, CHIP_ERROR err)
{
    BleLayer * layer = reinterpret_cast<BleLayer *>(appState);
    layer->mBleTransport->OnBleConnectionError(err);
}

} /* namespace Ble */
} /* namespace chip */

#endif /* CONFIG_NETWORK_LAYER_BLE */
