/*
 *
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file contains the external implementations of methods in
 *      the basis class for all the various transport endpoint classes
 *      in the Inet layer, i.e. TCP, UDP, Raw and Tun.
 */

#include <InetLayer/EndPointBasis.h>

#include <InetLayer/InetLayer.h>

namespace nl {
namespace Inet {

void EndPointBasis::InitEndPointBasis(InetLayer& aInetLayer, void* aAppState)
{
    InitInetLayerBasis(aInetLayer, aAppState);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    mLwIPEndPointType = kLwIPEndPointType_Unknown;
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    mSocket = INET_INVALID_SOCKET_FD;
    mPendingIO.Clear();
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
}

} // namespace Inet
} // namespace nl
