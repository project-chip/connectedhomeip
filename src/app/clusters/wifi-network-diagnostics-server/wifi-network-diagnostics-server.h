/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

class WiFiDiagnosticsServer : public DeviceLayer::WiFiDiagnosticsDelegate
{
public:
    static WiFiDiagnosticsServer & Instance();

    // Gets called when the Node detects Node’s Wi-Fi connection has been disconnected.
    void OnDisconnectionDetected(uint16_t reasonCode) override;

    // Gets called when the Node fails to associate or authenticate an access point.
    void OnAssociationFailureDetected(uint8_t associationFailureCause, uint16_t status) override;

    // Gets when the Node’s connection status to a Wi-Fi network has changed.
    void OnConnectionStatusChanged(uint8_t connectionStatus) override;

private:
    static WiFiDiagnosticsServer instance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
