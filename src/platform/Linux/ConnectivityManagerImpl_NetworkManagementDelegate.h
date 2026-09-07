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

#pragma once

namespace chip {
namespace DeviceLayer {

namespace Internal {

// Forward Declarations

class NetworkManagementBasis;

/**
 *  @brief
 *    A delegate interface for coordinating network-management-driven
 *    Wi-Fi medium availability with other Connectivity Manager
 *    implementation subsystems.
 *
 *  This delegate is used by a concrete network management basis
 *  implementation to report changes in Wi-Fi medium availability to
 *  an owning coordinator (typically the platform Connectivity Manager
 *  implementation).
 *
 *  Wi-Fi medium availability reflects whether the underlying Wi-Fi
 *  control plane / radio resources are usable for Wi-Fi operations
 *  (for example, whether the Wi-Fi interface is present and powered,
 *  and not blocked or otherwise unavailable).
 *
 *  The coordinator may use these notifications to:
 *
 *    * Update cached Connectivity Manager state.
 *    * Drive Device Layer events and diagnostic attributes.
 *    * Coordinate ownership or arbitration of Wi-Fi radio resources
 *      between operational Wi-Fi usage (station / AP) and the
 *      Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized Service
 *      Discovery (USD) / Public Action Frame (PAF) commissioning
 *      transport.
 *
 *  Concrete implementations should assume that notifications may be
 *  delivered from an implementation-defined execution
 *  context. Delegates should avoid blocking for extended periods.
 *
 */
class NetworkManagementDelegate
{
public:
    virtual ~NetworkManagementDelegate(void) = default;

    /**
     *  @brief
     *    Indicate whether the Wi-Fi medium is available.
     *
     *  @param[in,out]  inOutNetworkManagement
     *    A reference to the mutable network management entity making
     *    the delegation.
     *
     *  @param[in]  inAvailable
     *    A Boolean indicating whether the Wi-Fi medium is available.
     *
     */
    virtual void OnWiFiMediumAvailable(NetworkManagementBasis & inOutNetworkManagement, bool inAvailable) = 0;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
