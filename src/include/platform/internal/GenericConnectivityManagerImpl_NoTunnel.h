/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Provides an generic implementation of ConnectivityManager features
 *          for use on platforms that don't support a service tunnel.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_TUNNEL_H
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_TUNNEL_H

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of service tunnel-specific ConnectivityManager features for
 * platforms that don't support a service tunnel.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 * The members of this class are all inlined methods that do nothing, and return static return
 * values.  This allows the compiler to optimize away dead code without the use of #ifdef's.
 * For example:
 *
 * ```
 * if (ConnectivityMgr().GetServiceTunnelMode() != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
 * {
 *     // ... do something on devices that support service tunnels ...
 * }
 * ```
 */
template<class ImplClass>
class GenericConnectivityManagerImpl_NoTunnel
{
public:

    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::ServiceTunnelMode _GetServiceTunnelMode(void);
    CHIP_ERROR _SetServiceTunnelMode(ConnectivityManager::ServiceTunnelMode val);
    bool _IsServiceTunnelConnected(void);
    bool _IsServiceTunnelRestricted(void);
    bool _HaveServiceConnectivityViaTunnel(void);
    static const char * _ServiceTunnelModeToStr(ConnectivityManager::ServiceTunnelMode mode);

private:

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template<class ImplClass>
inline ConnectivityManager::ServiceTunnelMode GenericConnectivityManagerImpl_NoTunnel<ImplClass>::_GetServiceTunnelMode(void)
{
    return ConnectivityManager::kServiceTunnelMode_NotSupported;
}

template<class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoTunnel<ImplClass>::_SetServiceTunnelMode(ConnectivityManager::ServiceTunnelMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_NoTunnel<ImplClass>::_IsServiceTunnelConnected(void)
{
    return false;
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_NoTunnel<ImplClass>::_IsServiceTunnelRestricted(void)
{
    return false;
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_NoTunnel<ImplClass>::_HaveServiceConnectivityViaTunnel(void)
{
    return false;
}

template<class ImplClass>
inline const char * GenericConnectivityManagerImpl_NoTunnel<ImplClass>::_ServiceTunnelModeToStr(ConnectivityManager::ServiceTunnelMode mode)
{
    return NULL;
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_TUNNEL_H
