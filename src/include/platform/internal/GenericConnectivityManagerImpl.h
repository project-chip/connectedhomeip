/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 * or indirectly) by the ConnectivityManagerImpl class, which also appears as the template's ImplClass
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

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
