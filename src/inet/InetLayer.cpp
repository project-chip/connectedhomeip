/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines classes for abstracting access to and
 *      interactions with a platform- and system-specific Internet
 *      Protocol stack which, as of this implementation, may be either
 *      BSD/POSIX Sockets or LwIP.
 *
 *      Major abstractions provided are:
 *
 *        * Timers
 *        * Domain Name System (DNS) resolution
 *        * TCP network transport
 *        * UDP network transport
 *        * Raw network transport
 *
 *      For BSD/POSIX Sockets (CHIP_SYSTEM_CONFIG_USE_SOCKETS), event readiness
 *      notification is handled via file descriptors, using a System::Layer API.
 *
 *      For LwIP (CHIP_SYSTEM_CONFIG_USE_LWIP), event readiness notification is handled
 *      via events / messages and platform- and system-specific hooks for the event
 *      / message system.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include "InetLayer.h"

#include "InetFaultInjection.h"

#include <platform/LockTracker.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utility>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/netif.h>
#include <lwip/sys.h>
#else // !CHIP_SYSTEM_CONFIG_USE_LWIP
#include <fcntl.h>
#include <net/if.h>
#include <unistd.h>
#ifdef __ANDROID__
#include <ifaddrs-android.h>
#elif CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <ifaddrs.h>
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#include <net/net_if.h>
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

namespace chip {
namespace Inet {

/**
 *  This is the InetLayer default constructor.
 *
 *  It performs some basic data member initialization; however, since
 *  InetLayer follows an explicit initializer design pattern, the InetLayer::Init
 *  method must be called successfully prior to using the object.
 *
 */
InetLayer::InetLayer() {}

/**
 *  This is the InetLayer explicit initializer. This must be called
 *  and complete successfully before the InetLayer may be used.
 *
 *  The caller may provide an optional context argument which will be
 *  passed back via any platform-specific hook functions. For
 *  LwIP-based adaptations, this will typically be a pointer to the
 *  event queue associated with the InetLayer instance.
 *
 *  @param[in]  aSystemLayer  A required instance of the chip System Layer
 *                            already successfully initialized.
 *
 *  @param[in]  aContext  An optional context argument which will be passed
 *                        back to the caller via any platform-specific hook
 *                        functions.
 *
 *  @retval   #CHIP_ERROR_INCORRECT_STATE        If the InetLayer is in an
 *                                               incorrect state.
 *  @retval   #CHIP_ERROR_NO_MEMORY              If the InetLayer runs out
 *                                               of resource for this
 *                                               request for a new timer.
 *  @retval   other Platform-specific errors indicating the reason for
 *            initialization failure.
 *  @retval   #CHIP_NO_ERROR                     On success.
 *
 */
CHIP_ERROR InetLayer::Init(chip::System::Layer & aSystemLayer, void * aContext)
{
    Inet::RegisterLayerErrorFormatter();
    VerifyOrReturnError(mLayerState.SetInitializing(), CHIP_ERROR_INCORRECT_STATE);

    // Platform-specific initialization may elect to set this data
    // member. Ensure it is set to a sane default value before
    // invoking platform-specific initialization.

    mPlatformData = nullptr;

    mSystemLayer = &aSystemLayer;
    mContext     = aContext;

    mLayerState.SetInitialized();

    return CHIP_NO_ERROR;
}

/**
 *  This is the InetLayer explicit deinitializer and should be called
 *  prior to disposing of an instantiated InetLayer instance.
 *
 *  @return #CHIP_NO_ERROR on success; otherwise, a specific error indicating
 *          the reason for shutdown failure.
 *
 */
CHIP_ERROR InetLayer::Shutdown()
{
    VerifyOrReturnError(mLayerState.SetShuttingDown(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Abort all TCP endpoints owned by this instance.
    TCPEndPointImpl::sPool.ForEachActiveObject([&](TCPEndPoint * lEndPoint) {
        if ((lEndPoint != nullptr) && &lEndPoint->Layer() == this)
        {
            lEndPoint->Abort();
        }
        return true;
    });
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
    // Close all UDP endpoints owned by this instance.
    UDPEndPointImpl::sPool.ForEachActiveObject([&](UDPEndPoint * lEndPoint) {
        if ((lEndPoint != nullptr) && &lEndPoint->Layer() == this)
        {
            lEndPoint->Close();
        }
        return true;
    });
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

    mLayerState.SetShutdown();
    mLayerState.Reset(); // Return to uninitialized state to permit re-initialization.
    return err;
}

/**
 * This returns any client-specific platform data assigned to the
 * instance, if it has been previously set.
 *
 * @return Client-specific platform data, if is has been previously set;
 *         otherwise, NULL.
 *
 */
void * InetLayer::GetPlatformData()
{
    return mPlatformData;
}

/**
 * This sets the specified client-specific platform data to the
 * instance for later retrieval by the client platform.
 *
 * @param[in]  aPlatformData  The client-specific platform data to set.
 *
 */
void InetLayer::SetPlatformData(void * aPlatformData)
{
    mPlatformData = aPlatformData;
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
bool InetLayer::IsIdleTimerRunning()
{
    bool timerRunning = false;

    // See if there are any TCP connections with the idle timer check in use.
    TCPEndPointImpl::sPool.ForEachActiveObject([&](TCPEndPoint * lEndPoint) {
        if ((lEndPoint != nullptr) && (lEndPoint->mIdleTimeout != 0))
        {
            timerRunning = true;
            return false;
        }
        return true;
    });

    return timerRunning;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
/**
 *  Creates a new TCPEndPoint object.
 *
 *  @note
 *    This function gets a free TCPEndPoint object from a pre-allocated pool
 *    and also calls the explicit initializer on the new object.
 *
 *  @param[in,out]  retEndPoint    A pointer to a pointer of the TCPEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #CHIP_ERROR_INCORRECT_STATE    If the InetLayer object is not initialized.
 *  @retval  #CHIP_ERROR_ENDPOINT_POOL_FULL If the InetLayer TCPEndPoint pool is full and no new
 *                                          endpoints can be created.
 *  @retval  #CHIP_NO_ERROR                 On success.
 *
 */
CHIP_ERROR InetLayer::NewTCPEndPoint(TCPEndPoint ** retEndPoint)
{
    assertChipStackLockedByCurrentThread();

    *retEndPoint = nullptr;

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = TCPEndPointImpl::sPool.CreateObject(*this);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "TCP");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kInetLayer_NumTCPEps);

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
/**
 *  Creates a new UDPEndPoint object.
 *
 *  @note
 *    This function gets a free UDPEndPoint object from a pre-allocated pool
 *    and also calls the explicit initializer on the new object.
 *
 *  @param[in,out]  retEndPoint    A pointer to a pointer of the UDPEndPoint object that is
 *                                 a return parameter upon completion of the object creation.
 *                                 *retEndPoint is NULL if creation fails.
 *
 *  @retval  #CHIP_ERROR_INCORRECT_STATE    If the InetLayer object is not initialized.
 *  @retval  #CHIP_ERROR_ENDPOINT_POOL_FULL If the InetLayer UDPEndPoint pool is full and no new
 *                                          endpoints can be created.
 *  @retval  #CHIP_NO_ERROR                 On success.
 *
 */
CHIP_ERROR InetLayer::NewUDPEndPoint(UDPEndPoint ** retEndPoint)
{
    assertChipStackLockedByCurrentThread();

    *retEndPoint = nullptr;

    VerifyOrReturnError(mLayerState.IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    *retEndPoint = UDPEndPointImpl::sPool.CreateObject(*this);
    if (*retEndPoint == nullptr)
    {
        ChipLogError(Inet, "%s endpoint pool FULL", "UDP");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kInetLayer_NumUDPEps);

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

#if INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0
void InetLayer::HandleTCPInactivityTimer(chip::System::Layer * aSystemLayer, void * aAppState)
{
    InetLayer & lInetLayer = *reinterpret_cast<InetLayer *>(aAppState);
    bool lTimerRequired    = lInetLayer.IsIdleTimerRunning();

    TCPEndPointImpl::sPool.ForEachActiveObject([&](TCPEndPoint * lEndPoint) {
        if (&lEndPoint->Layer() != &lInetLayer)
            return true;
        if (!lEndPoint->IsConnected())
            return true;
        if (lEndPoint->mIdleTimeout == 0)
            return true;

        if (lEndPoint->mRemainingIdleTime == 0)
        {
            lEndPoint->DoClose(INET_ERROR_IDLE_TIMEOUT, false);
        }
        else
        {
            --lEndPoint->mRemainingIdleTime;
        }

        return true;
    });

    if (lTimerRequired)
    {
        aSystemLayer->StartTimer(System::Clock::Milliseconds32(INET_TCP_IDLE_CHECK_INTERVAL), HandleTCPInactivityTimer,
                                 &lInetLayer);
    }
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT && INET_TCP_IDLE_CHECK_INTERVAL > 0

/**
 *  Reset the members of the IPPacketInfo object.
 *
 */
void IPPacketInfo::Clear()
{
    SrcAddress  = IPAddress::Any;
    DestAddress = IPAddress::Any;
    Interface   = InterfaceId::Null();
    SrcPort     = 0;
    DestPort    = 0;
}

} // namespace Inet
} // namespace chip
