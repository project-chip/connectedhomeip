/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/Resolver.h>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace Controller {

/// Callbacks for CHIP device discovery
class DLL_EXPORT DeviceDiscoveryDelegate
{
public:
    virtual ~DeviceDiscoveryDelegate() {}
    virtual void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) = 0;
};

} // namespace Controller
} // namespace chip
