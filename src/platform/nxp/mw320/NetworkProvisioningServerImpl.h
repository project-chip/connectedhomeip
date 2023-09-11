/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/internal/GenericNetworkProvisioningServerImpl.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the
 * NXP MW320 platforms.
 */
class NetworkProvisioningServerImpl final : public NetworkProvisioningServer,
                                            public Internal::GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>
{
    // Allow the NetworkProvisioningServer interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ::chip::DeviceLayer::Internal::NetworkProvisioningServer;

    // Allow the GenericNetworkProvisioningServerImpl base class to access helper methods
    // and types defined on this class.
    friend class GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>;

private:
    // ===== Members that implement the NetworkProvisioningServer public interface.

    CHIP_ERROR _Init(void);

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
 * that are specific to the MW320 platform.
 */
inline NetworkProvisioningServerImpl & NetworkProvisioningSvrImpl(void)
{
    return NetworkProvisioningServerImpl::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
