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
 *          for use on various platforms.
 */

#pragma once

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of ConnectivityManager features that works on multiple platforms.
 *
 * This template contains implementations of select features from the ConnectivityManager abstract
 * interface that are suitable for use on all platforms.  It is intended to be inherited (directly
 * or indirectly) by the ConfigurationManagerImpl class, which also appears as the template's ImplClass
 * parameter.
 */
template <class ImplClass>
class GenericConnectivityManagerImpl
{
public:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    bool _IsUserSelectedModeActive();
    void _SetUserSelectedMode(bool val);
    uint16_t _GetUserSelectedModeTimeout();
    void _SetUserSelectedModeTimeout(uint16_t val);

    void _ReleaseNetworkInterfaces(struct NetworkInterface ** netifp);
    CHIP_ERROR _GetNetworkInterfaces(struct NetworkInterface ** netifp);
    CHIP_ERROR _GetEthPHYRate(uint8_t & pHYRate);
    CHIP_ERROR _GetEthFullDuplex(bool & fullDuplex);
    CHIP_ERROR _GetEthCarrierDetect(bool & carrierDetect);
    CHIP_ERROR _GetEthTimeSinceReset(uint64_t & timeSinceReset);
    CHIP_ERROR _GetEthPacketRxCount(uint64_t & packetRxCount);
    CHIP_ERROR _GetEthPacketTxCount(uint64_t & packetTxCount);
    CHIP_ERROR _GetEthTxErrCount(uint64_t & txErrCount);
    CHIP_ERROR _GetEthCollisionCount(uint64_t & collisionCount);
    CHIP_ERROR _GetEthOverrunCount(uint64_t & overrunCount);
    CHIP_ERROR _ResetEthNetworkDiagnosticsCounts();

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline bool GenericConnectivityManagerImpl<ImplClass>::_IsUserSelectedModeActive()
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl<ImplClass>::_SetUserSelectedMode(bool val)
{}

template <class ImplClass>
inline uint16_t GenericConnectivityManagerImpl<ImplClass>::_GetUserSelectedModeTimeout()
{
    return 0;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl<ImplClass>::_SetUserSelectedModeTimeout(uint16_t val)
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl<ImplClass>::_ReleaseNetworkInterfaces(struct NetworkInterface ** netifp)
{}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetNetworkInterfaces(struct NetworkInterface ** netifp)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthPHYRate(uint8_t & pHYRate)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthFullDuplex(bool & fullDuplex)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthCarrierDetect(bool & carrierDetect)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthTimeSinceReset(uint64_t & timeSinceReset)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthPacketRxCount(uint64_t & packetRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthPacketTxCount(uint64_t & packetTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthTxErrCount(uint64_t & txErrCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthCollisionCount(uint64_t & collisionCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_GetEthOverrunCount(uint64_t & overrunCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl<ImplClass>::_ResetEthNetworkDiagnosticsCounts()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
