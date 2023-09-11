/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <inet/IPAddress.h>
#include <lib/core/PeerId.h>
#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/Query.h>

namespace mdns {
namespace Minimal {

/// Provides potentially verbose logging for DNSSD operations when using
/// MinMdns.
namespace Logging {

#if CHIP_MINMDNS_HIGH_VERBOSITY

void LogSendingQuery(const mdns::Minimal::Query & query);
void LogReceivedResource(const mdns::Minimal::ResourceData & data);
void LogFoundOperationalSrvRecord(const chip::PeerId & peerId, const mdns::Minimal::SerializedQNameIterator & targetHost);
void LogFoundCommissionSrvRecord(const char * instance, const mdns::Minimal::SerializedQNameIterator & targetHost);
void LogFoundIPAddress(const mdns::Minimal::SerializedQNameIterator & targetHost, const chip::Inet::IPAddress & addr);

#else

inline void LogSendingQuery(const mdns::Minimal::Query & query) {}
inline void LogReceivedResource(const mdns::Minimal::ResourceData & data) {}
inline void LogFoundOperationalSrvRecord(const chip::PeerId & peerId, const mdns::Minimal::SerializedQNameIterator & targetHost) {}
inline void LogFoundCommissionSrvRecord(const char * instance, const mdns::Minimal::SerializedQNameIterator & targetHost) {}
inline void LogFoundIPAddress(const mdns::Minimal::SerializedQNameIterator & targetHost, const chip::Inet::IPAddress & addr) {}

#endif

} // namespace Logging
} // namespace Minimal
} // namespace mdns
