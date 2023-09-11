/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

namespace chip {
namespace Tracing {

// These structures are forward-declared so that tracing itself has no direct dependencies
// on actual types. This allows tracing to be used anywhere lib/support could be used.

struct MessageSendInfo;
struct MessageReceivedInfo;
struct NodeLookupInfo;
struct NodeDiscoveredInfo;
struct NodeDiscoveryFailedInfo;

} // namespace Tracing
} // namespace chip
