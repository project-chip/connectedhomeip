/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
