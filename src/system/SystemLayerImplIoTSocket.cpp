/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file implements Layer using select().
 */

#include <cmsis_os2.h>
#include <inet/EndPointStateIoTSocket.h>
#include <iot_socket.h>
#include <lib/support/CodeUtils.h>
#include <system/PlatformEventSupport.h>
#include <system/SystemFaultInjection.h>
#include <system/SystemLayer.h>
#include <system/SystemLayerImplIoTSocket.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/inet.h>
#include <lwip/ip_addr.h>
#endif //

using namespace ::chip::Inet;

namespace chip {
namespace System {

enum signal_flags_t
{
    SIGNAL_FLAGS_WAITING_FOR_SELECT_RETURN = 0x01,
    SIGNAL_FLAGS_SELECT_RETURNED           = 0x02,
    SIGNAL_FLAGS_SELECT_PENDING            = 0x04
};

constexpr uint16_t SIGNAL_SOCKET_PORT = 1;

std::atomic<int32_t> LayerImplOpenIoTSDK::mSignalSocket{ EndPointStateIoTSocket::kInvalidSocketFd };
Mutex LayerImplOpenIoTSDK::mSelectMutex{};
size_t LayerImplOpenIoTSDK::mMaskSize = 0;

LayerImplOpenIoTSDK::LayerImplOpenIoTSDK() : mHandlingTimerComplete(false) {}

CHIP_ERROR LayerImplOpenIoTSDK::Init()
{
    if (mLayerState.IsInitialized())
    {
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

    Mutex::Init(mSelectMutex);
    Mutex::Init(mSignalMutex);
    mSignalFlags = osEventFlagsNew(nullptr);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    RegisterLwIPErrorFormatter();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    ReturnErrorOnFailure(EnableSignalSocket());

    VerifyOrReturnError(mLayerState.SetInitialized(), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void LayerImplOpenIoTSDK::Shutdown()
{
    if (mSignalSocket != EndPointStateIoTSocket::kInvalidSocketFd)
    {
        iotSocketClose(mSignalSocket);
        mSignalSocket = EndPointStateIoTSocket::kInvalidSocketFd;
    }
    if (mSignalFlags)
    {
        osEventFlagsDelete(mSignalFlags);
        mSignalFlags = nullptr;
    }
    mLayerState.ResetFromInitialized();
}

CHIP_ERROR LayerImplOpenIoTSDK::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_SYSTEM_FAULT_INJECT(FaultInjection::kFault_TimeoutImmediate, delay = Clock::kZero);

    CancelTimer(onComplete, appState);

    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp() + delay, onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // this is the new earliest timer and so the timer needs (re-)starting provided that
        // the system is not currently processing expired timers, in which case it is left to
        // HandleExpiredTimers() to re-start the timer.
        if (!mHandlingTimerComplete)
        {
            StartPlatformTimer(delay);
        }
    }
    return CHIP_NO_ERROR;
}

void LayerImplOpenIoTSDK::CancelTimer(TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturn(mLayerState.IsInitialized());

    TimerList::Node * timer = mTimerList.Remove(onComplete, appState);
    if (timer != nullptr)
    {
        mTimerPool.Release(timer);
    }
}

CHIP_ERROR LayerImplOpenIoTSDK::ScheduleWork(TimerCompleteCallback onComplete, void * appState)
{
    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    TimerList::Node * timer = mTimerPool.Create(*this, SystemClock().GetMonotonicTimestamp(), onComplete, appState);
    VerifyOrReturnError(timer != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mTimerList.Add(timer) == timer)
    {
        // The new timer is the earliest, so the time until the next event has probably changed.
        Signal();
    }
    return CHIP_NO_ERROR;
}

/**
 * Start the platform timer with specified millsecond duration.
 */
CHIP_ERROR LayerImplOpenIoTSDK::StartPlatformTimer(System::Clock::Timeout aDelay)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR status = PlatformEventing::StartTimer(*this, aDelay);
    return status;
}

/**
 * Handle the platform timer expiration event. Completes any timers that have expired.
 *
 * A static API that gets called when the platform timer expires. Any expired timers are completed and removed from the list
 * of active timers in the layer object. If unexpired timers remain on completion, StartPlatformTimer will be called to
 * restart the platform timer.
 *
 * It is assumed that this API is called only while on the thread which owns the CHIP System Layer object.
 *
 * @note
 *      It's harmless if this API gets called and there are no expired timers.
 *
 *  @return CHIP_NO_ERROR on success, error code otherwise.
 *
 */
CHIP_ERROR LayerImplOpenIoTSDK::HandlePlatformTimer()
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    // Expire each timer in turn until an unexpired timer is reached or the timerlist is emptied.  We set the current expiration
    // time outside the loop; that way timers set after the current tick will not be executed within this expiration window
    // regardless how long the processing of the currently expired timers took.
    // The platform timer API has MSEC resolution so expire any timer with less than 1 msec remaining.
    Clock::Timestamp expirationTime = SystemClock().GetMonotonicTimestamp() + Clock::Timeout(1);

    // limit the number of timers handled before the control is returned to the event queue.  The bound is similar to
    // (though not exactly same) as that on the sockets-based systems.

    size_t timersHandled    = 0;
    TimerList::Node * timer = nullptr;
    while ((timersHandled < CHIP_SYSTEM_CONFIG_NUM_TIMERS) && ((timer = mTimerList.PopIfEarlier(expirationTime)) != nullptr))
    {
        mHandlingTimerComplete = true;
        mTimerPool.Invoke(timer);
        mHandlingTimerComplete = false;
        timersHandled++;
    }

    if (!mTimerList.Empty())
    {
        // timers still exist so restart the platform timer.
        Clock::Timeout delay = System::Clock::kZero;

        Clock::Timestamp currentTime = SystemClock().GetMonotonicTimestamp();

        if (currentTime < mTimerList.Earliest()->AwakenTime())
        {
            // the next timer expires in the future, so set the delay to a non-zero value
            delay = mTimerList.Earliest()->AwakenTime() - currentTime;
        }

        StartPlatformTimer(delay);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplOpenIoTSDK::EnableSignalSocket()
{
    // early return to avoid mutex
    if (mSignalSocket != EndPointStateIoTSocket::kInvalidSocketFd)
    {
        return CHIP_NO_ERROR;
    }

    mSelectMutex.Lock();

    // we need to check again in case someone else created it since we got the lock
    if (mSignalSocket != EndPointStateIoTSocket::kInvalidSocketFd)
    {
        mSelectMutex.Unlock();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    // create dummy signal socket
    int32_t socket_id = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_IPPROTO_UDP);
    if (socket_id < 0)
    {
        // return early, no need to cleanup as we failed to create the socket
        mSelectMutex.Unlock();
        return CHIP_ERROR_INTERNAL;
    }

    int32_t opt = 1;
    if (iotSocketSetOpt(socket_id, IOT_SOCKET_IO_FIONBIO, &opt, sizeof(opt)) != 0)
    {
        err = CHIP_ERROR_INTERNAL;
    }

    if (err == CHIP_NO_ERROR)
    {
        // build loopback address
        iot_in_addr loopback_addr;
        loopback_addr.s_addr = htonl(INADDR_LOOPBACK);

        if (iotSocketBind(socket_id, (uint8_t *) &loopback_addr, sizeof(loopback_addr), SIGNAL_SOCKET_PORT) != 0)
        {
            ChipLogError(NotSpecified, "Cannot bind the signal socket");
            err = CHIP_ERROR_INTERNAL;
        }

        // create select masks and add the signal socket to read mask
        mMaskSize = iotSocketMaskGetSize();

        if (mMaskSize && !mMaskMemory)
        {
            // allocate memory block that can be traversed with uin32_t pointer
            const size_t aligned_size = ((mMaskSize * SelectMaskTypeMax) + 3) & ~3;

            mMaskMemory = (SelectMask) malloc(aligned_size);
        }

        if (!mMaskMemory)
        {
            ChipLogError(NotSpecified, "Cannot allocate memory for mask size %lu", (uint32_t)(mMaskSize * SelectMaskTypeMax));
            err = CHIP_ERROR_INTERNAL;
        }
        else
        {
            for (size_t i = 0; i < SelectMaskTypeMax; i++)
            {
                iotSocketMaskZero(GetSelectMask((SelectMaskType) i));
            }
            iotSocketMaskSet(socket_id, GetSelectMask(ReadMask));
        }
    }

    // if we failed we need to close the socket to be able to retry
    if (err != CHIP_NO_ERROR)
    {
        iotSocketClose(socket_id);
    }
    else
    {
        mSignalSocket = socket_id;
    }

    mSelectMutex.Unlock();

    return err;
}

CHIP_ERROR LayerImplOpenIoTSDK::EnableSelectCallback(chip::Inet::EndPointStateIoTSocket * endpoint, bool read, bool write)
{
    EnableSignalSocket();
    mSelectMutex.Lock();

    bool add = true;
    for (size_t i = 0; i < mSelectEndpointsNumber; ++i)
    {
        if (mSelectEndpoints[i] == endpoint)
        {
            add = false;
            // already enabled
            break;
        }
    }

    if (add)
    {
        if (mSelectEndpointsNumber == kSocketMax)
        {
            // impossible to have more endpoints than allowed
            // maybe some endpoints haven't removed their callback
            mSelectMutex.Unlock();
            return CHIP_ERROR_INCORRECT_STATE;
        }
        else
        {
            mSelectEndpoints[mSelectEndpointsNumber] = endpoint;
            mSelectEndpointsNumber++;
        }
    }

    if (read)
    {
        iotSocketMaskSet(endpoint->GetSocketId(), GetSelectMask(ReadMask));
    }
    if (write)
    {
        iotSocketMaskSet(endpoint->GetSocketId(), GetSelectMask(WriteMask));
    }

    mSelectMutex.Unlock();

    Signal();

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplOpenIoTSDK::DisableSelectCallback(chip::Inet::EndPointStateIoTSocket * endpoint, bool read, bool write)
{
    EnableSignalSocket();
    mSelectMutex.Lock();

    // if the either mask remains set do not remove
    bool keep = false;
    if (endpoint->GetSocketId() != EndPointStateIoTSocket::kInvalidSocketFd)
    {
        if (read)
        {
            iotSocketMaskUnset(endpoint->GetSocketId(), GetSelectMask(ReadMask));
        }
        else
        {
            keep = iotSocketMaskIsSet(endpoint->GetSocketId(), GetSelectMask(ReadMask));
        }

        if (write)
        {
            iotSocketMaskUnset(endpoint->GetSocketId(), GetSelectMask(WriteMask));
        }
        else
        {
            keep |= iotSocketMaskIsSet(endpoint->GetSocketId(), GetSelectMask(WriteMask));
        }
    }

    if (!keep)
    {
        // find the one to remove
        for (size_t i = 0; i < mSelectEndpointsNumber; ++i)
        {
            if (mSelectEndpoints[i] == endpoint)
            {
                mSelectEndpoints[i] = nullptr;
                mSelectEndpointsNumber--;

                // if this element wasn't last, grab the last one which is now beyond mSelectEndpointsNumber limit
                if (i != mSelectEndpointsNumber)
                {
                    mSelectEndpoints[i] = mSelectEndpoints[mSelectEndpointsNumber];
                }
                break;
            }
        }
    }

    bool select_pending = ((osEventFlagsGet(mSignalFlags) & SIGNAL_FLAGS_SELECT_PENDING) != 0);

    mSelectMutex.Unlock();

    if (select_pending)
    {
        mSignalMutex.Lock();

        osEventFlagsSet(mSignalFlags, SIGNAL_FLAGS_WAITING_FOR_SELECT_RETURN);
        Signal();
        osEventFlagsWait(mSignalFlags, SIGNAL_FLAGS_SELECT_RETURNED, 0, osWaitForever);

        mSignalMutex.Unlock();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LayerImplOpenIoTSDK::WaitForEvents()
{
    if (mSignalSocket != EndPointStateIoTSocket::kInvalidSocketFd)
    {
        // update the masks
        mSelectMutex.Lock();
        osEventFlagsSet(mSignalFlags, SIGNAL_FLAGS_SELECT_PENDING);
        memcpy(GetSelectMask(ReadMaskOut), GetSelectMask(ReadMask), mMaskSize);
        memcpy(GetSelectMask(WriteMaskOut), GetSelectMask(WriteMask), mMaskSize);
        memcpy(GetSelectMask(ExceptionMaskOut), GetSelectMask(ExceptionMask), mMaskSize);
        mSelectMutex.Unlock();

        int32_t ret = iotSocketSelect(GetSelectMask(ReadMaskOut), GetSelectMask(WriteMaskOut), GetSelectMask(ExceptionMaskOut),
                                      osWaitForever);

        osEventFlagsClear(mSignalFlags, SIGNAL_FLAGS_SELECT_PENDING);
        if ((osEventFlagsGet(mSignalFlags) & SIGNAL_FLAGS_WAITING_FOR_SELECT_RETURN) != 0)
        {
            osEventFlagsClear(mSignalFlags, SIGNAL_FLAGS_WAITING_FOR_SELECT_RETURN);
            osEventFlagsSet(mSignalFlags, SIGNAL_FLAGS_SELECT_RETURNED);
        }

        if (ret < 0)
        {
            ChipLogError(NotSpecified, "Select failed with error: %ld", ret);
            return CHIP_ERROR_INTERNAL;
        }
    }

    return CHIP_NO_ERROR;
}

void LayerImplOpenIoTSDK::Signal()
{
    if (EnableSignalSocket() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Cannot initialise Signal socket");
    }
    else
    {
        iot_in_addr loopback_addr;
        loopback_addr.s_addr = htonl(INADDR_LOOPBACK);
        const char dummy     = 0x01;
        int32_t ret =
            iotSocketSendTo(mSignalSocket, &dummy, 1, (uint8_t *) &loopback_addr, sizeof(loopback_addr), SIGNAL_SOCKET_PORT);
        if (ret < 0)
        {
            ChipLogError(NotSpecified, "Cannot send on Signal socket");
        }
    }
}

void LayerImplOpenIoTSDK::HandleEvents()
{
    if (mSignalSocket != EndPointStateIoTSocket::kInvalidSocketFd)
    {
        char dummy;
        bool unset = false;
        while (iotSocketRecv(mSignalSocket, &dummy, sizeof(dummy)) == sizeof(dummy))
        {
            unset = true;
        }

        if (unset)
        {
            iotSocketMaskUnset(mSignalSocket, GetSelectMask(ReadMaskOut));
        }

        if (IsResultSelectMaskSet())
        {
            for (size_t i = 0; i < mSelectEndpointsNumber; ++i)
            {
                mSelectEndpoints[i]->SelectCallback(GetSelectMask(ReadMaskOut), GetSelectMask(WriteMaskOut),
                                                    GetSelectMask(ExceptionMaskOut));
            }
        }
    }
}

} // namespace System
} // namespace chip
