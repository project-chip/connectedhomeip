/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the CHIP Device Network Provisioning object.
 *
 */

#pragma once

#include <lib/core/CHIPCore.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class DLL_EXPORT DeviceNetworkProvisioningDelegate
{
public:
    /**
     * @brief
     *   Called to provision WiFi credentials in a device
     *
     * @param ssid WiFi SSID
     * @param passwd WiFi password
     */
    virtual CHIP_ERROR ProvisionWiFi(const char * ssid, const char * passwd) = 0;

    /**
     * @brief
     *   Called to provision Thread credentials in a device
     *
     */
    virtual CHIP_ERROR ProvisionThread(ByteSpan threadData) = 0;

    virtual ~DeviceNetworkProvisioningDelegate() {}
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
