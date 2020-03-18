/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#ifndef CONNECTIVITY_MANAGER_IMPL_H
#define CONNECTIVITY_MANAGER_IMPL_H

#include <Weave/DeviceLayer/ConnectivityManager.h>
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl.h>
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_Thread.h>
#else
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_NoThread.h>
#endif
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_NoTunnel.h>
#include <Weave/Support/FlagUtils.hpp>

namespace nl {
namespace Inet {
class IPAddress;
} // namespace Inet
} // namespace nl

namespace nl {
namespace Weave {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for Silicon Labs EFR32 platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
                                      public Internal::GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
                                      public Internal::GenericConnectivityManagerImpl_NoTunnel<ConnectivityManagerImpl>
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    bool        _HaveIPv4InternetConnectivity(void);
    bool        _HaveIPv6InternetConnectivity(void);
    bool        _HaveServiceConnectivity(void);
    WEAVE_ERROR _Init(void);
    void        _OnPlatformEvent(const WeaveDeviceEvent *event);

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager &    ConnectivityMgr(void);
    friend ConnectivityManagerImpl &ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;
};

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return _HaveServiceConnectivityViaThread();
}

/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * Weave applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager &ConnectivityMgr(void)
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * Weave applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the ESP32 platform.
 */
inline ConnectivityManagerImpl &ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // CONNECTIVITY_MANAGER_IMPL_H
