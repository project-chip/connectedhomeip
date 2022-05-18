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

void LogFoundOperationalSrvRecord(const chip::PeerId & peerId);
void LogFoundCommissionSrvRecord(const char * instance);

#else

inline void LogSendingQuery(const mdns::Minimal::Query & query) {}
inline void LogReceivedResource(const mdns::Minimal::ResourceData & data) {}
inline void LogFoundOperationalSrvRecord(const chip::PeerId & peerId) {}
inline void LogFoundCommissionSrvRecord(const char * instance) {}

#endif

} // namespace Logging
} // namespace Minimal
} // namespace mdns
