/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

#include <inet/EndPointBasis.h>

#include <inet/InetLayer.h>

namespace chip {
namespace Inet {

void EndPointBasis::InitEndPointBasis(InetLayer & aInetLayer, void * aAppState)
{
    InitInetLayerBasis(aInetLayer, aAppState);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    mLwIPEndPointType = kLwIPEndPointType_Unknown;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    mSocket.Init(aInetLayer.SystemLayer()->WatchableEvents());
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
}

} // namespace Inet
} // namespace chip
