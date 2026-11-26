/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *          bluetooth2 GATT client calls that the wbs layer (wbs/, kept unmodified) does not provide,
 *          used by the webOS BLEManagerImpl to work around wbs connection handling.
 */

#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct WbsEndpoint;

// Synchronous LS2 calls. Must not be called on the LsRequester (lsTask) thread, which dispatches the replies.

/// gatt/connect with a custom timeout. Returns the WBS client id, or an empty string on failure.
std::string WbsGattConnect(const std::string & address, int timeoutSec);

/// gatt/disconnect of a WBS client.
void WbsGattDisconnect(const std::string & clientId);

/// Logs gatt/getStatus of a peer (diagnostic only).
void WbsGattLogStatus(const std::string & address);

/// gatt/disconnect of every WBS client in the wbs connection table (run on the Matter GLib context).
void WbsGattDisconnectAll(WbsEndpoint * endpoint);

/// WBS clients opened by WbsGattConnect() (not tracked by the wbs layer). Thread-safe.
class WbsPreconnectClients
{
public:
    void Add(const std::string & clientId);
    /// gatt/disconnect of every registered client.
    void ReleaseAll();

private:
    std::mutex mMutex;
    std::vector<std::string> mClientIds;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
