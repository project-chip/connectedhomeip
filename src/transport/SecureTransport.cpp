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

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/SecureTransport.h>

#include <inttypes.h>

namespace chip {

// Maximum length of application data that can be encrypted as one block.
// The limit is derived from IPv6 MTU (1280 bytes) - expected header overheads.
// This limit would need additional reviews once we have formalized Secure Transport header. 
const size_t kMax_SecureSDU_Length = 1024;

SecureTransport::SecureTransport() : mState(kState_NotReady), mRefCount(1)
{
    mUDPEndPoint      = NULL;
    OnMessageReceived = NULL;
    OnReceiveError    = NULL;
}

void SecureTransport::Init(Inet::InetLayer * inetLayer)
{
    if (mState != kState_NotReady)
    {
        return;
    }

    mInetLayer = inetLayer;
    mState     = kState_ReadyToConnect;

} // namespace chip

CHIP_ERROR SecureTransport::ManualKeyExchange(const unsigned char * remote_public_key, const size_t public_key_length,
                                              const unsigned char * local_private_key, const size_t private_key_length)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Connected, err = CHIP_ERROR_INCORRECT_STATE);

    err = mSecureChannel.Init(remote_public_key, public_key_length, local_private_key, private_key_length,
                              (const unsigned char *) "", 0, (const unsigned char *) "", 0);
    SuccessOrExit(err);
    mState = kState_Connected;
exit:
    return err;
}

CHIP_ERROR SecureTransport::Connect(IPAddressType addrType, InterfaceId intfId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_ReadyToConnect, err = CHIP_ERROR_INCORRECT_STATE);

    // Bump the reference count when we start the connection process. The corresponding decrement happens when the
    // DoClose() method is called. This ensures the object stays live while there's the possibility of a callback
    // happening from an underlying layer.
    mRefCount++;

    ChipLogProgress(Inet, "Connection start");

    err = mInetLayer->NewUDPEndPoint(&mUDPEndPoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Error: %s\n Couldn't create connection\n", ErrorStr(err));
        SuccessOrExit(err);
    }

    err = mUDPEndPoint->Bind(addrType, IPAddress::Any, CHIP_PORT, intfId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Error: %s\n Bind failed\n", ErrorStr(err));
        SuccessOrExit(err);
    }

    err = mUDPEndPoint->Listen();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Inet, "Error: %s\n Listen failed\n", ErrorStr(err));
        SuccessOrExit(err);
    }

    mUDPEndPoint->AppState          = this;
    mUDPEndPoint->OnMessageReceived = HandleDataReceived;
    mUDPEndPoint->OnReceiveError    = HandleReceiveError;
    mState                          = kState_Connected;
exit:
    return err;
}

CHIP_ERROR SecureTransport::SendMessage(PacketBuffer * msgBuf, const IPAddress & peerAddr, uint32_t msg_id)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(StateAllowsSend(), err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(msgBuf != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == NULL, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    IPPacketInfo addrInfo;
    addrInfo.Clear();
    addrInfo.DestAddress = peerAddr;
    addrInfo.DestPort    = CHIP_PORT;

    {
        uint8_t * plain_text    = msgBuf->Start();
        uint16_t plain_text_len = msgBuf->DataLength();

        uint8_t * encrypted_text = plain_text - CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;
        uint16_t encrypted_len   = plain_text_len + CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;

        err = mSecureChannel.Encrypt(plain_text, plain_text_len, encrypted_text, encrypted_len);
        SuccessOrExit(err);

        msgBuf->SetStart(encrypted_text);
        msgBuf->SetDataLength(encrypted_len, msgBuf);

        ChipLogProgress(Inet, "Secure transport transmitting msg %d after encryption", msg_id);
    }

    err    = mUDPEndPoint->SendMsg(&addrInfo, msgBuf);
    msgBuf = NULL;

exit:
    if (msgBuf != NULL)
    {
        ChipLogProgress(Inet, "Secure transport failed to encrypt msg %d", msg_id);
        PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

void SecureTransport::HandleDataReceived(IPEndPointBasis * endPoint, chip::System::PacketBuffer * msg, const IPPacketInfo * pktInfo)
{
    UDPEndPoint * udpEndPoint    = static_cast<UDPEndPoint *>(endPoint);
    SecureTransport * connection = (SecureTransport *) udpEndPoint->AppState;

    // TODO this is where messages should be decoded
    if (connection->StateAllowsReceive() && msg != NULL)
    {
        uint8_t * encrypted_text = msg->Start();
        uint16_t encrypted_len   = msg->DataLength();

        uint8_t * plain_text  = encrypted_text + CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;
        size_t plain_text_len = encrypted_len - CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;

        CHIP_ERROR err = connection->mSecureChannel.Decrypt(encrypted_text, encrypted_len, plain_text, plain_text_len);

        if (err == CHIP_NO_ERROR)
        {
            msg->SetStart(plain_text);
            msg->SetDataLength(plain_text_len, msg);

            connection->OnMessageReceived(connection, msg, pktInfo);
        }
    }
}

void SecureTransport::HandleReceiveError(IPEndPointBasis * endPoint, CHIP_ERROR err, const IPPacketInfo * pktInfo)
{
    UDPEndPoint * udpEndPoint    = static_cast<UDPEndPoint *>(endPoint);
    SecureTransport * connection = (SecureTransport *) udpEndPoint->AppState;
    if (connection->StateAllowsReceive())
    {
        connection->OnReceiveError(connection, err, pktInfo);
    }
}
/**
 *  Performs a non-blocking graceful close of the UDP based SecureTransport, delivering any
 *  remaining outgoing data before resetting the connection.
 *
 *  This method provides no strong guarantee that any outgoing message not acknowledged at the application
 *  protocol level has been received by the remote peer.
 *
 *  Once Close() has been called, the SecureTransport object can no longer be used for further communication.
 *
 *  Calling Close() decrements the reference count associated with the SecureTransport object, whether or not
 *  the connection is open/active at the time the method is called.  If this results in the reference count
 *  reaching zero, the resources associated with the connection object are freed.  When this happens, the
 *  application must have no further interactions with the object.
 *
 *  @sa Shutdown(), Abort(), Retain() and Release().
 *
 *  @return #CHIP_NO_ERROR unconditionally.
 *
 */
CHIP_ERROR SecureTransport::Close()
{
    // Perform a graceful close.
    DoClose(CHIP_NO_ERROR);

    // Decrement the ref count that was added when the SecureTransport object
    // was allocated.
    VerifyOrDie(mRefCount != 0);
    mRefCount--;

    return CHIP_NO_ERROR;
}

void SecureTransport::DoClose(CHIP_ERROR err)
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
 * Reserve a reference to the SecureTransport object.
 *
 * The Retain() method increments the reference count associated with the SecureTransport object.  For every
 * call to Retain(), the application is responsible for making a corresponding call to either Release(), Close()
 * or Abort().
 */
void SecureTransport::Retain()
{
    VerifyOrDie(mRefCount < UINT8_MAX);
    ++mRefCount;
}

/**
 *  Decrement the reference count on the SecureTransport object.
 *
 *  The Release() method decrements the reference count associated with the SecureTransport object.  If
 *  this results in the reference count reaching zero, the connection is closed and the connection object
 *  is freed.  When this happens, the application must have no further interactions with the object.
 */
void SecureTransport::Release()
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
