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
 *    @file
 *      This file implements the <tt>Inet::TCPEndPoint</tt> class,
 *      where the CHIP Inet Layer encapsulates methods for interacting
 *      with TCP transport endpoints (SOCK_DGRAM sockets on Linux and
 *      BSD-derived systems) or LwIP TCP protocol control blocks, as
 *      the system is configured accordingly.
 *
 */

#include <inet/TCPEndPoint.h>

#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <stdio.h>
#include <string.h>
#include <utility>

namespace chip {
namespace Inet {

CHIP_ERROR TCPEndPoint::Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr)
{
    VerifyOrReturnError(mState == State::kReady, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (addr != IPAddress::Any && addr.Type() != IPAddressType::kAny && addr.Type() != addrType)
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    res = BindImpl(addrType, addr, port, reuseAddr);

    if (res == CHIP_NO_ERROR)
    {
        mState = State::kBound;
    }

    return res;
}

CHIP_ERROR TCPEndPoint::Listen(uint16_t backlog)
{
    VerifyOrReturnError(mState == State::kBound, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_NO_ERROR;

    res = ListenImpl(backlog);

    if (res == CHIP_NO_ERROR)
    {
        // Once Listening, bump the reference count.  The corresponding call to Release() will happen in DoClose().
        Retain();
        mState = State::kListening;
    }

    return res;
}

CHIP_ERROR TCPEndPoint::Connect(const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    VerifyOrReturnError(mState == State::kReady || mState == State::kBound, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_NO_ERROR;

    ReturnErrorOnFailure(ConnectImpl(addr, port, intfId));

    StartConnectTimerIfSet();

    return res;
}

CHIP_ERROR TCPEndPoint::Send(System::PacketBufferHandle && data, bool push)
{
    VerifyOrReturnError(mState == State::kConnected || mState == State::kReceiveShutdown, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_NO_ERROR;

    bool queueWasEmpty = mSendQueue.IsNull();
    if (queueWasEmpty)
    {
        mSendQueue = std::move(data);
    }
    else
    {
        mSendQueue->AddToEnd(std::move(data));
    }

    ReturnErrorOnFailure(SendQueuedImpl(queueWasEmpty));

    if (push)
    {
        res = DriveSending();
    }

    return res;
}

CHIP_ERROR TCPEndPoint::SetReceivedDataForTesting(System::PacketBufferHandle && data)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    mRcvQueue = std::move(data);

    return CHIP_NO_ERROR;
}

uint32_t TCPEndPoint::PendingSendLength()
{
    if (!mSendQueue.IsNull())
    {
        return mSendQueue->TotalLength();
    }
    return 0;
}

uint32_t TCPEndPoint::PendingReceiveLength()
{
    if (!mRcvQueue.IsNull())
    {
        return mRcvQueue->TotalLength();
    }
    return 0;
}

void TCPEndPoint::Shutdown()
{
    VerifyOrReturn(IsConnected());

    // If fully connected, enter the SendShutdown state.
    if (mState == State::kConnected)
    {
        mState = State::kSendShutdown;
        DriveSending();
    }

    // Otherwise, if the peer has already closed their end of the connection,
    else if (mState == State::kReceiveShutdown)
    {
        DoClose(CHIP_NO_ERROR, false);
    }
}

void TCPEndPoint::Close()
{
    // Clear the receive queue.
    mRcvQueue = nullptr;

    // Suppress closing callbacks, since the application explicitly called Close().
    OnConnectionClosed = nullptr;
    OnPeerClose        = nullptr;
    OnConnectComplete  = nullptr;

    // Perform a graceful close.
    DoClose(CHIP_NO_ERROR, true);
}

void TCPEndPoint::Abort()
{
    // Suppress closing callbacks, since the application explicitly called Abort().
    OnConnectionClosed = nullptr;
    OnPeerClose        = nullptr;
    OnConnectComplete  = nullptr;

    DoClose(CHIP_ERROR_CONNECTION_ABORTED, true);
}

void TCPEndPoint::Free()
{
    // Ensure no callbacks to the app after this point.
    OnAcceptError        = nullptr;
    OnConnectComplete    = nullptr;
    OnConnectionReceived = nullptr;
    OnConnectionClosed   = nullptr;
    OnPeerClose          = nullptr;
    OnDataReceived       = nullptr;
    OnDataSent           = nullptr;

    // Ensure the end point is Closed or Closing.
    Close();

    // Release the Retain() that happened when the end point was allocated.
    Release();
}

#if INET_TCP_IDLE_CHECK_INTERVAL > 0
void TCPEndPoint::SetIdleTimeout(uint32_t timeoutMS)
{
    uint32_t newIdleTimeout = (timeoutMS + (INET_TCP_IDLE_CHECK_INTERVAL - 1)) / INET_TCP_IDLE_CHECK_INTERVAL;
    bool isIdleTimerRunning = IsIdleTimerRunning(GetEndPointManager());

    if (newIdleTimeout > UINT16_MAX)
    {
        newIdleTimeout = UINT16_MAX;
    }
    mIdleTimeout = mRemainingIdleTime = static_cast<uint16_t>(newIdleTimeout);

    if (!isIdleTimerRunning && mIdleTimeout)
    {
        GetSystemLayer().StartTimer(System::Clock::Milliseconds32(INET_TCP_IDLE_CHECK_INTERVAL), HandleIdleTimer,
                                    &GetEndPointManager());
    }
}

// static
void TCPEndPoint::HandleIdleTimer(chip::System::Layer * aSystemLayer, void * aAppState)
{
    auto & endPointManager = *reinterpret_cast<EndPointManager<TCPEndPoint> *>(aAppState);
    bool lTimerRequired    = IsIdleTimerRunning(endPointManager);

    endPointManager.ForEachEndPoint([](TCPEndPoint * lEndPoint) -> Loop {
        if (!lEndPoint->IsConnected())
            return Loop::Continue;
        if (lEndPoint->mIdleTimeout == 0)
            return Loop::Continue;

        if (lEndPoint->mRemainingIdleTime == 0)
        {
            lEndPoint->DoClose(INET_ERROR_IDLE_TIMEOUT, false);
        }
        else
        {
            --lEndPoint->mRemainingIdleTime;
        }

        return Loop::Continue;
    });

    if (lTimerRequired)
    {
        aSystemLayer->StartTimer(System::Clock::Milliseconds32(INET_TCP_IDLE_CHECK_INTERVAL), HandleIdleTimer, &endPointManager);
    }
}

// static
bool TCPEndPoint::IsIdleTimerRunning(EndPointManager<TCPEndPoint> & endPointManager)
{
    // See if there are any TCP connections with the idle timer check in use.
    return Loop::Break == endPointManager.ForEachEndPoint([](TCPEndPoint * lEndPoint) {
        return (lEndPoint->mIdleTimeout == 0) ? Loop::Continue : Loop::Break;
    });
}

#endif // INET_TCP_IDLE_CHECK_INTERVAL > 0

CHIP_ERROR TCPEndPoint::SetUserTimeout(uint32_t userTimeoutMillis)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR res = CHIP_NO_ERROR;

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    // Store the User timeout configuration if it is being overridden.
    mUserTimeoutMillis = userTimeoutMillis;

#else // !INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    res = SetUserTimeoutImpl(userTimeoutMillis);

#endif // !INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    return res;
}

void TCPEndPoint::StartConnectTimerIfSet()
{
    if (mConnectTimeoutMsecs > 0)
    {
        GetSystemLayer().StartTimer(System::Clock::Milliseconds32(mConnectTimeoutMsecs), TCPConnectTimeoutHandler, this);
    }
}

void TCPEndPoint::StopConnectTimer()
{
    GetSystemLayer().CancelTimer(TCPConnectTimeoutHandler, this);
}

void TCPEndPoint::TCPConnectTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState)
{
    TCPEndPoint * tcpEndPoint = reinterpret_cast<TCPEndPoint *>(aAppState);
    VerifyOrDie((aSystemLayer != nullptr) && (tcpEndPoint != nullptr));

    // Close Connection as we have timed out and Connect has not returned to stop this timer.
    tcpEndPoint->DoClose(INET_ERROR_TCP_CONNECT_TIMEOUT, false);
}

bool TCPEndPoint::IsConnected(State state)
{
    return state == State::kConnected || state == State::kSendShutdown || state == State::kReceiveShutdown ||
        state == State::kClosing;
}

CHIP_ERROR TCPEndPoint::DriveSending()
{
    CHIP_ERROR err = DriveSendingImpl();

    if (err != CHIP_NO_ERROR)
    {
        DoClose(err, false);
    }

    CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT();

    return err;
}

void TCPEndPoint::DriveReceiving()
{
    // If there's data in the receive queue and the app is ready to receive it then call the app's callback
    // with the entire receive queue.
    if (!mRcvQueue.IsNull() && mReceiveEnabled && OnDataReceived != nullptr)
    {
        // Acknowledgement is done after handling the buffers to allow the
        // application processing to throttle flow.
        uint16_t ackLength = mRcvQueue->TotalLength();
        CHIP_ERROR err     = OnDataReceived(this, std::move(mRcvQueue));
        if (err != CHIP_NO_ERROR)
        {
            DoClose(err, false);
            return;
        }
        AckReceive(ackLength);
    }

    // If the connection is closing, and the receive queue is now empty, call DoClose() to complete
    // the process of closing the connection.
    if (mState == State::kClosing && mRcvQueue.IsNull())
    {
        DoClose(CHIP_NO_ERROR, false);
    }
}

void TCPEndPoint::HandleConnectComplete(CHIP_ERROR err)
{
    // If the connect succeeded enter the Connected state and call the app's callback.
    if (err == CHIP_NO_ERROR)
    {
        // Stop the TCP Connect timer in case it is still running.
        StopConnectTimer();

        // Mark the connection as being active.
        MarkActive();

        mState = State::kConnected;

        HandleConnectCompleteImpl();

        if (OnConnectComplete != nullptr)
        {
            OnConnectComplete(this, CHIP_NO_ERROR);
        }
    }

    // Otherwise, close the connection with an error.
    else
    {
        DoClose(err, false);
    }
}

void TCPEndPoint::DoClose(CHIP_ERROR err, bool suppressCallback)
{
    State oldState = mState;

    // If in one of the connected states (Connected, LocalShutdown, PeerShutdown or Closing)
    // AND this is a graceful close (i.e. not prompted by an error)
    // AND there is data waiting to be processed on either the send or receive queues
    // ... THEN enter the Closing state, allowing the queued data to drain,
    // ... OTHERWISE go straight to the Closed state.
    if (IsConnected() && err == CHIP_NO_ERROR && (!mSendQueue.IsNull() || !mRcvQueue.IsNull()))
    {
        mState = State::kClosing;
    }
    else
    {
        mState = State::kClosed;
    }

    if (oldState != State::kClosed)
    {
        // Stop the Connect timer in case it is still running.
        StopConnectTimer();
    }

    // If not making a state transition, return immediately.
    if (mState == oldState)
    {
        return;
    }

    DoCloseImpl(err, oldState);

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    // Stop the TCP UserTimeout timer if it is running.
    StopTCPUserTimeoutTimer();
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    // If entering the Closed state...
    if (mState == State::kClosed)
    {
        // Clear clear the send and receive queues.
        mSendQueue = nullptr;
        mRcvQueue  = nullptr;

        // Call the appropriate app callback if allowed.
        if (!suppressCallback)
        {
            if (oldState == State::kConnecting)
            {
                if (OnConnectComplete != nullptr)
                {
                    OnConnectComplete(this, err);
                }
            }
            else if ((oldState == State::kConnected || oldState == State::kSendShutdown || oldState == State::kReceiveShutdown ||
                      oldState == State::kClosing) &&
                     OnConnectionClosed != nullptr)
            {
                OnConnectionClosed(this, err);
            }
        }

        // Decrement the ref count that was added when the connection started (in Connect()) or listening started (in Listen()).
        if (oldState != State::kReady && oldState != State::kBound)
        {
            Release();
        }
    }
}

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

void TCPEndPoint::ScheduleNextTCPUserTimeoutPoll(uint32_t aTimeOut)
{
    GetSystemLayer().StartTimer(System::Clock::Milliseconds32(aTimeOut), TCPUserTimeoutHandler, this);
}

void TCPEndPoint::StartTCPUserTimeoutTimer()
{
    ScheduleNextTCPUserTimeoutPoll(mUserTimeoutMillis);
    mUserTimeoutTimerRunning = true;
}

void TCPEndPoint::StopTCPUserTimeoutTimer()
{
    GetSystemLayer().CancelTimer(TCPUserTimeoutHandler, this);
    mUserTimeoutTimerRunning = false;
}

void TCPEndPoint::RestartTCPUserTimeoutTimer()
{
    StopTCPUserTimeoutTimer();
    StartTCPUserTimeoutTimer();
}

// static
void TCPEndPoint::TCPUserTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState)
{
    TCPEndPoint * tcpEndPoint = reinterpret_cast<TCPEndPoint *>(aAppState);
    VerifyOrDie((aSystemLayer != nullptr) && (tcpEndPoint != nullptr));
    tcpEndPoint->TCPUserTimeoutHandler();
}

#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

} // namespace Inet
} // namespace chip
