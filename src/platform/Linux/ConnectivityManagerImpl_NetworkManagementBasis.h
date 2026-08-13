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

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Forward Declarations

class NetworkManagementDelegate;

/**
 *  @brief
 *    A lightweight base class providing delegate wiring for Linux
 *    Connectivity Manager network management implementations.
 *
 *  This serves as a common base for concrete Linux Connectivity
 *  Manager network management backends (for example, Connection
 *  Manager- (also known as, connman) or wpa_supplicant-based
 *  management).
 *
 *  The class provides:
 *
 *    * Storage and management of an optional network management
 *      delegate.
 *    * A protected action-delegation helper for reporting Wi-Fi
 *      medium availability changes to the delegate.
 *
 *  This base does not define the full network management
 *  control-plane API. Instead, it provides a small amount of shared
 *  "glue" used to coordinate cross-cutting concerns between the
 *  network management backend and the owning platform Connectivity
 *  Manager implementation (which may also be coordinating the Wi-Fi
 *  NAN USD / PAF commissioning transport).
 *
 *  @note
 *    The delegate is not owned by this class. Callers must ensure the
 *    delegate remains valid for as long as it is set.
 *
 *  @note
 *    This type is intended to be used as a mix-in base alongside a
 *    concrete network management interface implementation.
 *
 */
class NetworkManagementBasis
{
public:
    virtual ~NetworkManagementBasis() = default;

    /**
     *  @brief
     *    Perform explicit class initialization.
     *
     *  Initializes internal state used by the base class, including
     *  clearing any previously-set delegate.
     *
     *  Concrete derived classes should invoke this during their own
     *  initialization (typically before establishing subscriptions or
     *  starting external network management services).
     *
     */
    CHIP_ERROR Init() noexcept;

    /**
     *  @brief
     *    Set the delegate to receive coordination callbacks.
     *
     *  Sets (or clears) the network management delegate that will be
     *  notified of relevant state changes observed by the network
     *  management backend.
     *
     *  @note
     *    The delegate is not owned by this object.
     *
     *  @param[in]  inNetworkManagementDelegate
     *    A pointer to the delegate to set, or null to clear the
     *    delegate.
     *
     *  @sa OnWiFiMediumAvailable
     *
     */
    void SetDelegate(NetworkManagementDelegate * inNetworkManagementDelegate) noexcept;

protected:
    /**
     *  @brief
     *    Notify the delegate of Wi-Fi medium availability changes.
     *
     *  Derived classes should invoke this helper when the underlying
     *  Wi-Fi control plane indicates that the Wi-Fi medium (radio /
     *  interface resources) has become available or unavailable.
     *
     *  If a delegate is set, this forwards the notification to
     *  NetworkManagementDelegate::OnWiFiMediumAvailable.
     *
     *  @param[in]  inAvailable
     *    A Boolean indicating whether the Wi-Fi medium is available.
     *
     *  @sa SetDelegate
     *
     */
    void OnWiFiMediumAvailable(bool inAvailable) noexcept;

private:
    NetworkManagementDelegate * mDelegate;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
