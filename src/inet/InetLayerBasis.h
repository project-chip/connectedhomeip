/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <support/DLLUtil.h>
#include <system/SystemObject.h>

#include <stdint.h>
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
 *  @class SocketEvents
 *
 *  @brief
 *    Represent a set of I/O events requested/pending on a socket.
 *
 */
class SocketEvents
{
public:
    enum : uint8_t
    {
        kRead  = 0x01, /**< Bit flag indicating if there is a read event on a socket. */
        kWrite = 0x02, /**< Bit flag indicating if there is a write event on a socket. */
        kError = 0x04, /**< Bit flag indicating if there is an error event on a socket. */
    };

    int Value; /**< Contains the bit flags for the socket event. */

    /**
     *  Constructor for the SocketEvents class.
     *
     */
    SocketEvents() { Value = 0; }

    /**
     *  Copy constructor for the SocketEvents class.
     *
     */
    SocketEvents(const SocketEvents & other) { Value = other.Value; }

    /**
     *  Copy assignment operator for the SocketEvents class.
     *
     */
    SocketEvents & operator=(const SocketEvents & other) = default;

    /**
     *  Check if any of the bit flags for the socket events are set.
     *
     *  @return true if set, otherwise false.
     *
     */
    bool IsSet() const { return Value != 0; }

    /**
     *  Check if the bit flags indicate that the socket is readable.
     *
     *  @return true if socket is readable, otherwise false.
     *
     */
    bool IsReadable() const { return (Value & kRead) != 0; }

    /**
     *  Check if the bit flags indicate that the socket is writable.
     *
     *  @return true if socket is writable, otherwise false.
     *
     */
    bool IsWriteable() const { return (Value & kWrite) != 0; }

    /**
     *  Check if the bit flags indicate that the socket has an error.
     *
     *  @return true if socket has an error, otherwise false.
     *
     */
    bool IsError() const { return (Value & kError) != 0; }

    /**
     *  Set the read bit flag for the socket.
     *
     */
    void SetRead() { Value |= kRead; }

    /**
     *  Set the write bit flag for the socket.
     *
     */
    void SetWrite() { Value |= kWrite; }

    /**
     *  Set the error bit flag for the socket.
     *
     */
    void SetError() { Value |= kError; }

    /**
     *  Clear the bit flags for the socket.
     *
     */
    void Clear() { Value = 0; }

    /**
     *  Clear the read bit flag for the socket.
     *
     */
    void ClearRead() { Value &= ~kRead; }

    /**
     *  Clear the write bit flag for the socket.
     *
     */
    void ClearWrite() { Value &= ~kWrite; }

    /**
     *  Clear the error bit flag for the socket.
     *
     */
    void ClearError() { Value &= ~kError; }

    void SetFDs(int socket, int & nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds);
    static SocketEvents FromFDs(int socket, fd_set * readfds, fd_set * writefds, fd_set * exceptfds);
};

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
