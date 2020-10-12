/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <platform/internal/GenericNetworkProvisioningServerImpl.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the ESP32 platform.
 */
class NetworkProvisioningServerImpl final : public NetworkProvisioningServer,
                                            public GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>
{
private:
    using GenericImplClass = GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>;

    // Allow the NetworkProvisioningServer interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ::chip::DeviceLayer::Internal::NetworkProvisioningServer;

    // Allow the GenericNetworkProvisioningServerImpl base class to access helper methods
    // and types defined on this class.
    friend class GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>;

    // ===== Members that implement the NetworkProvisioningServer public interface.

    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

    // NOTE: Other public interface methods are implemented by GenericNetworkProvisioningServerImpl<>.

    // ===== Members used by GenericNetworkProvisioningServerImpl<> to invoke platform-specific
    //       operations.

    CHIP_ERROR GetWiFiStationProvision(NetworkInfo & netInfo, bool includeCredentials);
    CHIP_ERROR SetWiFiStationProvision(const NetworkInfo & netInfo);
    CHIP_ERROR ClearWiFiStationProvision(void);
    CHIP_ERROR InitiateWiFiScan(void);
    void HandleScanDone(void);
    static NetworkProvisioningServerImpl & Instance(void);
    static void HandleScanTimeOut(::chip::System::Layer * aLayer, void * aAppState, ::chip::System::Error aError);
    static bool IsSupportedWiFiSecurityType(WiFiSecurityType_t wifiSecType);

    // ===== Members for internal use by the following friends.

    friend ::chip::DeviceLayer::Internal::NetworkProvisioningServer & NetworkProvisioningSvr(void);
    friend NetworkProvisioningServerImpl & NetworkProvisioningSvrImpl(void);

    static NetworkProvisioningServerImpl sInstance;
};

/**
 * Returns a reference to the public interface of the NetworkProvisioningServer singleton object.
 *
 * Internal components should use this to access features of the NetworkProvisioningServer object
 * that are common to all platforms.
 */
inline NetworkProvisioningServer & NetworkProvisioningSvr(void)
{
    return NetworkProvisioningServerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the NetworkProvisioningServer singleton object.
 *
 * Internal components can use this to gain access to features of the NetworkProvisioningServer
 * that are specific to the ESP32 platform.
 */
inline NetworkProvisioningServerImpl & NetworkProvisioningSvrImpl(void)
{
    return NetworkProvisioningServerImpl::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
