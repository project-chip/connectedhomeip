/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#pragma once

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>

namespace mdns {
namespace Minimal {

/// Provides a list of intefaces to listen on.
///
/// When listening on IP, both IP address type (IPv4 or IPv6) and interface id
/// are important. In particular, when using link-local IP addresses, the actual
/// interface matters (e.g. FF02::FB will care over which IPv6 interface it is sent)
///
/// For MDNS in particular, you may want:
///  - IPv4 listen on InterfaceId::Null()
///  - IPv6 listen on every specific interface id available (except local loopback and other
///    not usable interfaces like docker)
class ListenIterator
{
public:
    virtual ~ListenIterator() {}

    // Get the next interface/address type to listen on
    virtual bool Next(chip::Inet::InterfaceId * id, chip::Inet::IPAddressType * type) = 0;
};

} // namespace Minimal
} // namespace mdns
