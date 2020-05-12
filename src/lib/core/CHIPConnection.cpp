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
 *      This file implements the CHIP Connection object that maintains a UDP connection.
 *      TODO This class should be extended to support TCP as well...
 *
 */

#include <core/CHIPConnection.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <inttypes.h>

namespace chip {

void ChipConnection::Init(Inet::InetLayer * inetLayer)
{
    mInetLayer        = inetLayer;
    mPeerNodeId       = 0;
    mPeerAddr         = IPAddress::Any;
    mPeerPort         = 0;
    mState            = ChipConnection::kState_ReadyToConnect;
    mUDPEndPoint      = NULL;
    mRefCount         = 1;
    OnMessageReceived = NULL;
    OnReceiveError    = NULL;
}

CHIP_ERROR ChipConnection::Connect(uint64_t peerNodeId, const IPAddress & peerAddr, uint16_t peerPort)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_ReadyToConnect, err = CHIP_ERROR_INCORRECT_STATE);

    mPeerNodeId = peerNodeId;
    mPeerAddr   = peerAddr;
    mPeerPort   = (peerPort != 0) ? peerPort : CHIP_PORT;

    // Bump the reference count when we start the connection process. The corresponding decrement happens when the
    // DoClose() method is called. This ensures the object stays live while there's the possibility of a callback
    // happening from an underlying layer.
    mRefCount++;

    ChipLogProgress(Inet, "Connection start %016llX", peerNodeId);
    err = DoConnect();
exit:
    return err;
}

CHIP_ERROR ChipConnection::DoConnect()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mInetLayer->NewUDPEndPoint(&mUDPEndPoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Error: %s\n Couldn't create connection\n", ErrorStr(err));
        return err;
    }

    err = mUDPEndPoint->Bind(mPeerAddr.Type(), IPAddress::Any, CHIP_PORT);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Error: %s\n Bind failed\n", ErrorStr(err));
        return err;
    }

    err = mUDPEndPoint->Listen();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Error: %s\n Listen failed\n", ErrorStr(err));
        return err;
    }

    mUDPEndPoint->AppState          = this;
    mUDPEndPoint->OnMessageReceived = HandleDataReceived;
    mUDPEndPoint->OnReceiveError    = HandleReceiveError;

#if CHIP_PROGRESS_LOGGING
    {
        char ipAddrStr[64];
        mPeerAddr.ToString(ipAddrStr, sizeof(ipAddrStr));
        ChipLogProgress(Inet, "Connection started %s %d", ipAddrStr, (int) mPeerPort);
    }
#endif
    mState = kState_Connected;

    return err;
}

CHIP_ERROR ChipConnection::SendMessage(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!StateAllowsSend())
    {
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

    // UDP EndPoint's SendMsg decrements the ref count
    msgBuf->AddRef();

    IPPacketInfo addrInfo;
    addrInfo.Clear();
    addrInfo.DestAddress = mPeerAddr;
    addrInfo.DestPort    = mPeerPort;

    err = mUDPEndPoint->SendMsg(&addrInfo, msgBuf);

exit:
    if (msgBuf != NULL)
    {
        PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

void ChipConnection::HandleDataReceived(IPEndPointBasis * endPoint, chip::System::PacketBuffer * msg, const IPPacketInfo * pktInfo)
{
    UDPEndPoint * udpEndPoint   = static_cast<UDPEndPoint *>(endPoint);
    ChipConnection * connection = (ChipConnection *) udpEndPoint->AppState;

    // TODO this where where messages should be decoded
    if (connection->StateAllowsReceive() && msg != NULL)
    {
        connection->OnMessageReceived(connection, msg, pktInfo);
    }
}

void ChipConnection::HandleReceiveError(IPEndPointBasis * endPoint, CHIP_ERROR err, const IPPacketInfo * pktInfo)
{
    UDPEndPoint * udpEndPoint   = static_cast<UDPEndPoint *>(endPoint);
    ChipConnection * connection = (ChipConnection *) udpEndPoint->AppState;
    if (connection->StateAllowsReceive())
    {
        connection->OnReceiveError(connection, err, pktInfo);
    }
}
/**
 *  Performs a non-blocking graceful close of the UDP based ChipConnection, delivering any
 *  remaining outgoing data before resetting the connection.
 *
 *  This method provides no strong guarantee that any outgoing message not acknowledged at the application
 *  protocol level has been received by the remote peer.
 *
 *  Once Close() has been called, the ChipConnection object can no longer be used for further communication.
 *
 *  Calling Close() decrements the reference count associated with the ChipConnection object, whether or not
 *  the connection is open/active at the time the method is called.  If this results in the reference count
 *  reaching zero, the resources associated with the connection object are freed.  When this happens, the
 *  application must have no further interactions with the object.
 *
 *  @sa Shutdown(), Abort(), Retain() and Release().
 *
 *  @return #CHIP_NO_ERROR unconditionally.
 *
 */
CHIP_ERROR ChipConnection::Close()
{
    // Perform a graceful close.
    DoClose(CHIP_NO_ERROR);

    // Decrement the ref count that was added when the ChipConnection object
    // was allocated.
    VerifyOrDie(mRefCount != 0);
    mRefCount--;

    return CHIP_NO_ERROR;
}

void ChipConnection::DoClose(CHIP_ERROR err)
{
    if (mState != kState_Closed)
    {
        if (mUDPEndPoint != NULL)
        {
            if (err == CHIP_NO_ERROR)
            {
                mUDPEndPoint->Close();
            }
            mUDPEndPoint->Free();
            mUDPEndPoint = NULL;
        }
    }
    uint8_t oldState = mState;
    mState           = kState_Closed;
    ChipLogProgress(Inet, "Connection closed %ld", (long) err);

    // Decrement the ref count that was added when the connection started.
    if (oldState != kState_ReadyToConnect && oldState != kState_Closed)
    {
        VerifyOrDie(mRefCount != 0);
        mRefCount--;
    }
}

/**
 * Reserve a reference to the ChipConnection object.
 *
 * The Retain() method increments the reference count associated with the ChipConnection object.  For every
 * call to Retain(), the application is responsible for making a corresponding call to either Release(), Close()
 * or Abort().
 */
void ChipConnection::Retain()
{
    VerifyOrDie(mRefCount < UINT8_MAX);
    ++mRefCount;
}

/**
 *  Decrement the reference count on the ChipConnection object.
 *
 *  The Release() method decrements the reference count associated with the ChipConnection object.  If
 *  this results in the reference count reaching zero, the connection is closed and the connection object
 *  is freed.  When this happens, the application must have no further interactions with the object.
 */
void ChipConnection::Release()
{
    // If the only reference that will remain after this call is the one that was automatically added
    // when the connection started, close the connection.
    if (mRefCount == 2 && mState != kState_ReadyToConnect && mState != kState_Closed)
    {
        DoClose(CHIP_NO_ERROR);
    }

    VerifyOrDie(mRefCount != 0);
    mRefCount--;
}

} // namespace chip