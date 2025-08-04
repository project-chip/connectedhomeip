/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 * This file implements Inet::TCPEndPoint using LwIP.
 */

#include <inet/TCPEndPointImplLwIP.h>

#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <stdio.h>
#include <string.h>
#include <utility>

#include <lwip/tcp.h>
#include <lwip/tcpip.h>

static_assert(LWIP_VERSION_MAJOR > 1, "CHIP requires LwIP 2.0 or later");

namespace chip {
namespace Inet {

namespace {

/*
 * This logic to register a null operation callback with the LwIP TCP/IP task
 * ensures that the TCP timer loop is started when a connection is established,
 * which is necessary to ensure that initial SYN and SYN-ACK packets are
 * retransmitted during the 3-way handshake.
 */

void nil_tcpip_callback(void * _aContext) {}

err_t start_tcp_timers(void)
{
    return tcpip_callback(nil_tcpip_callback, nullptr);
}

} // anonymous namespace

CHIP_ERROR TCPEndPointImplLwIP::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr)
{
    // Get the appropriate type of PCB.
    CHIP_ERROR res = GetPCB(addrType);

    // Bind the PCB to the specified address/port.
    ip_addr_t ipAddr;
    if (res == CHIP_NO_ERROR)
    {
        if (reuseAddr)
        {
            RunOnTCPIP([this]() { ip_set_option(mTCP, SOF_REUSEADDR); });
        }
        res = addr.ToLwIPAddr(addrType, ipAddr);
    }

    if (res == CHIP_NO_ERROR)
    {
        res = chip::System::MapErrorLwIP(RunOnTCPIPRet([this, &ipAddr, port]() { return tcp_bind(mTCP, &ipAddr, port); }));
    }

    return res;
}

CHIP_ERROR TCPEndPointImplLwIP::ListenImpl(uint16_t backlog)
{
    mLwIPEndPointType = LwIPEndPointType::TCP;
    CHIP_ERROR err    = CHIP_NO_ERROR;
    if (!mPreAllocatedConnectEP)
    {
        // Pre allocate a TCP EndPoint for TCP connection, it will be released under either of the two conditions:
        // - The Listen EndPoint receives a connection and the connection will use this endpoint. The endpoint will be release when
        // the connection is released.
        // - The Listen Endpoint is closed.
        err = GetEndPointManager().NewEndPoint(&mPreAllocatedConnectEP);
    }
    if (err == CHIP_NO_ERROR)
    {
        RunOnTCPIP([this]() {
            // Start listening for incoming connections.
            mTCP = tcp_listen(mTCP);
            tcp_arg(mTCP, this);
            tcp_accept(mTCP, LwIPHandleIncomingConnection);
        });
    }
    return err;
}

CHIP_ERROR TCPEndPointImplLwIP::ConnectImpl(const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    CHIP_ERROR res         = CHIP_NO_ERROR;
    IPAddressType addrType = addr.Type();

    // LwIP does not provides an API for initiating a TCP connection via a specific interface.
    // As a work-around, if the destination is an IPv6 link-local address, we bind the PCB
    // to the link local address associated with the source interface; however this is only
    // viable if the endpoint hasn't already been bound.
    if (intfId.IsPresent())
    {
        IPAddress intfLLAddr;

        if (!addr.IsIPv6LinkLocal() || mState == State::kBound)
            return CHIP_ERROR_NOT_IMPLEMENTED;

        res = intfId.GetLinkLocalAddr(&intfLLAddr);
        if (res != CHIP_NO_ERROR)
            return res;

        res = Bind(IPAddressType::kIPv6, intfLLAddr, 0, true);
        if (res != CHIP_NO_ERROR)
            return res;
    }

    res = GetPCB(addrType);

    if (res == CHIP_NO_ERROR)
    {
        ip_addr_t lwipAddr = addr.ToLwIPAddr();
        res                = chip::System::MapErrorLwIP(RunOnTCPIPRet([this, &lwipAddr, port]() {
            tcp_arg(mTCP, this);
            tcp_err(mTCP, LwIPHandleError);
            return tcp_connect(mTCP, &lwipAddr, port, LwIPHandleConnectComplete);
        }));

        // Ensure that TCP timers are started
        if (res == CHIP_NO_ERROR)
        {
            err_t error = start_tcp_timers();
            if (error != ERR_OK)
            {
                res = chip::System::MapErrorLwIP(error);
            }
        }

        if (res == CHIP_NO_ERROR)
        {
            mState = State::kConnecting;
            Retain();
        }
    }

    return res;
}

CHIP_ERROR TCPEndPointImplLwIP::GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    if (mTCP != nullptr)
    {
        RunOnTCPIP([this, &retAddr, &retPort]() {
            *retPort = mTCP->remote_port;
            *retAddr = IPAddress(mTCP->remote_ip);
        });
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CONNECTION_ABORTED;
}

CHIP_ERROR TCPEndPointImplLwIP::GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    if (mTCP != nullptr)
    {
        RunOnTCPIP([this, &retAddr, &retPort]() {
            *retPort = mTCP->local_port;
            *retAddr = IPAddress(mTCP->local_ip);
        });
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CONNECTION_ABORTED;
}

CHIP_ERROR TCPEndPointImplLwIP::GetInterfaceId(InterfaceId * retInterface)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // TODO: Does netif_get_by_index(mTCP->netif_idx) do the right thing?  I
    // can't quite tell whether LwIP supports a specific interface id for TCP at
    // all.  For now just claim no particular interface id.
    *retInterface = InterfaceId::Null();
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplLwIP::SendQueuedImpl(bool queueWasEmpty)
{
#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    if (!mUserTimeoutTimerRunning)
    {
        // Timer was not running before this send. So, start
        // the timer.
        StartTCPUserTimeoutTimer();
    }
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplLwIP::EnableNoDelay()
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);
    if (mTCP != nullptr)
    {
        RunOnTCPIP([this]() { tcp_nagle_disable(mTCP); });
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CONNECTION_ABORTED;
}

CHIP_ERROR TCPEndPointImplLwIP::EnableKeepAlive(uint16_t interval, uint16_t timeoutCount)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_ERROR_NOT_IMPLEMENTED;

#if LWIP_TCP_KEEPALIVE
    if (mTCP != nullptr)
    {
        RunOnTCPIP([this, interval, timeoutCount]() {
            // Set the idle interval
            mTCP->keep_idle = (uint32_t) interval * 1000;

            // Set the probe retransmission interval.
            mTCP->keep_intvl = (uint32_t) interval * 1000;

            // Set the probe timeout count
            mTCP->keep_cnt = timeoutCount;

            // Enable keepalives for the connection.
            ip_set_option(mTCP, SOF_KEEPALIVE);
        });
        res = CHIP_NO_ERROR;
    }
    else
    {
        res = CHIP_ERROR_CONNECTION_ABORTED;
    }
#endif // LWIP_TCP_KEEPALIVE

    return res;
}

CHIP_ERROR TCPEndPointImplLwIP::DisableKeepAlive()
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_ERROR_NOT_IMPLEMENTED;

#if LWIP_TCP_KEEPALIVE
    if (mTCP != nullptr)
    {
        // Disable keepalives on the connection.
        RunOnTCPIP([this]() { ip_reset_option(mTCP, SOF_KEEPALIVE); });
        res = CHIP_NO_ERROR;
    }
    else
    {
        res = CHIP_ERROR_CONNECTION_ABORTED;
    }
#endif // LWIP_TCP_KEEPALIVE

    return res;
}

CHIP_ERROR TCPEndPointImplLwIP::SetUserTimeoutImpl(uint32_t userTimeoutMillis)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR TCPEndPointImplLwIP::DriveSendingImpl()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // If the connection hasn't been aborted ...
    if (mTCP != nullptr)
    {
        err_t lwipErr;

        // Determine the current send window size. This is the maximum amount we can write to the connection.
        uint16_t sendWindowSize;
        RunOnTCPIP([this, &sendWindowSize]() { sendWindowSize = tcp_sndbuf(mTCP); });

        // If there's data to be sent and the send window is open...
        bool canSend = (RemainingToSend() > 0 && sendWindowSize > 0);
        if (canSend)
        {
            // Find first packet buffer with remaining data to send by skipping
            // all sent but un-acked data.
            TCPEndPointImplLwIP::BufferOffset startOfUnsent = FindStartOfUnsent();

            // While there's data to be sent and a window to send it in...
            do
            {
                VerifyOrDie(!startOfUnsent.buffer.IsNull());
                VerifyOrDie(CanCastTo<uint16_t>(startOfUnsent.buffer->DataLength()));
                uint16_t bufDataLen = static_cast<uint16_t>(startOfUnsent.buffer->DataLength());

                // Get a pointer to the start of unsent data within the first buffer on the unsent queue.
                const uint8_t * sendData = startOfUnsent.buffer->Start() + startOfUnsent.offset;

                // Determine the amount of data to send from the current buffer.
                uint16_t sendLen = static_cast<uint16_t>(bufDataLen - startOfUnsent.offset);
                if (sendLen > sendWindowSize)
                    sendLen = sendWindowSize;

                // Call LwIP to queue the data to be sent, telling it if there's more data to come.
                // Data is queued in-place as a reference within the source packet buffer. It is
                // critical that the underlying packet buffer not be freed until the data
                // is acknowledged, otherwise retransmissions could use an invalid
                // backing. Using TCP_WRITE_FLAG_COPY would eliminate this requirement, but overall
                // requires many more memory allocations which may be problematic when very
                // memory-constrained or when using pool-based allocations.
                lwipErr = RunOnTCPIPRet([this, sendData, sendLen, canSend]() {
                    return tcp_write(mTCP, sendData, sendLen, (canSend) ? TCP_WRITE_FLAG_MORE : 0);
                });
                if (lwipErr != ERR_OK)
                {
                    err = chip::System::MapErrorLwIP(lwipErr);
                    break;
                }
                // Start accounting for the data sent as yet-to-be-acked.
                // This cast is safe, because mUnackedLength + sendLen <= bufDataLen, which fits in uint16_t.
                mUnackedLength = static_cast<uint16_t>(mUnackedLength + sendLen);

                // Adjust the unsent data offset by the length of data that was written.
                // If the entire buffer has been sent advance to the next one.
                // This cast is safe, because startOfUnsent.offset + sendLen <= bufDataLen, which fits in uint16_t.
                startOfUnsent.offset = static_cast<uint16_t>(startOfUnsent.offset + sendLen);
                if (startOfUnsent.offset == bufDataLen)
                {
                    startOfUnsent.buffer.Advance();
                    startOfUnsent.offset = 0;
                }

                // Adjust the remaining window size.
                sendWindowSize = static_cast<uint16_t>(sendWindowSize - sendLen);

                // Determine if there's more data to be sent after this buffer.
                canSend = (RemainingToSend() > 0 && sendWindowSize > 0);
            } while (canSend);

            // Call LwIP to send the queued data.
            INET_FAULT_INJECT(FaultInjection::kFault_Send, err = chip::System::MapErrorLwIP(ERR_RTE));

            if (err == CHIP_NO_ERROR)
            {
                lwipErr = RunOnTCPIPRet([this]() { return tcp_output(mTCP); });

                if (lwipErr != ERR_OK)
                    err = chip::System::MapErrorLwIP(lwipErr);
            }
        }

        if (err == CHIP_NO_ERROR)
        {
            // If in the SendShutdown state and the unsent queue is now empty, shutdown the PCB for sending.
            if (mState == State::kSendShutdown && (RemainingToSend() == 0))
            {
                lwipErr = RunOnTCPIPRet([this]() { return tcp_shutdown(mTCP, 0, 1); });
                if (lwipErr != ERR_OK)
                    err = chip::System::MapErrorLwIP(lwipErr);
            }
        }
    }
    else
    {
        err = CHIP_ERROR_CONNECTION_ABORTED;
    }
    return err;
}

void TCPEndPointImplLwIP::HandleConnectCompleteImpl() {}

void TCPEndPointImplLwIP::DoCloseImpl(CHIP_ERROR err, State oldState)
{
    // If the LwIP PCB hasn't been closed yet...
    if (mTCP != nullptr)
    {
        // If the endpoint was a connection endpoint (vs. a listening endpoint)...
        if (oldState != State::kListening)
        {
            // Prevent further callbacks for incoming data.  This has the effect of instructing
            // LwIP to discard any further data received from the peer.
            RunOnTCPIP([this]() { tcp_recv(mTCP, NULL); });

            // If entering the Closed state...
            if (mState == State::kClosed)
            {
                // Prevent further callbacks to the error handler.
                //
                // Note: It is important to understand that LwIP can continue to make callbacks after
                // a PCB has been closed via the tcp_close() API. In particular, LwIP will continue
                // to call the 'data sent' callback to signal the acknowledgment of data that was
                // sent, but not acknowledged, prior to the close call. Additionally, LwIP will call
                // the error callback if the peer fails to respond in a timely manner to the either
                // sent data or the FIN. Unfortunately, there is no callback in the case where the
                // connection closes successfully. Because of this, it is impossible know definitively
                // when LwIP will no longer make callbacks to its user. Thus we must block further
                // callbacks to prevent them from happening after the endpoint has been freed.
                //
                RunOnTCPIP([this]() { tcp_err(mTCP, nullptr); });

                // If the endpoint is being closed without error, THEN call tcp_close() to close the underlying
                // TCP connection gracefully, preserving any in-transit send data.
                if (err == CHIP_NO_ERROR)
                {
                    RunOnTCPIP([this]() { tcp_close(mTCP); });
                }
                // OTHERWISE, call tcp_abort() to abort the TCP connection, discarding any in-transit data.
                else
                {
                    RunOnTCPIP([this]() { tcp_abort(mTCP); });
                }

                // Discard the reference to the PCB to ensure there is no further interaction with it
                // after this point.
                mTCP              = nullptr;
                mLwIPEndPointType = LwIPEndPointType::Unknown;
            }
        }

        // OTHERWISE the endpoint was being used for listening, so simply close it.
        else
        {
            RunOnTCPIP([this]() { tcp_close(mTCP); });

            // Discard the reference to the PCB to ensure there is no further interaction with it
            // after this point.
            mTCP              = nullptr;
            mLwIPEndPointType = LwIPEndPointType::Unknown;
        }
    }

    if (mPreAllocatedConnectEP)
    {
        // If the Listen EndPoint has a pre-allocated connect EndPoint, release it for the Retain() in the constructor
        mPreAllocatedConnectEP->Free();
        mPreAllocatedConnectEP = nullptr;
    }
    if (mState == State::kClosed)
    {
        mUnackedLength = 0;
    }
}

CHIP_ERROR TCPEndPointImplLwIP::AckReceive(size_t len)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_INVALID_ARGUMENT);

    if (mTCP != nullptr)
    {
        RunOnTCPIP([this, len]() { tcp_recved(mTCP, static_cast<uint16_t>(len)); });
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_CONNECTION_ABORTED;
}

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
void TCPEndPointImplLwIP::TCPUserTimeoutHandler()
{
    // Set the timer running flag to false
    mUserTimeoutTimerRunning = false;

    // Close Connection as we have timed out and there is still
    // data not sent out successfully.
    DoClose(INET_ERROR_TCP_USER_TIMEOUT, false);
}
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

uint16_t TCPEndPointImplLwIP::RemainingToSend()
{
    if (mSendQueue.IsNull())
    {
        return 0;
    }
    // We can never have reported more unacked data than there is pending
    // in the send queue! This would indicate a critical accounting bug.
    VerifyOrDie(mUnackedLength <= mSendQueue->TotalLength());
    return static_cast<uint16_t>(mSendQueue->TotalLength() - mUnackedLength);
}

TCPEndPointImplLwIP::BufferOffset TCPEndPointImplLwIP::FindStartOfUnsent()
{
    // Find first packet buffer with remaining data to send by skipping
    // all sent but un-acked data. This is necessary because of the Consume()
    // call in HandleDataSent(), which potentially releases backing memory for
    // fully-sent packet buffers, causing an invalidation of all possible
    // offsets one might have cached. The TCP acnowledgements may come back
    // with a variety of sizes depending on prior activity, and size of the
    // send window. The only way to ensure we get the correct offsets into
    // unsent data while retaining the buffers that have un-acked data is to
    // traverse all sent-but-unacked data in the chain to reach the beginning
    // of ready-to-send data.
    TCPEndPointImplLwIP::BufferOffset startOfUnsent(mSendQueue.Retain());
    uint16_t leftToSkip = mUnackedLength;

    VerifyOrDie(leftToSkip < mSendQueue->TotalLength());

    while (leftToSkip > 0)
    {
        VerifyOrDie(!startOfUnsent.buffer.IsNull());
        VerifyOrDie(CanCastTo<uint16_t>(startOfUnsent.buffer->DataLength()));
        uint16_t bufDataLen = static_cast<uint16_t>(startOfUnsent.buffer->DataLength());
        if (leftToSkip >= bufDataLen)
        {
            // We have more to skip than current packet buffer size.
            // Follow the chain to continue.
            startOfUnsent.buffer.Advance();
            leftToSkip = static_cast<uint16_t>(leftToSkip - bufDataLen);
        }
        else
        {
            // Done skipping all data, currentUnsentBuf is first packet buffer
            // containing unsent data.
            startOfUnsent.offset = leftToSkip;
            leftToSkip           = 0;
        }
    }

    return startOfUnsent;
}

CHIP_ERROR TCPEndPointImplLwIP::GetPCB(IPAddressType addrType)
{
    // IMMPORTANT: This method MUST be called with the LwIP stack LOCKED!
    if (mTCP == nullptr)
    {
        switch (addrType)
        {
        case IPAddressType::kIPv6:
            RunOnTCPIP([this]() { mTCP = tcp_new_ip_type(IPADDR_TYPE_V6); });
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPAddressType::kIPv4:
            RunOnTCPIP([this]() { mTCP = tcp_new_ip_type(IPADDR_TYPE_V4); });
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        if (mTCP == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mLwIPEndPointType = LwIPEndPointType::TCP;
    }
    else
    {
        switch (IP_GET_TYPE(&mTCP->local_ip))
        {
        case IPADDR_TYPE_V6:
            if (addrType != IPAddressType::kIPv6)
                return INET_ERROR_WRONG_ADDRESS_TYPE;
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPADDR_TYPE_V4:
            if (addrType != IPAddressType::kIPv4)
                return INET_ERROR_WRONG_ADDRESS_TYPE;
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            break;
        }
    }

    return CHIP_NO_ERROR;
}

void TCPEndPointImplLwIP::HandleDataSent(uint16_t lenSent)
{
    if (IsConnected())
    {
        // Ensure we do not have internal inconsistency in the lwIP, which
        // could cause invalid pointer accesses.
        if (lenSent > mUnackedLength)
        {
            ChipLogError(Inet, "Got more ACKed bytes (%d) than were pending (%d)", (int) lenSent, (int) mUnackedLength);
            DoClose(CHIP_ERROR_UNEXPECTED_EVENT, false);
            return;
        }
        if (mSendQueue.IsNull())
        {
            ChipLogError(Inet, "Got ACK for %d bytes but data backing gone", (int) lenSent);
            DoClose(CHIP_ERROR_UNEXPECTED_EVENT, false);
            return;
        }

        // Consume data off the head of the send queue equal to the amount of data being acknowledged.
        mSendQueue.Consume(lenSent);
        mUnackedLength = static_cast<uint16_t>(mUnackedLength - lenSent);

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
        // Only change the UserTimeout timer if lenSent > 0,
        // indicating progress being made in sending data
        // across.
        if (lenSent > 0)
        {
            if (RemainingToSend() == 0)
            {
                // If the output queue has been flushed then stop the timer.
                StopTCPUserTimeoutTimer();
            }
            else
            {
                // Progress is being made. So, shift the timer
                // forward if it was started.
                RestartTCPUserTimeoutTimer();
            }
        }
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

        // Mark the connection as being active.
        MarkActive();

        // If requested, call the app's OnDataSent callback.
        if (OnDataSent != nullptr)
        {
            OnDataSent(this, lenSent);
        }
        // If unsent data exists, attempt to send it now...
        if (RemainingToSend() > 0)
        {
            DriveSending();
        }
        // If in the closing state and the send queue is now empty, attempt to transition to closed.
        if ((mState == State::kClosing) && (RemainingToSend() == 0))
        {
            DoClose(CHIP_NO_ERROR, false);
        }
    }
}

void TCPEndPointImplLwIP::HandleDataReceived(System::PacketBufferHandle && buf)
{
    // Only receive new data while in the Connected or SendShutdown states.
    if (mState == State::kConnected || mState == State::kSendShutdown)
    {
        // Mark the connection as being active.
        MarkActive();

        // If we received a data buffer, queue it on the receive queue.  If there's already data in
        // the queue, compact the data into the head buffer.
        if (!buf.IsNull())
        {
            if (mRcvQueue.IsNull())
            {
                mRcvQueue = std::move(buf);
            }
            else
            {
                mRcvQueue->AddToEnd(std::move(buf));
                mRcvQueue->CompactHead();
            }
        }

        // Otherwise buf == NULL means the other side closed the connection, so ...
        else
        {

            // If in the Connected state and the app has provided an OnPeerClose callback,
            // enter the ReceiveShutdown state.  Providing an OnPeerClose callback allows
            // the app to decide whether to keep the send side of the connection open after
            // the peer has closed. If no OnPeerClose is provided, we assume that the app
            // wants to close both directions and automatically enter the Closing state.
            if (mState == State::kConnected && OnPeerClose != nullptr)
            {
                mState = State::kReceiveShutdown;
            }
            else
            {
                mState = State::kClosing;
            }
            // Call the app's OnPeerClose.
            if (OnPeerClose != NULL)
            {
                OnPeerClose(this);
            }
        }

        // Drive the received data into the app.
        DriveReceiving();
    }
}

void TCPEndPointImplLwIP::HandleIncomingConnection(TCPEndPoint * conEP)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    IPAddress peerAddr;
    uint16_t peerPort;

    if (mState == State::kListening)
    {
        // If there's no callback available, fail with an error.
        if (OnConnectionReceived == nullptr)
        {
            err = CHIP_ERROR_NO_CONNECTION_HANDLER;
        }
        // Extract the peer's address information.
        if (err == CHIP_NO_ERROR)
        {
            err = conEP->GetPeerInfo(&peerAddr, &peerPort);
        }
        // If successful, call the app's callback function.
        if (err == CHIP_NO_ERROR)
        {
            OnConnectionReceived(this, conEP, peerAddr, peerPort);
        }
        // Otherwise clean up and call the app's error callback.
        else if (OnAcceptError != nullptr)
        {
            OnAcceptError(this, err);
        }
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    // If something failed above, abort and free the connection end point.
    if (err != CHIP_NO_ERROR)
    {
        conEP->Free();
    }
}

void TCPEndPointImplLwIP::HandleError(CHIP_ERROR err)
{
    if (mState == State::kListening)
    {
        if (OnAcceptError != nullptr)
        {
            OnAcceptError(this, err);
        }
    }
    else
    {
        DoClose(err, false);
    }
}

err_t TCPEndPointImplLwIP::LwIPHandleConnectComplete(void * arg, struct tcp_pcb * tpcb, err_t lwipErr)
{
    err_t res = ERR_OK;

    if (arg != nullptr)
    {
        TCPEndPointImplLwIP * ep = static_cast<TCPEndPointImplLwIP *>(arg);

        if (lwipErr == ERR_OK)
        {
            // Setup LwIP callback functions for data transmission.
            tcp_recv(ep->mTCP, LwIPHandleDataReceived);
            tcp_sent(ep->mTCP, LwIPHandleDataSent);
        }

        // Post callback to HandleConnectComplete.
        ep->Retain();
        CHIP_ERROR err = ep->GetSystemLayer().ScheduleLambda([ep, conErr = System::MapErrorLwIP(lwipErr)] {
            ep->HandleConnectComplete(conErr);
            ep->Release();
        });
        if (err != CHIP_NO_ERROR)
        {
            ep->Release();
            res = ERR_ABRT;
        }
    }
    else
    {
        res = ERR_ABRT;
    }
    if (res != ERR_OK)
    {
        tcp_abort(tpcb);
    }
    return res;
}

err_t TCPEndPointImplLwIP::LwIPHandleIncomingConnection(void * arg, struct tcp_pcb * tpcb, err_t lwipErr)
{
    CHIP_ERROR err = chip::System::MapErrorLwIP(lwipErr);

    if (arg != nullptr)
    {
        TCPEndPointImplLwIP * listenEP = static_cast<TCPEndPointImplLwIP *>(arg);
        TCPEndPointImplLwIP * conEP    = nullptr;
        System::Layer & lSystemLayer   = listenEP->GetSystemLayer();

        // Tell LwIP we've accepted the connection so it can decrement the listen PCB's pending_accepts counter.
        tcp_accepted(listenEP->mTCP);

        // If we did in fact receive a connection, rather than an error, use the pre-allocated end point object for the incoming
        // connection.
        //
        // NOTE: Although most of the LwIP callbacks defer the real work to happen on the endpoint's thread
        // (by posting events to the thread's event queue) we can't do that here because as soon as this
        // function returns, LwIP is free to begin calling callbacks on the new PCB. For that to work we need
        // to have an end point associated with the PCB.
        //
        if (err == CHIP_NO_ERROR)
        {
            conEP = static_cast<TCPEndPointImplLwIP *>(listenEP->mPreAllocatedConnectEP);
            if (conEP == nullptr)
            {
                // The listen endpoint received a new incoming connection before it had a chance to pre-allocate a new connection
                // endpoint.
                err = CHIP_ERROR_BUSY;
            }
        }

        // Ensure that TCP timers have been started
        if (err == CHIP_NO_ERROR)
        {
            err_t error = start_tcp_timers();
            if (error != ERR_OK)
            {
                err = chip::System::MapErrorLwIP(error);
            }
        }

        // If successful in allocating an end point...
        if (err == CHIP_NO_ERROR)
        {
            // Put the new end point into the Connected state.
            conEP->mState            = State::kConnected;
            conEP->mTCP              = tpcb;
            conEP->mLwIPEndPointType = LwIPEndPointType::TCP;
            conEP->Retain();

            // Setup LwIP callback functions for the new PCB.
            tcp_arg(tpcb, conEP);
            tcp_recv(tpcb, LwIPHandleDataReceived);
            tcp_sent(tpcb, LwIPHandleDataSent);
            tcp_err(tpcb, LwIPHandleError);

            // Post a callback to the HandleConnectionReceived() function, passing it the new end point.
            listenEP->Retain();
            conEP->Retain();
            // Hand over the implied ref from constructing mPreAllocatedConnectEP to
            // ongoing connection.
            listenEP->mPreAllocatedConnectEP = nullptr;

            err = lSystemLayer.ScheduleLambda([listenEP, conEP] {
                listenEP->HandleIncomingConnection(conEP);
                // Pre-allocate another endpoint for next connection if current connection is established
                CHIP_ERROR error = listenEP->GetEndPointManager().NewEndPoint(&listenEP->mPreAllocatedConnectEP);
                if (error != CHIP_NO_ERROR)
                {
                    listenEP->HandleError(error);
                }
                conEP->Release();
                listenEP->Release();
            });
            if (err != CHIP_NO_ERROR)
            {
                conEP->Release(); // for the Ref in ScheduleLambda
                listenEP->Release();
                err = CHIP_ERROR_CONNECTION_ABORTED;
                conEP->Release(); // for the Retain() above
            }
        }

        // Otherwise, there was an error accepting the connection, so post a callback to the HandleError function.
        else
        {
            listenEP->Retain();
            err = lSystemLayer.ScheduleLambda([listenEP, err] {
                listenEP->HandleError(err);
                listenEP->Release();
            });
            if (err != CHIP_NO_ERROR)
            {
                listenEP->Release();
            }
        }
    }
    else
    {
        err = CHIP_ERROR_CONNECTION_ABORTED;
    }

    if (err != CHIP_NO_ERROR && tpcb != nullptr)
    {
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    return ERR_OK;
}

err_t TCPEndPointImplLwIP::LwIPHandleDataReceived(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t _err)
{
    err_t res = ERR_OK;

    if (arg != nullptr)
    {
        TCPEndPointImplLwIP * ep = static_cast<TCPEndPointImplLwIP *>(arg);

        // Post callback to HandleDataReceived.
        ep->Retain();
        CHIP_ERROR err = ep->GetSystemLayer().ScheduleLambda([ep, p] {
            ep->HandleDataReceived(System::PacketBufferHandle::Adopt(p));
            ep->Release();
        });
        if (err != CHIP_NO_ERROR)
        {
            ep->Release();
            res = ERR_ABRT;
        }
    }
    else
    {
        res = ERR_ABRT;
    }

    if (res != ERR_OK)
    {
        if (p != nullptr)
        {
            pbuf_free(p);
        }
        tcp_abort(tpcb);
    }

    return res;
}

err_t TCPEndPointImplLwIP::LwIPHandleDataSent(void * arg, struct tcp_pcb * tpcb, u16_t len)
{
    err_t res = ERR_OK;

    if (arg != nullptr)
    {
        TCPEndPointImplLwIP * ep = static_cast<TCPEndPointImplLwIP *>(arg);

        // Post callback to HandleDataReceived.
        ep->Retain();
        CHIP_ERROR err = ep->GetSystemLayer().ScheduleLambda([ep, len] {
            ep->HandleDataSent(len);
            ep->Release();
        });
        if (err != CHIP_NO_ERROR)
        {
            ep->Release();
            res = ERR_ABRT;
        }
    }
    else
    {
        res = ERR_ABRT;
    }

    if (res != ERR_OK)
    {
        tcp_abort(tpcb);
    }

    return res;
}

void TCPEndPointImplLwIP::LwIPHandleError(void * arg, err_t lwipErr)
{
    if (arg != nullptr)
    {
        TCPEndPointImplLwIP * ep     = static_cast<TCPEndPointImplLwIP *>(arg);
        System::Layer & lSystemLayer = ep->GetSystemLayer();

        // At this point LwIP has already freed the PCB.  Since the thread that owns the TCPEndPoint may
        // try to use the PCB before it receives the TCPError event posted below, we set the PCB to NULL
        // as a means to signal the other thread that the connection has been aborted.  The implication
        // of this is that the mTCP field is shared state between the two threads and thus must only be
        // accessed with the LwIP lock held.
        ep->mTCP              = nullptr;
        ep->mLwIPEndPointType = LwIPEndPointType::Unknown;

        // Post callback to HandleError.
        ep->Retain();
        CHIP_ERROR err = lSystemLayer.ScheduleLambda([ep, conErr = System::MapErrorLwIP(lwipErr)] {
            ep->HandleError(conErr);
            ep->Release();
        });
        if (err != CHIP_NO_ERROR)
        {
            ep->Release();
        }
    }
}

} // namespace Inet
} // namespace chip
