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
