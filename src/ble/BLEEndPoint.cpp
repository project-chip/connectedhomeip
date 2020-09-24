/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a Bluetooth Low Energy (BLE) connection
 *      endpoint abstraction for the byte-streaming,
 *      connection-oriented CHIP over Bluetooth Low Energy (CHIPoBLE)
 *      Bluetooth Transport Protocol (BTP).
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <string.h>

#include <ble/BleConfig.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <core/CHIPConfig.h>

#include <support/CHIPFaultInjection.h>
#include <support/CodeUtils.h>
#include <support/FlagUtils.hpp>
#include <support/logging/CHIPLogging.h>

#include <ble/BLEEndPoint.h>
#include <ble/BleLayer.h>
#include <ble/BtpEngine.h>
#if CHIP_ENABLE_CHIPOBLE_TEST
#include "BtpEngineTest.h"
#endif

// clang-format off

// Define below to enable extremely verbose, BLE end point-specific debug logging.
#undef CHIP_BLE_END_POINT_DEBUG_LOGGING_ENABLED

#ifdef CHIP_BLE_END_POINT_DEBUG_LOGGING_ENABLED
#define ChipLogDebugBleEndPoint(MOD, MSG, ...) ChipLogError(MOD, MSG, ## __VA_ARGS__)
#else
#define ChipLogDebugBleEndPoint(MOD, MSG, ...)
#endif

/**
 *  @def BLE_CONFIG_IMMEDIATE_ACK_WINDOW_THRESHOLD
 *
 *  @brief
 *    If an end point's receive window drops equal to or below this value, it will send an immediate acknowledgement
 *    packet to re-open its window instead of waiting for the send-ack timer to expire.
 *
 */
#define BLE_CONFIG_IMMEDIATE_ACK_WINDOW_THRESHOLD                   1

/**
 * @def BLE_CONNECT_TIMEOUT_MS
 *
 * @brief
 *   This is the amount of time, in milliseconds, after a BLE end point initiates a transport protocol connection
 *   or receives the initial portion of a connect request before the end point will automatically release its BLE
 *   connection and free itself if the transport connection has not been established.
 *
 */
#define BLE_CONNECT_TIMEOUT_MS                                5000 // 5 seconds

/**
 *  @def BLE_UNSUBSCRIBE_TIMEOUT_MS
 *
 *  @brief
 *    This is amount of time, in milliseconds, which a BLE end point will wait for an unsubscribe operation to complete
 *    before it automatically releases its BLE connection and frees itself. The default value of 5 seconds is arbitary.
 *
 */
#define BLE_UNSUBSCRIBE_TIMEOUT_MS                            5000 // 5 seconds

#define BTP_ACK_RECEIVED_TIMEOUT_MS                          15000 // 15 seconds
#define BTP_ACK_SEND_TIMEOUT_MS                               2500 // 2.5 seconds

#define BTP_WINDOW_NO_ACK_SEND_THRESHOLD                         1 // Data fragments may only be sent without piggybacked
                                                                   // acks if receiver's window size is above this threshold.

// clang-format on

namespace chip {
namespace Ble {

BLE_ERROR BLEEndPoint::StartConnect()
{
    BLE_ERROR err = BLE_NO_ERROR;
    BleTransportCapabilitiesRequestMessage req;
    PacketBuffer * buf = nullptr;
    int i;
    int numVersions;

    // Ensure we're in the correct state.
    VerifyOrExit(mState == kState_Ready, err = BLE_ERROR_INCORRECT_STATE);
    mState = kState_Connecting;

    // Build BLE transport protocol capabilities request.
    buf = PacketBuffer::New();
    VerifyOrExit(buf != nullptr, err = BLE_ERROR_NO_MEMORY);

    // Zero-initialize BLE transport capabilities request.
    memset(&req, 0, sizeof(req));

    req.mMtu = mBle->mPlatformDelegate->GetMTU(mConnObj);

    req.mWindowSize = BLE_MAX_RECEIVE_WINDOW_SIZE;

    // Populate request with highest supported protocol versions
    numVersions = CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION - CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION + 1;
    VerifyOrExit(numVersions <= NUM_SUPPORTED_PROTOCOL_VERSIONS, err = BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS);
    for (i = 0; i < numVersions; i++)
    {
        req.SetSupportedProtocolVersion(i, CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION - i);
    }

    err = req.Encode(buf);
    SuccessOrExit(err);

    // Start connect timer. Canceled when end point freed or connection established.
    err = StartConnectTimer();
    SuccessOrExit(err);

    // Send BLE transport capabilities request to peripheral via GATT write.
    if (!SendWrite(buf))
    {
        err = BLE_ERROR_GATT_WRITE_FAILED;
        ExitNow();
    }

    // Free request buffer on write confirmation. Stash a reference to it in mSendQueue, which we don't use anyway
    // until the connection has been set up.
    QueueTx(buf, kType_Data);
    buf = nullptr;

exit:
    if (buf != nullptr)
    {
        PacketBuffer::Free(buf);
    }

    // If we failed to initiate the connection, close the end point.
    if (err != BLE_NO_ERROR)
    {
        StopConnectTimer();
        DoClose(kBleCloseFlag_AbortTransmission, err);
    }

    return err;
}

BLE_ERROR BLEEndPoint::HandleConnectComplete()
{
    BLE_ERROR err = BLE_NO_ERROR;

    mState = kState_Connected;

    // Cancel the connect timer.
    StopConnectTimer();

    // We've successfully completed the BLE transport protocol handshake, so let the application know we're open for business.
    if (OnConnectComplete != nullptr)
    {
        // Indicate connect complete to next-higher layer.
        OnConnectComplete(this, BLE_NO_ERROR);
    }
    else
    {
        // If no connect complete callback has been set up, close the end point.
        err = BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK;
    }

    return err;
}

BLE_ERROR BLEEndPoint::HandleReceiveConnectionComplete()
{
    BLE_ERROR err = BLE_NO_ERROR;

    ChipLogDebugBleEndPoint(Ble, "entered HandleReceiveConnectionComplete");
    mState = kState_Connected;

    // Cancel receive connection timer.
    StopReceiveConnectionTimer();

    // We've successfully completed the BLE transport protocol handshake, so let the application know we're open for business.
    if (mBle->OnChipBleConnectReceived != nullptr)
    {
        // Indicate BLE transport protocol connection received to next-higher layer.
        mBle->OnChipBleConnectReceived(this);
    }
    else
    {
        err = BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK;
    }

    return err;
}

void BLEEndPoint::HandleSubscribeReceived()
{
    BLE_ERROR err = BLE_NO_ERROR;

    VerifyOrExit(mState == kState_Connecting || mState == kState_Aborting, err = BLE_ERROR_INCORRECT_STATE);
    VerifyOrExit(mSendQueue != nullptr, err = BLE_ERROR_INCORRECT_STATE);

    // Send BTP capabilities response to peripheral via GATT indication.
#if CHIP_ENABLE_CHIPOBLE_TEST
    VerifyOrExit(mBtpEngine.PopPacketTag(mSendQueue) == kType_Data, err = BLE_ERROR_INVALID_BTP_HEADER_FLAGS);
#endif
    if (!SendIndication(mSendQueue))
    {
        // Ensure transmit queue is empty and set to NULL.
        QueueTxLock();
        PacketBuffer::Free(mSendQueue);
        mSendQueue = nullptr;
        QueueTxUnlock();

        ChipLogError(Ble, "cap resp ind failed");
        err = BLE_ERROR_GATT_INDICATE_FAILED;
        ExitNow();
    }

    // Shrink remote receive window counter by 1, since we've sent an indication which requires acknowledgement.
    mRemoteReceiveWindowSize -= 1;
    ChipLogDebugBleEndPoint(Ble, "decremented remote rx window, new size = %u", mRemoteReceiveWindowSize);

    // Start ack recvd timer for handshake indication.
    err = StartAckReceivedTimer();
    SuccessOrExit(err);

    ChipLogDebugBleEndPoint(Ble, "got subscribe, sent indication w/ capabilities response");

    // If SendIndication returns true, mSendQueue is freed on indication confirmation, or on close in case of
    // connection error.

    if (mState != kState_Aborting)
    {
        // If peripheral accepted the BTP connection, its end point must enter the connected state here, i.e. before it
        // receives a GATT confirmation for the capabilities response indication. This behavior is required to handle the
        // case where a peripheral's BLE controller passes up the central's first message fragment write before the
        // capabilities response indication confirmation. If the end point waited for this indication confirmation before
        // it entered the connected state, it'd be in the wrong state to receive the central's first data write, and drop
        // the corresponding message fragment.
        err = HandleReceiveConnectionComplete();
        SuccessOrExit(err);
    } // Else State == kState_Aborting, so we'll close end point when indication confirmation received.

exit:
    if (err != BLE_NO_ERROR)
    {
        DoClose(kBleCloseFlag_SuppressCallback | kBleCloseFlag_AbortTransmission, err);
    }

    return;
}

void BLEEndPoint::HandleSubscribeComplete()
{
    ChipLogProgress(Ble, "subscribe complete, ep = %p", this);
    SetFlag(mConnStateFlags, kConnState_GattOperationInFlight, false);

    BLE_ERROR err = DriveSending();

    if (err != BLE_NO_ERROR)
    {
        DoClose(kBleCloseFlag_AbortTransmission, BLE_NO_ERROR);
    }
}

void BLEEndPoint::HandleUnsubscribeComplete()
{
    // Don't bother to clear GattOperationInFlight, we're about to free the end point anyway.
    Free();
}

bool BLEEndPoint::IsConnected(uint8_t state) const
{
    return (state == kState_Connected || state == kState_Closing);
}

bool BLEEndPoint::IsUnsubscribePending() const
{
    return (GetFlag(mTimerStateFlags, kTimerState_UnsubscribeTimerRunning));
}

void BLEEndPoint::Abort()
{
    // No more callbacks after this point, since application explicitly called Abort().
    OnConnectComplete  = nullptr;
    OnConnectionClosed = nullptr;
    OnMessageReceived  = nullptr;
#if CHIP_ENABLE_CHIPOBLE_TEST
    OnCommandReceived = NULL;
#endif

    DoClose(kBleCloseFlag_SuppressCallback | kBleCloseFlag_AbortTransmission, BLE_NO_ERROR);
}

void BLEEndPoint::Close()
{
    // No more callbacks after this point, since application explicitly called Close().
    OnConnectComplete  = nullptr;
    OnConnectionClosed = nullptr;
    OnMessageReceived  = nullptr;
#if CHIP_ENABLE_CHIPOBLE_TEST
    OnCommandReceived = NULL;
#endif

    DoClose(kBleCloseFlag_SuppressCallback, BLE_NO_ERROR);
}

void BLEEndPoint::DoClose(uint8_t flags, BLE_ERROR err)
{
    uint8_t oldState = mState;

    // If end point is not closed or closing, OR end point was closing gracefully, but tx abort has been specified...
    if ((mState != kState_Closed && mState != kState_Closing) ||
        (mState == kState_Closing && (flags & kBleCloseFlag_AbortTransmission)))
    {
        // Cancel Connect and ReceiveConnect timers if they are running.
        // Check role first to avoid needless iteration over timer pool.
        if (mRole == kBleRole_Central)
        {
            StopConnectTimer();
        }
        else // (mRole == kBleRole_Peripheral), verified on Init
        {
            StopReceiveConnectionTimer();
        }

        // If transmit buffer is empty or a transmission abort was specified...
        if (mBtpEngine.TxState() == BtpEngine::kState_Idle || (flags & kBleCloseFlag_AbortTransmission))
        {
            FinalizeClose(oldState, flags, err);
        }
        else
        {
            // Wait for send queue and fragmenter's tx buffer to become empty, to ensure all pending messages have been
            // sent. Only free end point and tell platform it can throw away the underlying BLE connection once all
            // pending messages have been sent and acknowledged by the remote CHIPoBLE stack, or once the remote stack
            // closes the CHIPoBLE connection.
            //
            // In so doing, BLEEndPoint attempts to emulate the level of reliability afforded by TCPEndPoint and TCP
            // sockets in general with a typical default SO_LINGER option. That said, there is no hard guarantee that
            // pending messages will be sent once (Do)Close() is called, so developers should use application-level
            // messages to confirm the receipt of all data sent prior to a Close() call.
            mState = kState_Closing;

            if ((flags & kBleCloseFlag_SuppressCallback) == 0)
            {
                DoCloseCallback(oldState, flags, err);
            }
        }
    }
}

void BLEEndPoint::FinalizeClose(uint8_t oldState, uint8_t flags, BLE_ERROR err)
{
    mState = kState_Closed;

    // Ensure transmit queue is empty and set to NULL.
    QueueTxLock();
    PacketBuffer::Free(mSendQueue);
    mSendQueue = nullptr;
    QueueTxUnlock();

#if CHIP_ENABLE_CHIPOBLE_TEST
    PacketBuffer::Free(mBtpEngineTest.mCommandReceiveQueue);
    mBtpEngineTest.mCommandReceiveQueue = NULL;
#endif

    // Fire application's close callback if we haven't already, and it's not suppressed.
    if (oldState != kState_Closing && (flags & kBleCloseFlag_SuppressCallback) == 0)
    {
        DoCloseCallback(oldState, flags, err);
    }

    // If underlying BLE connection has closed, connection object is invalid, so just free the end point and return.
    if (err == BLE_ERROR_REMOTE_DEVICE_DISCONNECTED || err == BLE_ERROR_APP_CLOSED_CONNECTION)
    {
        mConnObj = BLE_CONNECTION_UNINITIALIZED; // Clear handle to BLE connection, so we don't double-close it.
        Free();
    }
    else // Otherwise, try to signal close to remote device before end point releases BLE connection and frees itself.
    {
        if (mRole == kBleRole_Central && GetFlag(mConnStateFlags, kConnState_DidBeginSubscribe))
        {
            // Cancel send and receive-ack timers, if running.
            StopAckReceivedTimer();
            StopSendAckTimer();

            // Indicate close of chipConnection to peripheral via GATT unsubscribe. Keep end point allocated until
            // unsubscribe completes or times out, so platform doesn't close underlying BLE connection before
            // we're really sure the unsubscribe request has been sent.
            if (!mBle->mPlatformDelegate->UnsubscribeCharacteristic(mConnObj, &CHIP_BLE_SVC_ID, &mBle->CHIP_BLE_CHAR_2_ID))
            {
                ChipLogError(Ble, "BtpEngine unsub failed");

                // If unsubscribe fails, release BLE connection and free end point immediately.
                Free();
            }
            else if (mConnObj != BLE_CONNECTION_UNINITIALIZED)
            {
                // Unsubscribe request was sent successfully, and a confirmation wasn't spontaneously generated or
                // received in the downcall to UnsubscribeCharacteristic, so set timer for the unsubscribe to complete.
                err = StartUnsubscribeTimer();

                if (err != BLE_NO_ERROR)
                {
                    Free();
                }

                // Mark unsubscribe GATT operation in progress.
                SetFlag(mConnStateFlags, kConnState_GattOperationInFlight, true);
            }
        }
        else // mRole == kBleRole_Peripheral, OR GetFlag(mTimerStateFlags, kConnState_DidBeginSubscribe) == false...
        {
            Free();
        }
    }
}

void BLEEndPoint::DoCloseCallback(uint8_t state, uint8_t flags, BLE_ERROR err)
{
    if (state == kState_Connecting)
    {
        if (OnConnectComplete != nullptr)
        {
            OnConnectComplete(this, err);
        }
    }
    else
    {
        if (OnConnectionClosed != nullptr)
        {
            OnConnectionClosed(this, err);
        }
    }

    // Callback fires once per end point lifetime.
    OnConnectComplete  = nullptr;
    OnConnectionClosed = nullptr;
}

void BLEEndPoint::ReleaseBleConnection()
{
    if (mConnObj != BLE_CONNECTION_UNINITIALIZED)
    {
        if (GetFlag(mConnStateFlags, kConnState_AutoClose))
        {
            ChipLogProgress(Ble, "Auto-closing end point's BLE connection.");
            mBle->mPlatformDelegate->CloseConnection(mConnObj);
        }
        else
        {
            ChipLogProgress(Ble, "Releasing end point's BLE connection back to application.");
            mBle->mApplicationDelegate->NotifyChipConnectionClosed(mConnObj);
        }

        // Never release the same BLE connection twice.
        mConnObj = BLE_CONNECTION_UNINITIALIZED;
    }
}

void BLEEndPoint::Free()
{
    // Release BLE connection. Will close connection if AutoClose enabled for this end point. Otherwise, informs
    // application that CHIP is done with this BLE connection, and application makes decision about whether to close
    // and clean up or retain connection.
    ReleaseBleConnection();

    // Clear fragmentation and reassembly engine's Tx and Rx buffers. Counters will be reset by next engine init.
    FreeBtpEngine();

    // Clear pending ack buffer, if any.
    PacketBuffer::Free(mAckToSend);

    // Cancel all timers.
    StopConnectTimer();
    StopReceiveConnectionTimer();
    StopAckReceivedTimer();
    StopSendAckTimer();
    StopUnsubscribeTimer();
#if CHIP_ENABLE_CHIPOBLE_TEST
    mBtpEngineTest.StopTestTimer();
    // Clear callback
    OnCommandReceived = NULL;
#endif

    // Clear callbacks.
    OnConnectComplete  = nullptr;
    OnMessageReceived  = nullptr;
    OnConnectionClosed = nullptr;

    // Clear handle to underlying BLE connection.
    mConnObj = BLE_CONNECTION_UNINITIALIZED;

    // Release the AddRef() that happened when the end point was allocated.
    Release();
}

void BLEEndPoint::FreeBtpEngine()
{
    PacketBuffer * buf;

    // Free transmit disassembly buffer
    buf = mBtpEngine.TxPacket();
    mBtpEngine.ClearTxPacket();
    PacketBuffer::Free(buf);

    // Free receive reassembly buffer
    buf = mBtpEngine.RxPacket();
    mBtpEngine.ClearRxPacket();
    PacketBuffer::Free(buf);
}

BLE_ERROR BLEEndPoint::Init(BleLayer * bleLayer, BLE_CONNECTION_OBJECT connObj, BleRole role, bool autoClose)
{
    BLE_ERROR err = BLE_NO_ERROR;
    bool expectInitialAck;

    // Fail if already initialized.
    VerifyOrExit(mBle == nullptr, err = BLE_ERROR_INCORRECT_STATE);

    // Validate args.
    VerifyOrExit(bleLayer != nullptr, err = BLE_ERROR_BAD_ARGS);
    VerifyOrExit(connObj != BLE_CONNECTION_UNINITIALIZED, err = BLE_ERROR_BAD_ARGS);
    VerifyOrExit((role == kBleRole_Central || role == kBleRole_Peripheral), err = BLE_ERROR_BAD_ARGS);

    // Null-initialize callbacks and data members.
    //
    // Beware this line should we ever use virtuals in this class or its
    // super(s). See similar lines in chip::System::Layer end points.
    memset((void *) this, 0, sizeof(*this));

    // If end point plays peripheral role, expect ack for indication sent as last step of BTP handshake.
    // If central, periperal's handshake indication 'ack's write sent by central to kick off the BTP handshake.
    expectInitialAck = (role == kBleRole_Peripheral);

    err = mBtpEngine.Init(this, expectInitialAck);
    if (err != BLE_NO_ERROR)
    {
        ChipLogError(Ble, "BtpEngine init failed");
        ExitNow();
    }

#if CHIP_ENABLE_CHIPOBLE_TEST
    err = (BLE_ERROR) mTxQueueMutex.Init(mTxQueueMutex);
    if (err != BLE_NO_ERROR)
    {
        ChipLogError(Ble, "%s: Mutex init failed", __FUNCTION__);
        ExitNow();
    }
    err = mBtpEngineTest.Init(this);
    if (err != BLE_NO_ERROR)
    {
        ChipLogError(Ble, "BTP test init failed");
        ExitNow();
    }
#endif

    // BleLayerObject initialization:
    mBle      = bleLayer;
    mRefCount = 1;

    // BLEEndPoint data members:
    mConnObj         = connObj;
    mRole            = role;
    mConnStateFlags  = 0;
    mTimerStateFlags = 0;
    SetFlag(mConnStateFlags, kConnState_AutoClose, autoClose);
    mLocalReceiveWindowSize  = 0;
    mRemoteReceiveWindowSize = 0;
    mReceiveWindowMaxSize    = 0;
    mSendQueue               = nullptr;
    mAckToSend               = nullptr;

    ChipLogDebugBleEndPoint(Ble, "initialized local rx window, size = %u", mLocalReceiveWindowSize);

    // End point is ready to connect or receive a connection.
    mState = kState_Ready;

exit:
    return err;
}

BLE_ERROR BLEEndPoint::SendCharacteristic(PacketBuffer * buf)
{
    BLE_ERROR err = BLE_NO_ERROR;

    if (mRole == kBleRole_Central)
    {
        if (!SendWrite(buf))
        {
            err = BLE_ERROR_GATT_WRITE_FAILED;
        }
        else
        {
            // Write succeeded, so shrink remote receive window counter by 1.
            mRemoteReceiveWindowSize -= 1;
            ChipLogDebugBleEndPoint(Ble, "decremented remote rx window, new size = %u", mRemoteReceiveWindowSize);
        }
    }
    else // (mRole == kBleRole_Peripheral), verified on Init
    {
        if (!SendIndication(buf))
        {
            err = BLE_ERROR_GATT_INDICATE_FAILED;
        }
        else
        {
            // Indication succeeded, so shrink remote receive window counter by 1.
            mRemoteReceiveWindowSize -= 1;
            ChipLogDebugBleEndPoint(Ble, "decremented remote rx window, new size = %u", mRemoteReceiveWindowSize);
        }
    }

    return err;
}

/*
 *  Routine to queue the Tx packet with a packet type
 *  kType_Data(0)       - data packet
 *  kType_Control(1)    - control packet
 */
void BLEEndPoint::QueueTx(PacketBuffer * data, PacketType_t type)
{
#if CHIP_ENABLE_CHIPOBLE_TEST
    ChipLogDebugBleEndPoint(Ble, "%s: data->%p, type %d, len %d", __FUNCTION__, data, type, data->DataLength());
    mBtpEngine.PushPacketTag(data, type);
#endif

    QueueTxLock();

    if (mSendQueue == nullptr)
    {
        mSendQueue = data;
        ChipLogDebugBleEndPoint(Ble, "%s: Set data as new mSendQueue %p, type %d", __FUNCTION__, mSendQueue, type);
    }
    else
    {
        mSendQueue->AddToEnd(data);
        ChipLogDebugBleEndPoint(Ble, "%s: Append data to mSendQueue %p, type %d", __FUNCTION__, mSendQueue, type);
    }

    QueueTxUnlock();
}

BLE_ERROR BLEEndPoint::Send(PacketBuffer * data)
{
    ChipLogDebugBleEndPoint(Ble, "entered Send");

    BLE_ERROR err = BLE_NO_ERROR;

    VerifyOrExit(data != nullptr, err = BLE_ERROR_BAD_ARGS);
    VerifyOrExit(IsConnected(mState), err = BLE_ERROR_INCORRECT_STATE);

    // Ensure outgoing message fits in a single contiguous PacketBuffer, as currently required by the
    // message fragmentation and reassembly engine.
    if (data->Next() != nullptr)
    {
        data->CompactHead();

        if (data->Next() != nullptr)
        {
            err = BLE_ERROR_OUTBOUND_MESSAGE_TOO_BIG;
            ExitNow();
        }
    }

    // Add new message to send queue.
    QueueTx(data, kType_Data);
    data = nullptr; // Buffer freed when send queue freed on close, or on completion of current message transmission.

    // Send first fragment of new message, if we can.
    err = DriveSending();
    SuccessOrExit(err);

exit:
    ChipLogDebugBleEndPoint(Ble, "exiting Send");

    if (data != nullptr)
    {
        PacketBuffer::Free(data);
    }

    if (err != BLE_NO_ERROR)
    {
        DoClose(kBleCloseFlag_AbortTransmission, err);
    }

    return err;
}

bool BLEEndPoint::PrepareNextFragment(PacketBuffer * data, bool & sentAck)
{
    // If we have a pending fragment acknowledgement to send, piggyback it on the fragment we're about to transmit.
    if (GetFlag(mTimerStateFlags, kTimerState_SendAckTimerRunning))
    {
        // Reset local receive window counter.
        mLocalReceiveWindowSize = mReceiveWindowMaxSize;
        ChipLogDebugBleEndPoint(Ble, "reset local rx window on piggyback ack tx, size = %u", mLocalReceiveWindowSize);

        // Tell caller AND fragmenter we have an ack to piggyback.
        sentAck = true;
    }
    else
    {
        // No ack to piggyback.
        sentAck = false;
    }

    return mBtpEngine.HandleCharacteristicSend(data, sentAck);
}

BLE_ERROR BLEEndPoint::SendNextMessage()
{
    BLE_ERROR err = BLE_NO_ERROR;
    bool sentAck;

    // Get the first queued packet to send
    QueueTxLock();
#if CHIP_ENABLE_CHIPOBLE_TEST
    // Return if tx queue is empty
    // Note: DetachTail() does not check an empty queue
    if (mSendQueue == NULL)
    {
        QueueTxUnlock();
        return err;
    }
#endif

    PacketBuffer * data = mSendQueue;
    mSendQueue          = mSendQueue->DetachTail();
    QueueTxUnlock();

#if CHIP_ENABLE_CHIPOBLE_TEST
    // Get and consume the packet tag in message buffer
    PacketType_t type = mBtpEngine.PopPacketTag(data);
    mBtpEngine.SetTxPacketType(type);
    mBtpEngineTest.DoTxTiming(data, BTP_TX_START);
#endif

    // Hand whole message payload to the fragmenter.
    VerifyOrExit(PrepareNextFragment(data, sentAck), err = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT);
    data = nullptr; // Ownership passed to fragmenter's tx buf on PrepareNextFragment success.

    /*
     // Todo: reenabled it after integrating fault injection
    // Send first message fragment over the air.
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_CHIPOBLESend, {
        if (mRole == kBleRole_Central)
        {
            err = BLE_ERROR_GATT_WRITE_FAILED;
        }
        else
        {
            err = BLE_ERROR_GATT_INDICATE_FAILED;
        }
        ExitNow();
    });
     */
    err = SendCharacteristic(mBtpEngine.TxPacket());
    SuccessOrExit(err);

    if (sentAck)
    {
        // If sent piggybacked ack, stop send-ack timer.
        StopSendAckTimer();
    }

    // Start ack received timer, if it's not already running.
    err = StartAckReceivedTimer();
    SuccessOrExit(err);

exit:
    if (data != nullptr)
    {
        PacketBuffer::Free(data);
    }

    return err;
}

BLE_ERROR BLEEndPoint::ContinueMessageSend()
{
    BLE_ERROR err;
    bool sentAck;

    if (!PrepareNextFragment(nullptr, sentAck))
    {
        // Log BTP error
        ChipLogError(Ble, "btp fragmenter error on send!");
        mBtpEngine.LogState();

        err = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        ExitNow();
    }

    err = SendCharacteristic(mBtpEngine.TxPacket());
    SuccessOrExit(err);

    if (sentAck)
    {
        // If sent piggybacked ack, stop send-ack timer.
        StopSendAckTimer();
    }

    // Start ack received timer, if it's not already running.
    err = StartAckReceivedTimer();
    SuccessOrExit(err);

exit:
    return err;
}

BLE_ERROR BLEEndPoint::HandleHandshakeConfirmationReceived()
{
    ChipLogDebugBleEndPoint(Ble, "entered HandleHandshakeConfirmationReceived");

    BLE_ERROR err      = BLE_NO_ERROR;
    uint8_t closeFlags = kBleCloseFlag_AbortTransmission;

    // Free capabilities request/response payload.
    QueueTxLock();
    mSendQueue = PacketBuffer::FreeHead(mSendQueue);
    QueueTxUnlock();

    if (mRole == kBleRole_Central)
    {
        // Subscribe to characteristic which peripheral will use to send indications. Prompts peripheral to send
        // BLE transport capabilities indication.
        VerifyOrExit(mBle->mPlatformDelegate->SubscribeCharacteristic(mConnObj, &CHIP_BLE_SVC_ID, &mBle->CHIP_BLE_CHAR_2_ID),
                     err = BLE_ERROR_GATT_SUBSCRIBE_FAILED);

        // We just sent a GATT subscribe request, so make sure to attempt unsubscribe on close.
        SetFlag(mConnStateFlags, kConnState_DidBeginSubscribe, true);

        // Mark GATT operation in progress for subscribe request.
        SetFlag(mConnStateFlags, kConnState_GattOperationInFlight, true);
    }
    else // (mRole == kBleRole_Peripheral), verified on Init
    {
        ChipLogDebugBleEndPoint(Ble, "got peripheral handshake indication confirmation");

        if (mState == kState_Connected) // If we accepted BTP connection...
        {
            // If local receive window size has shrunk to or below immediate ack threshold, AND a message fragment is not
            // pending on which to piggyback an ack, send immediate stand-alone ack.
            if (mLocalReceiveWindowSize <= BLE_CONFIG_IMMEDIATE_ACK_WINDOW_THRESHOLD && mSendQueue == nullptr)
            {
                err = DriveStandAloneAck(); // Encode stand-alone ack and drive sending.
                SuccessOrExit(err);
            }
            else
            {
                // Drive sending in case application callend Send() after we sent the handshake indication, but
                // before the GATT confirmation for this indication was received.
                err = DriveSending();
                SuccessOrExit(err);
            }
        }
        else if (mState == kState_Aborting) // Else, if we rejected BTP connection...
        {
            closeFlags |= kBleCloseFlag_SuppressCallback;
            err = BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS;
            ExitNow();
        }
    }

exit:
    ChipLogDebugBleEndPoint(Ble, "exiting HandleHandshakeConfirmationReceived");

    if (err != BLE_NO_ERROR)
    {
        DoClose(closeFlags, err);
    }

    return err;
}

BLE_ERROR BLEEndPoint::HandleFragmentConfirmationReceived()
{
    BLE_ERROR err = BLE_NO_ERROR;

    ChipLogDebugBleEndPoint(Ble, "entered HandleFragmentConfirmationReceived");

    // Suppress error logging if GATT confirmation overlaps with unsubscribe on final close.
    if (IsUnsubscribePending())
    {
        ChipLogDebugBleEndPoint(Ble, "send conf rx'd while unsubscribe in flight");
        ExitNow();
    }

    // Ensure we're in correct state to receive confirmation of non-handshake GATT send.
    VerifyOrExit(IsConnected(mState), err = BLE_ERROR_INCORRECT_STATE);

    // TODO PacketBuffer high water mark optimization: if ack pending, but fragmenter state == complete, free fragmenter's
    // tx buf before sending ack.

    if (GetFlag(mConnStateFlags, kConnState_StandAloneAckInFlight))
    {
        // If confirmation was received for stand-alone ack, free its tx buffer.
        PacketBuffer::Free(mAckToSend);
        mAckToSend = nullptr;

        SetFlag(mConnStateFlags, kConnState_StandAloneAckInFlight, false);
    }

    // If local receive window size has shrunk to or below immediate ack threshold, AND a message fragment is not
    // pending on which to piggyback an ack, send immediate stand-alone ack.
    //
    // This check covers the case where the local receive window has shrunk between transmission and confirmation of
    // the stand-alone ack, and also the case where a window size < the immediate ack threshold was detected in
    // Receive(), but the stand-alone ack was deferred due to a pending outbound message fragment.
    if (mLocalReceiveWindowSize <= BLE_CONFIG_IMMEDIATE_ACK_WINDOW_THRESHOLD &&
        !(mSendQueue != nullptr || mBtpEngine.TxState() == BtpEngine::kState_InProgress))
    {
        err = DriveStandAloneAck(); // Encode stand-alone ack and drive sending.
        SuccessOrExit(err);
    }
    else
    {
        err = DriveSending();
        SuccessOrExit(err);
    }

exit:
    if (err != BLE_NO_ERROR)
    {
        DoClose(kBleCloseFlag_AbortTransmission, err);
    }

    return err;
}

BLE_ERROR BLEEndPoint::HandleGattSendConfirmationReceived()
{
    ChipLogDebugBleEndPoint(Ble, "entered HandleGattSendConfirmationReceived");

    // Mark outstanding GATT operation as finished.
    SetFlag(mConnStateFlags, kConnState_GattOperationInFlight, false);

    // If confirmation was for outbound portion of BTP connect handshake...
    if (!GetFlag(mConnStateFlags, kConnState_CapabilitiesConfReceived))
    {
        SetFlag(mConnStateFlags, kConnState_CapabilitiesConfReceived, true);

        return HandleHandshakeConfirmationReceived();
    }

    return HandleFragmentConfirmationReceived();
}

BLE_ERROR BLEEndPoint::DriveStandAloneAck()
{
    BLE_ERROR err = BLE_NO_ERROR;

    // Stop send-ack timer if running.
    StopSendAckTimer();

    // If stand-alone ack not already pending, allocate new payload buffer here.
    if (mAckToSend == nullptr)
    {
        mAckToSend = PacketBuffer::New();
        VerifyOrExit(mAckToSend != nullptr, err = BLE_ERROR_NO_MEMORY);
    }

    // Attempt to send stand-alone ack.
    err = DriveSending();
    SuccessOrExit(err);

exit:
    return err;
}

BLE_ERROR BLEEndPoint::DoSendStandAloneAck()
{
    ChipLogDebugBleEndPoint(Ble, "entered DoSendStandAloneAck; sending stand-alone ack");

    // Encode and transmit stand-alone ack.
    mBtpEngine.EncodeStandAloneAck(mAckToSend);
    BLE_ERROR err = SendCharacteristic(mAckToSend);
    SuccessOrExit(err);

    // Reset local receive window counter.
    mLocalReceiveWindowSize = mReceiveWindowMaxSize;
    ChipLogDebugBleEndPoint(Ble, "reset local rx window on stand-alone ack tx, size = %u", mLocalReceiveWindowSize);

    SetFlag(mConnStateFlags, kConnState_StandAloneAckInFlight, true);

    // Start ack received timer, if it's not already running.
    err = StartAckReceivedTimer();
    SuccessOrExit(err);

exit:
    return err;
}

BLE_ERROR BLEEndPoint::DriveSending()
{
    BLE_ERROR err = BLE_NO_ERROR;

    ChipLogDebugBleEndPoint(Ble, "entered DriveSending");

    // If receiver's window is almost closed and we don't have an ack to send, OR we do have an ack to send but
    // receiver's window is completely empty, OR another GATT operation is in flight, awaiting confirmation...
    if ((mRemoteReceiveWindowSize <= BTP_WINDOW_NO_ACK_SEND_THRESHOLD &&
         !GetFlag(mTimerStateFlags, kTimerState_SendAckTimerRunning) && mAckToSend == nullptr) ||
        (mRemoteReceiveWindowSize == 0) || (GetFlag(mConnStateFlags, kConnState_GattOperationInFlight)))
    {
#ifdef CHIP_BLE_END_POINT_DEBUG_LOGGING_ENABLED
        if (mRemoteReceiveWindowSize <= BTP_WINDOW_NO_ACK_SEND_THRESHOLD &&
            !GetFlag(mTimerStateFlags, kTimerState_SendAckTimerRunning) && mAckToSend == NULL)
        {
            ChipLogDebugBleEndPoint(Ble, "NO SEND: receive window almost closed, and no ack to send");
        }

        if (mRemoteReceiveWindowSize == 0)
        {
            ChipLogDebugBleEndPoint(Ble, "NO SEND: remote receive window closed");
        }

        if (GetFlag(mConnStateFlags, kConnState_GattOperationInFlight))
        {
            ChipLogDebugBleEndPoint(Ble, "NO SEND: Gatt op in flight");
        }
#endif

        // Can't send anything.
        ExitNow();
    }

    // Otherwise, let's see what we can send.

    if (mAckToSend != nullptr) // If immediate, stand-alone ack is pending, send it.
    {
        err = DoSendStandAloneAck();
        SuccessOrExit(err);
    }
    else if (mBtpEngine.TxState() == BtpEngine::kState_Idle) // Else send next message fragment, if any.
    {
        // Fragmenter's idle, let's see what's in the send queue...
        if (mSendQueue != nullptr)
        {
            // Transmit first fragment of next whole message in send queue.
            err = SendNextMessage();
            SuccessOrExit(err);
        }
        else
        {
            // Nothing to send!
        }
    }
    else if (mBtpEngine.TxState() == BtpEngine::kState_InProgress)
    {
        // Send next fragment of message currently held by fragmenter.
        err = ContinueMessageSend();
        SuccessOrExit(err);
    }
    else if (mBtpEngine.TxState() == BtpEngine::kState_Complete)
    {
        // Clear fragmenter's pointer to sent message buffer and reset its Tx state.
        PacketBuffer * sentBuf = mBtpEngine.TxPacket();
#if CHIP_ENABLE_CHIPOBLE_TEST
        mBtpEngineTest.DoTxTiming(sentBuf, BTP_TX_DONE);
#endif // CHIP_ENABLE_CHIPOBLE_TEST
        mBtpEngine.ClearTxPacket();

        // Free sent buffer.
        PacketBuffer::Free(sentBuf);
        sentBuf = nullptr;

        if (mSendQueue != nullptr)
        {
            // Transmit first fragment of next whole message in send queue.
            err = SendNextMessage();
            SuccessOrExit(err);
        }
        else if (mState == kState_Closing && !mBtpEngine.ExpectingAck()) // and mSendQueue is NULL, per above...
        {
            // If end point closing, got last ack, and got out-of-order confirmation for last send, finalize close.
            FinalizeClose(mState, kBleCloseFlag_SuppressCallback, BLE_NO_ERROR);
        }
        else
        {
            // Nothing to send!
        }
    }

exit:
    return err;
}

BLE_ERROR BLEEndPoint::HandleCapabilitiesRequestReceived(PacketBuffer * data)
{
    BLE_ERROR err = BLE_NO_ERROR;
    BleTransportCapabilitiesRequestMessage req;
    BleTransportCapabilitiesResponseMessage resp;
    PacketBuffer * responseBuf = nullptr;
    uint16_t mtu;

    VerifyOrExit(data != nullptr, err = BLE_ERROR_BAD_ARGS);

    mState = kState_Connecting;

    // Decode BTP capabilities request.
    err = BleTransportCapabilitiesRequestMessage::Decode((*data), req);
    SuccessOrExit(err);

    responseBuf = PacketBuffer::New();
    VerifyOrExit(responseBuf != nullptr, err = BLE_ERROR_NO_MEMORY);

    // Determine BLE connection's negotiated ATT MTU, if possible.
    if (req.mMtu > 0) // If MTU was observed and provided by central...
    {
        mtu = req.mMtu; // Accept central's observation of the MTU.
    }
    else // Otherwise, retrieve it via the platform delegate...
    {
        mtu = mBle->mPlatformDelegate->GetMTU(mConnObj);
    }

    // Select fragment size for connection based on ATT MTU.
    if (mtu > 0) // If one or both device knows connection's MTU...
    {
        resp.mFragmentSize =
            chip::min(static_cast<uint16_t>(mtu - 3), BtpEngine::sMaxFragmentSize); // Reserve 3 bytes of MTU for ATT header.
    }
    else // Else, if neither device knows MTU...
    {
        ChipLogProgress(Ble, "cannot determine ATT MTU; selecting default fragment size = %u", BtpEngine::sDefaultFragmentSize);
        resp.mFragmentSize = BtpEngine::sDefaultFragmentSize;
    }

    // Select local and remote max receive window size based on local resources available for both incoming writes AND
    // GATT confirmations.
    mRemoteReceiveWindowSize = mLocalReceiveWindowSize = mReceiveWindowMaxSize =
        chip::min(req.mWindowSize, static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE));
    resp.mWindowSize = mReceiveWindowMaxSize;

    ChipLogProgress(Ble, "local and remote recv window sizes = %u", resp.mWindowSize);

    // Select BLE transport protocol version from those supported by central, or none if no supported version found.
    resp.mSelectedProtocolVersion = BleLayer::GetHighestSupportedProtocolVersion(req);
    ChipLogProgress(Ble, "selected BTP version %d", resp.mSelectedProtocolVersion);

    if (resp.mSelectedProtocolVersion == kBleTransportProtocolVersion_None)
    {
        // If BLE transport protocol versions incompatible, prepare to close connection after subscription has been
        // received and capabilities response has been sent.
        ChipLogError(Ble, "incompatible BTP versions; peripheral expected between %d and %d",
                     CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION, CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION);
        mState = kState_Aborting;
    }
    else if ((resp.mSelectedProtocolVersion == kBleTransportProtocolVersion_V1) ||
             (resp.mSelectedProtocolVersion == kBleTransportProtocolVersion_V2))
    {
        // Set Rx and Tx fragment sizes to the same value
        mBtpEngine.SetRxFragmentSize(resp.mFragmentSize);
        mBtpEngine.SetTxFragmentSize(resp.mFragmentSize);
    }
    else // resp.SelectedProtocolVersion >= kBleTransportProtocolVersion_V3
    {
        // This is the peripheral, so set Rx fragment size, and leave Tx at default
        mBtpEngine.SetRxFragmentSize(resp.mFragmentSize);
    }
    ChipLogProgress(Ble, "using BTP fragment sizes rx %d / tx %d.", mBtpEngine.GetRxFragmentSize(), mBtpEngine.GetTxFragmentSize());

    err = resp.Encode(responseBuf);
    SuccessOrExit(err);

    // Stash capabilities response payload and wait for subscription from central.
    QueueTx(responseBuf, kType_Data);
    responseBuf = nullptr;

    // Start receive timer. Canceled when end point freed or connection established.
    err = StartReceiveConnectionTimer();
    SuccessOrExit(err);

exit:
    if (responseBuf != nullptr)
    {
        PacketBuffer::Free(responseBuf);
    }

    if (data != nullptr)
    {
        PacketBuffer::Free(data);
    }

    return err;
}

BLE_ERROR BLEEndPoint::HandleCapabilitiesResponseReceived(PacketBuffer * data)
{
    BLE_ERROR err = BLE_NO_ERROR;
    BleTransportCapabilitiesResponseMessage resp;

    VerifyOrExit(data != nullptr, err = BLE_ERROR_BAD_ARGS);

    // Decode BTP capabilities response.
    err = BleTransportCapabilitiesResponseMessage::Decode((*data), resp);
    SuccessOrExit(err);

    VerifyOrExit(resp.mFragmentSize > 0, err = BLE_ERROR_INVALID_FRAGMENT_SIZE);

    ChipLogProgress(Ble, "peripheral chose BTP version %d; central expected between %d and %d", resp.mSelectedProtocolVersion,
                    CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION, CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION);

    if ((resp.mSelectedProtocolVersion < CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION) ||
        (resp.mSelectedProtocolVersion > CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION))
    {
        err = BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS;
        ExitNow();
    }

    // Set fragment size as minimum of (reported ATT MTU, BTP characteristic size)
    resp.mFragmentSize = chip::min(resp.mFragmentSize, BtpEngine::sMaxFragmentSize);

    if ((resp.mSelectedProtocolVersion == kBleTransportProtocolVersion_V1) ||
        (resp.mSelectedProtocolVersion == kBleTransportProtocolVersion_V2))
    {
        mBtpEngine.SetRxFragmentSize(resp.mFragmentSize);
        mBtpEngine.SetTxFragmentSize(resp.mFragmentSize);
    }
    else // resp.SelectedProtocolVersion >= kBleTransportProtocolVersion_V3
    {
        // This is the central, so set Tx fragement size, and leave Rx at default.
        mBtpEngine.SetTxFragmentSize(resp.mFragmentSize);
    }
    ChipLogProgress(Ble, "using BTP fragment sizes rx %d / tx %d.", mBtpEngine.GetRxFragmentSize(), mBtpEngine.GetTxFragmentSize());

    // Select local and remote max receive window size based on local resources available for both incoming indications
    // AND GATT confirmations.
    mRemoteReceiveWindowSize = mLocalReceiveWindowSize = mReceiveWindowMaxSize = resp.mWindowSize;

    ChipLogProgress(Ble, "local and remote recv window size = %u", resp.mWindowSize);

    // Shrink local receive window counter by 1, since connect handshake indication requires acknowledgement.
    mLocalReceiveWindowSize -= 1;
    ChipLogDebugBleEndPoint(Ble, "decremented local rx window, new size = %u", mLocalReceiveWindowSize);

    // Send ack for connection handshake indication when timer expires. Sequence numbers always start at 0,
    // and the reassembler's "last received seq num" is initialized to 0 and updated when new fragments are
    // received from the peripheral, so we don't need to explicitly mark the ack num to send here.
    err = StartSendAckTimer();
    SuccessOrExit(err);

    // We've sent a capabilities request write and received a compatible response, so the connect
    // operation has completed successfully.
    err = HandleConnectComplete();
    SuccessOrExit(err);

exit:
    if (data != nullptr)
    {
        PacketBuffer::Free(data);
    }

    return err;
}

// Returns number of open slots in remote receive window given the input values.
SequenceNumber_t BLEEndPoint::AdjustRemoteReceiveWindow(SequenceNumber_t lastReceivedAck, SequenceNumber_t maxRemoteWindowSize,
                                                        SequenceNumber_t newestUnackedSentSeqNum)
{
    // Assumption: SequenceNumber_t is uint8_t.
    // Assumption: Maximum possible sequence number value is UINT8_MAX.
    // Assumption: Sequence numbers incremented past maximum value wrap to 0.
    // Assumption: newest unacked sent sequence number never exceeds current (and by extension, new and un-wrapped)
    //             window boundary, so it never wraps relative to last received ack, if new window boundary would not
    //             also wrap.

    // Define new window boundary (inclusive) as uint16_t, so its value can temporarily exceed UINT8_MAX.
    uint16_t newRemoteWindowBoundary = lastReceivedAck + maxRemoteWindowSize;

    if (newRemoteWindowBoundary > UINT8_MAX && newestUnackedSentSeqNum < lastReceivedAck)
    {
        // New window boundary WOULD wrap, and latest unacked seq num already HAS wrapped, so add offset to difference.
        return (newRemoteWindowBoundary - (newestUnackedSentSeqNum + UINT8_MAX));
    }

    // Neither values would or have wrapped, OR new boundary WOULD wrap but latest unacked seq num does not, so no
    // offset required.
    return (newRemoteWindowBoundary - newestUnackedSentSeqNum);
}

BLE_ERROR BLEEndPoint::Receive(PacketBuffer * data)
{
    ChipLogDebugBleEndPoint(Ble, "+++++++++++++++++++++ entered receive");
    BLE_ERROR err                = BLE_NO_ERROR;
    SequenceNumber_t receivedAck = 0;
    uint8_t closeFlags           = kBleCloseFlag_AbortTransmission;
    bool didReceiveAck           = false;

#if CHIP_ENABLE_CHIPOBLE_TEST
    if (mBtpEngine.IsCommandPacket(data))
    {
        ChipLogDebugBleEndPoint(Ble, "%s: Received Control frame: Flags %x", __FUNCTION__, *(data->Start()));
    }
    else
#endif
    { // This is a special handling on the first CHIPoBLE data packet, the CapabilitiesRequest.
        // Suppress error logging if peer's send overlaps with our unsubscribe on final close.
        if (IsUnsubscribePending())
        {
            ChipLogDebugBleEndPoint(Ble, "characteristic rx'd while unsubscribe in flight");
            ExitNow();
        }

        // If we're receiving the first inbound packet of a BLE transport connection handshake...
        if (!GetFlag(mConnStateFlags, kConnState_CapabilitiesMsgReceived))
        {
            if (mRole == kBleRole_Central) // If we're a central receiving a capabilities response indication...
            {
                // Ensure end point's in the right state before continuing.
                VerifyOrExit(mState == kState_Connecting, err = BLE_ERROR_INCORRECT_STATE);
                SetFlag(mConnStateFlags, kConnState_CapabilitiesMsgReceived, true);

                err  = HandleCapabilitiesResponseReceived(data);
                data = nullptr;
                SuccessOrExit(err);
            }
            else // Or, a peripheral receiving a capabilities request write...
            {
                // Ensure end point's in the right state before continuing.
                VerifyOrExit(mState == kState_Ready, err = BLE_ERROR_INCORRECT_STATE);
                SetFlag(mConnStateFlags, kConnState_CapabilitiesMsgReceived, true);

                err  = HandleCapabilitiesRequestReceived(data);
                data = nullptr;

                if (err != BLE_NO_ERROR)
                {
                    // If an error occurred decoding and handling the capabilities request, release the BLE connection.
                    // Central's connect attempt will time out if peripheral's application decides to keep the BLE
                    // connection open, or fail immediately if the application closes the connection.
                    closeFlags = closeFlags | kBleCloseFlag_SuppressCallback;
                    ExitNow();
                }
            }

            // If received data was handshake packet, don't feed it to message reassembler.
            ExitNow();
        }
    } // End handling the CapabilitiesRequest

    ChipLogDebugBleEndPoint(Ble, "prepared to rx post-handshake btp packet");

    // We've received a post-handshake BTP packet.
    // Ensure end point's in the right state before continuing.
    if (!IsConnected(mState))
    {
        ChipLogError(Ble, "ep rx'd packet in bad state");
        err = BLE_ERROR_INCORRECT_STATE;

        ExitNow();
    }

    ChipLogDebugBleEndPoint(Ble, "BTP about to rx characteristic, state before:");
    mBtpEngine.LogStateDebug();

    // Pass received packet into BTP protocol engine.
    err  = mBtpEngine.HandleCharacteristicReceived(data, receivedAck, didReceiveAck);
    data = nullptr; // Buffer consumed by protocol engine; either freed or added to message reassembly area.

    ChipLogDebugBleEndPoint(Ble, "BTP rx'd characteristic, state after:");
    mBtpEngine.LogStateDebug();

    SuccessOrExit(err);

    // Protocol engine accepted the fragment, so shrink local receive window counter by 1.
    mLocalReceiveWindowSize -= 1;
    ChipLogDebugBleEndPoint(Ble, "decremented local rx window, new size = %u", mLocalReceiveWindowSize);

    // Respond to received ack, if any.
    if (didReceiveAck)
    {
        ChipLogDebugBleEndPoint(Ble, "got btp ack = %u", receivedAck);

        // If ack was rx'd for neweset unacked sent fragment, stop ack received timer.
        if (!mBtpEngine.ExpectingAck())
        {
            ChipLogDebugBleEndPoint(Ble, "got ack for last outstanding fragment");
            StopAckReceivedTimer();

            if (mState == kState_Closing && mSendQueue == nullptr && mBtpEngine.TxState() == BtpEngine::kState_Idle)
            {
                // If end point closing, got confirmation for last send, and waiting for last ack, finalize close.
                FinalizeClose(mState, kBleCloseFlag_SuppressCallback, BLE_NO_ERROR);
                ExitNow();
            }
        }
        else // Else there are still sent fragments for which acks are expected, so restart ack received timer.
        {
            ChipLogDebugBleEndPoint(Ble, "still expecting ack(s), restarting timer...");
            err = RestartAckReceivedTimer();
            SuccessOrExit(err);
        }

        ChipLogDebugBleEndPoint(Ble, "about to adjust remote rx window; got ack num = %u, newest unacked sent seq num = %u, \
                old window size = %u, max window size = %u",
                                receivedAck, mBtpEngine.GetNewestUnackedSentSequenceNumber(), mRemoteReceiveWindowSize,
                                mReceiveWindowMaxSize);

        // Open remote device's receive window according to sequence number it just acknowledged.
        mRemoteReceiveWindowSize =
            AdjustRemoteReceiveWindow(receivedAck, mReceiveWindowMaxSize, mBtpEngine.GetNewestUnackedSentSequenceNumber());

        ChipLogDebugBleEndPoint(Ble, "adjusted remote rx window, new size = %u", mRemoteReceiveWindowSize);

        // Restart message transmission if it was previously paused due to window exhaustion.
        err = DriveSending();
        SuccessOrExit(err);
    }

    // The previous DriveSending() might have generated a piggyback acknowledgement if there was
    // previously un-acked data.  Otherwise, prepare to send acknowledgement for newly received fragment.
    //
    // If local receive window is below immediate ack threshold, AND there is no previous stand-alone ack in
    // flight, AND there is no pending outbound message fragment on which the ack can and will be piggybacked,
    // send immediate stand-alone ack to reopen window for sender.
    //
    // The "GATT operation in flight" check below covers "pending outbound message fragment" by extension, as when
    // a message has been passed to the end point via Send(), its next outbound fragment must either be in flight
    // itself, or awaiting the completion of another in-flight GATT operation.
    //
    // If any GATT operation is in flight that is NOT a stand-alone ack, the window size will be checked against
    // this threshold again when the GATT operation is confirmed.
    if (mBtpEngine.HasUnackedData())
    {
        if (mLocalReceiveWindowSize <= BLE_CONFIG_IMMEDIATE_ACK_WINDOW_THRESHOLD &&
            !GetFlag(mConnStateFlags, kConnState_GattOperationInFlight))
        {
            ChipLogDebugBleEndPoint(Ble, "sending immediate ack");
            err = DriveStandAloneAck();
            SuccessOrExit(err);
        }
        else
        {
            ChipLogDebugBleEndPoint(Ble, "starting send-ack timer");

            // Send ack when timer expires.
            err = StartSendAckTimer();
            SuccessOrExit(err);
        }
    }

    // If we've reassembled a whole message...
    if (mBtpEngine.RxState() == BtpEngine::kState_Complete)
    {
        // Take ownership of message PacketBuffer
        PacketBuffer * full_packet = mBtpEngine.RxPacket();
        mBtpEngine.ClearRxPacket();

        ChipLogDebugBleEndPoint(Ble, "reassembled whole msg, len = %d", full_packet->DataLength());

#if CHIP_ENABLE_CHIPOBLE_TEST
        // If we have a control message received callback, and end point is not closing...
        if (mBtpEngine.RxPacketType() == kType_Control && OnCommandReceived && mState != kState_Closing)
        {
            ChipLogDebugBleEndPoint(Ble, "%s: calling OnCommandReceived, seq# %u, len = %u, type %u", __FUNCTION__, receivedAck,
                                    full_packet->DataLength(), mBtpEngine.RxPacketType());
            // Pass received control message up the stack.
            mBtpEngine.SetRxPacketSeq(receivedAck);
            OnCommandReceived(this, full_packet);
        }
        else
#endif
            // If we have a message received callback, and end point is not closing...
            if (OnMessageReceived && mState != kState_Closing)
        {
            // Pass received message up the stack.
            OnMessageReceived(this, full_packet);
        }
        else
        {
            // Free received message if there's no one to own it.
            PacketBuffer::Free(full_packet);
        }
    }

exit:
    if (data != nullptr)
    {
        PacketBuffer::Free(data);
    }

    if (err != BLE_NO_ERROR)
    {
        DoClose(closeFlags, err);
    }

    return err;
}

bool BLEEndPoint::SendWrite(PacketBuffer * buf)
{
    // Add reference to message fragment for duration of platform's GATT write attempt. CHIP retains partial
    // ownership of message fragment's PacketBuffer, since this is the same buffer as that of the whole message, just
    // with a fragmenter-modified payload offset and data length. Buffer must be decref'd (i.e. PacketBuffer::Free'd) by
    // platform when BLE GATT operation completes.
    buf->AddRef();

    SetFlag(mConnStateFlags, kConnState_GattOperationInFlight, true);

    return mBle->mPlatformDelegate->SendWriteRequest(mConnObj, &CHIP_BLE_SVC_ID, &mBle->CHIP_BLE_CHAR_1_ID, buf);
}

bool BLEEndPoint::SendIndication(PacketBuffer * buf)
{
    // Add reference to message fragment for duration of platform's GATT indication attempt. CHIP retains partial
    // ownership of message fragment's PacketBuffer, since this is the same buffer as that of the whole message, just
    // with a fragmenter-modified payload offset and data length. Buffer must be decref'd (i.e. PacketBuffer::Free'd) by
    // platform when BLE GATT operation completes.
    buf->AddRef();

    SetFlag(mConnStateFlags, kConnState_GattOperationInFlight, true);

    return mBle->mPlatformDelegate->SendIndication(mConnObj, &CHIP_BLE_SVC_ID, &mBle->CHIP_BLE_CHAR_2_ID, buf);
}

BLE_ERROR BLEEndPoint::StartConnectTimer()
{
    BLE_ERROR err = BLE_NO_ERROR;
    chip::System::Error timerErr;

    timerErr = mBle->mSystemLayer->StartTimer(BLE_CONNECT_TIMEOUT_MS, HandleConnectTimeout, this);
    VerifyOrExit(timerErr == CHIP_SYSTEM_NO_ERROR, err = BLE_ERROR_START_TIMER_FAILED);
    SetFlag(mTimerStateFlags, kTimerState_ConnectTimerRunning, true);

exit:
    return err;
}

BLE_ERROR BLEEndPoint::StartReceiveConnectionTimer()
{
    BLE_ERROR err = BLE_NO_ERROR;
    chip::System::Error timerErr;

    timerErr = mBle->mSystemLayer->StartTimer(BLE_CONNECT_TIMEOUT_MS, HandleReceiveConnectionTimeout, this);
    VerifyOrExit(timerErr == CHIP_SYSTEM_NO_ERROR, err = BLE_ERROR_START_TIMER_FAILED);
    SetFlag(mTimerStateFlags, kTimerState_ReceiveConnectionTimerRunning, true);

exit:
    return err;
}

BLE_ERROR BLEEndPoint::StartAckReceivedTimer()
{
    BLE_ERROR err = BLE_NO_ERROR;
    chip::System::Error timerErr;

    if (!GetFlag(mTimerStateFlags, kTimerState_AckReceivedTimerRunning))
    {
        timerErr = mBle->mSystemLayer->StartTimer(BTP_ACK_RECEIVED_TIMEOUT_MS, HandleAckReceivedTimeout, this);
        VerifyOrExit(timerErr == CHIP_SYSTEM_NO_ERROR, err = BLE_ERROR_START_TIMER_FAILED);

        SetFlag(mTimerStateFlags, kTimerState_AckReceivedTimerRunning, true);
    }

exit:
    return err;
}

BLE_ERROR BLEEndPoint::RestartAckReceivedTimer()
{
    BLE_ERROR err = BLE_NO_ERROR;

    VerifyOrExit(GetFlag(mTimerStateFlags, kTimerState_AckReceivedTimerRunning), err = BLE_ERROR_INCORRECT_STATE);

    StopAckReceivedTimer();

    err = StartAckReceivedTimer();
    SuccessOrExit(err);

exit:
    return err;
}

BLE_ERROR BLEEndPoint::StartSendAckTimer()
{
    BLE_ERROR err = BLE_NO_ERROR;
    chip::System::Error timerErr;

    ChipLogDebugBleEndPoint(Ble, "entered StartSendAckTimer");

    if (!GetFlag(mTimerStateFlags, kTimerState_SendAckTimerRunning))
    {
        ChipLogDebugBleEndPoint(Ble, "starting new SendAckTimer");
        timerErr = mBle->mSystemLayer->StartTimer(BTP_ACK_SEND_TIMEOUT_MS, HandleSendAckTimeout, this);
        VerifyOrExit(timerErr == CHIP_SYSTEM_NO_ERROR, err = BLE_ERROR_START_TIMER_FAILED);

        SetFlag(mTimerStateFlags, kTimerState_SendAckTimerRunning, true);
    }

exit:
    return err;
}

BLE_ERROR BLEEndPoint::StartUnsubscribeTimer()
{
    BLE_ERROR err = BLE_NO_ERROR;
    chip::System::Error timerErr;

    timerErr = mBle->mSystemLayer->StartTimer(BLE_UNSUBSCRIBE_TIMEOUT_MS, HandleUnsubscribeTimeout, this);
    VerifyOrExit(timerErr == CHIP_SYSTEM_NO_ERROR, err = BLE_ERROR_START_TIMER_FAILED);
    SetFlag(mTimerStateFlags, kTimerState_UnsubscribeTimerRunning, true);

exit:
    return err;
}

void BLEEndPoint::StopConnectTimer()
{
    // Cancel any existing connect timer.
    mBle->mSystemLayer->CancelTimer(HandleConnectTimeout, this);
    SetFlag(mTimerStateFlags, kTimerState_ConnectTimerRunning, false);
}

void BLEEndPoint::StopReceiveConnectionTimer()
{
    // Cancel any existing receive connection timer.
    mBle->mSystemLayer->CancelTimer(HandleReceiveConnectionTimeout, this);
    SetFlag(mTimerStateFlags, kTimerState_ReceiveConnectionTimerRunning, false);
}

void BLEEndPoint::StopAckReceivedTimer()
{
    // Cancel any existing ack-received timer.
    mBle->mSystemLayer->CancelTimer(HandleAckReceivedTimeout, this);
    SetFlag(mTimerStateFlags, kTimerState_AckReceivedTimerRunning, false);
}

void BLEEndPoint::StopSendAckTimer()
{
    // Cancel any existing send-ack timer.
    mBle->mSystemLayer->CancelTimer(HandleSendAckTimeout, this);
    SetFlag(mTimerStateFlags, kTimerState_SendAckTimerRunning, false);
}

void BLEEndPoint::StopUnsubscribeTimer()
{
    // Cancel any existing unsubscribe timer.
    mBle->mSystemLayer->CancelTimer(HandleUnsubscribeTimeout, this);
    SetFlag(mTimerStateFlags, kTimerState_UnsubscribeTimerRunning, false);
}

void BLEEndPoint::HandleConnectTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err)
{
    BLEEndPoint * ep = static_cast<BLEEndPoint *>(appState);

    // Check for event-based timer race condition.
    if (GetFlag(ep->mTimerStateFlags, kTimerState_ConnectTimerRunning))
    {
        ChipLogError(Ble, "connect handshake timed out, closing ep %p", ep);
        SetFlag(ep->mTimerStateFlags, kTimerState_ConnectTimerRunning, false);
        ep->DoClose(kBleCloseFlag_AbortTransmission, BLE_ERROR_CONNECT_TIMED_OUT);
    }
}

void BLEEndPoint::HandleReceiveConnectionTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err)
{
    BLEEndPoint * ep = static_cast<BLEEndPoint *>(appState);

    // Check for event-based timer race condition.
    if (GetFlag(ep->mTimerStateFlags, kTimerState_ReceiveConnectionTimerRunning))
    {
        ChipLogError(Ble, "receive handshake timed out, closing ep %p", ep);
        SetFlag(ep->mTimerStateFlags, kTimerState_ReceiveConnectionTimerRunning, false);
        ep->DoClose(kBleCloseFlag_SuppressCallback | kBleCloseFlag_AbortTransmission, BLE_ERROR_RECEIVE_TIMED_OUT);
    }
}

void BLEEndPoint::HandleAckReceivedTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err)
{
    BLEEndPoint * ep = static_cast<BLEEndPoint *>(appState);

    // Check for event-based timer race condition.
    if (GetFlag(ep->mTimerStateFlags, kTimerState_AckReceivedTimerRunning))
    {
        ChipLogError(Ble, "ack recv timeout, closing ep %p", ep);
        ep->mBtpEngine.LogStateDebug();
        SetFlag(ep->mTimerStateFlags, kTimerState_AckReceivedTimerRunning, false);
        ep->DoClose(kBleCloseFlag_AbortTransmission, BLE_ERROR_FRAGMENT_ACK_TIMED_OUT);
    }
}

void BLEEndPoint::HandleSendAckTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err)
{
    BLEEndPoint * ep = static_cast<BLEEndPoint *>(appState);

    // Check for event-based timer race condition.
    if (GetFlag(ep->mTimerStateFlags, kTimerState_SendAckTimerRunning))
    {
        SetFlag(ep->mTimerStateFlags, kTimerState_SendAckTimerRunning, false);

        // If previous stand-alone ack isn't still in flight...
        if (!GetFlag(ep->mConnStateFlags, kConnState_StandAloneAckInFlight))
        {
            BLE_ERROR sendErr = ep->DriveStandAloneAck();

            if (sendErr != BLE_NO_ERROR)
            {
                ep->DoClose(kBleCloseFlag_AbortTransmission, sendErr);
            }
        }
    }
}

void BLEEndPoint::HandleUnsubscribeTimeout(chip::System::Layer * systemLayer, void * appState, chip::System::Error err)
{
    BLEEndPoint * ep = static_cast<BLEEndPoint *>(appState);

    // Check for event-based timer race condition.
    if (GetFlag(ep->mTimerStateFlags, kTimerState_UnsubscribeTimerRunning))
    {
        ChipLogError(Ble, "unsubscribe timed out, ble ep %p", ep);
        SetFlag(ep->mTimerStateFlags, kTimerState_UnsubscribeTimerRunning, false);
        ep->HandleUnsubscribeComplete();
    }
}

} /* namespace Ble */
} /* namespace chip */

#endif /* CONFIG_NETWORK_LAYER_BLE */
