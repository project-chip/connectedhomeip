/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#ifndef NETWORK_PROVISIONING_SERVER_IMPL_H
#define NETWORK_PROVISIONING_SERVER_IMPL_H

#include <Weave/DeviceLayer/internal/GenericNetworkProvisioningServerImpl.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the
 * Silicon Labs EFR32 platforms.
 */
class NetworkProvisioningServerImpl final
    : public NetworkProvisioningServer,
      public Internal::GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>
{
    // Allow the NetworkProvisioningServer interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class Internal::NetworkProvisioningServer;

    // Allow the GenericNetworkProvisioningServerImpl base class to access helper methods
    // and types defined on this class.
    friend class Internal::GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>;

private:
    // ===== Members that implement the NetworkProvisioningServer public interface.

    WEAVE_ERROR _Init(void);

    // ===== Members for internal use by the following friends.

    friend ::nl::Weave::DeviceLayer::Internal::NetworkProvisioningServer &NetworkProvisioningSvr(void);
    friend NetworkProvisioningServerImpl &                                NetworkProvisioningSvrImpl(void);

    static NetworkProvisioningServerImpl sInstance;
};

/**
 * Returns a reference to the public interface of the NetworkProvisioningServer singleton object.
 *
 * Internal components should use this to access features of the NetworkProvisioningServer object
 * that are common to all platforms.
 */
inline NetworkProvisioningServer &NetworkProvisioningSvr(void)
{
    return NetworkProvisioningServerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the NetworkProvisioningServer singleton object.
 *
 * Internal components can use this to gain access to features of the NetworkProvisioningServer
 * that are specific to the ESP32 platform.
 */
inline NetworkProvisioningServerImpl &NetworkProvisioningSvrImpl(void)
{
    return NetworkProvisioningServerImpl::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // NETWORK_PROVISIONING_SERVER_IMPL_H
