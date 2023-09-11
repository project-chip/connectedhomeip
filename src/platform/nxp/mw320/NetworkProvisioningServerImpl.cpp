/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/GenericNetworkProvisioningServerImpl.cpp>
#include <platform/internal/NetworkProvisioningServer.h>

#include <core/TLV.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

NetworkProvisioningServerImpl NetworkProvisioningServerImpl::sInstance;

CHIP_ERROR NetworkProvisioningServerImpl::_Init(void)
{
    return GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>::DoInit();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
