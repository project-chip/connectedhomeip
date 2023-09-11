/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/minimal_mdns/responders/RecordResponder.h>

namespace mdns {
namespace Minimal {

class IPv4Responder : public RecordResponder
{
public:
    IPv4Responder(const FullQName & qname) : RecordResponder(QType::A, qname) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override;
};

class IPv6Responder : public RecordResponder
{
public:
    IPv6Responder(const FullQName & qname) : RecordResponder(QType::AAAA, qname) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override;
};

} // namespace Minimal
} // namespace mdns
