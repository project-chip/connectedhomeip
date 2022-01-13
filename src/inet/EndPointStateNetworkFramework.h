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
 *  Shared state for Network Framework implementations of TCPEndPoint and UDPEndPoint.
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/IPAddress.h>

#include <Network/Network.h>

namespace chip {
namespace Inet {

class DLL_EXPORT EndPointStateNetworkFramework
{
protected:
    EndPointStateNetworkFramework() {}

    nw_parameters_t mParameters;
    IPAddressType mAddrType; /**< Protocol family, i.e. IPv4 or IPv6. */
};

} // namespace Inet
} // namespace chip
