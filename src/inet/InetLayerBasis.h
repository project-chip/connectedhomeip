/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
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
 *      This file contains the basis class for reference counting
 *      objects by the Inet layer as well as a class for representing
 *      the pending or resulting I/O events on a socket.
 */

#pragma once

#include <inet/InetConfig.h>

#include <support/BitFlags.h>
#include <support/DLLUtil.h>
#include <system/SystemObject.h>

#include <stdint.h>
#include <type_traits>
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <sys/select.h>
#endif

namespace chip {
namespace Inet {

//--- Forward declaration of InetLayer singleton class
class InetLayer;

/**
 *  @class InetLayerBasis
 *
 *  @brief
 *    This is the basis class of reference-counted objects managed by an
 *    InetLayer object.
 *
 */
class InetLayerBasis : public chip::System::Object
{
public:
    InetLayer & Layer() const;
    bool IsCreatedByInetLayer(const InetLayer & aInetLayer) const;

protected:
    void InitInetLayerBasis(InetLayer & aInetLayer, void * aAppState = nullptr);

private:
    InetLayer * mInetLayer; /**< Pointer to the InetLayer object that owns this object. */
};

/**
 *  Returns a reference to the Inet layer object that owns this basis object.
 */
inline InetLayer & InetLayerBasis::Layer() const
{
    return *mInetLayer;
}

/**
 *  Returns \c true if the basis object was obtained by the specified INET layer instance.
 *
 *  @param[in]  aInetLayer    An instance of the INET layer.
 *
 *  @return     \c true if owned by \c aInetLayer, otherwise \c false.
 *
 *  @note
 *      Does not check whether the object is actually obtained by the system layer instance associated with the INET layer
 *      instance. It merely tests whether \c aInetLayer is the INET layer instance that was provided to \c InitInetLayerBasis.
 */
inline bool InetLayerBasis::IsCreatedByInetLayer(const InetLayer & aInetLayer) const
{
    return mInetLayer == &aInetLayer;
}

inline void InetLayerBasis::InitInetLayerBasis(InetLayer & aInetLayer, void * aAppState)
{
    AppState   = aAppState;
    mInetLayer = &aInetLayer;
}

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

/**
 *  @def INET_INVALID_SOCKET_FD
 *
 *  @brief
 *    This is the invalid socket file descriptor identifier.
 */
#define INET_INVALID_SOCKET_FD (-1)

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip
