/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Utility functions for working with OpenThread.
 */

#pragma once

#include <string.h>

#include <openthread/error.h>
#include <openthread/ip6.h>
#include <openthread/thread.h>

#include <inet/IPAddress.h>
#include <inet/IPPrefix.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *  @def CHIP_CONFIG_OPENTHREAD_ERROR_MIN
 *
 *  @brief
 *      The base value for OpenThread errors when mapped into the CHIP error space.
 */
#ifndef CHIP_CONFIG_OPENTHREAD_ERROR_MIN
#define CHIP_CONFIG_OPENTHREAD_ERROR_MIN 9000000
#endif // CHIP_CONFIG_OPENTHREAD_ERROR_MIN

/**
 *  @def CHIP_CONFIG_OPENTHREAD_ERROR_MAX
 *
 *  @brief
 *      The max value for OpenThread errors when mapped into the CHIP error space.
 */
#ifndef CHIP_CONFIG_OPENTHREAD_ERROR_MAX
#define CHIP_CONFIG_OPENTHREAD_ERROR_MAX 9000999
#endif // CHIP_CONFIG_OPENTHREAD_ERROR_MAX

extern CHIP_ERROR MapOpenThreadError(otError otErr);
extern void RegisterOpenThreadErrorFormatter(void);
extern void LogOpenThreadStateChange(otInstance * otInst, uint32_t flags);
extern void LogOpenThreadPacket(const char * titleStr, otMessage * pkt);
extern bool IsOpenThreadMeshLocalAddress(otInstance * otInst, const Inet::IPAddress & addr);
extern const char * OpenThreadRoleToStr(otDeviceRole role);

inline otIp6Address ToOpenThreadIP6Address(const Inet::IPAddress & addr)
{
    otIp6Address otAddr;
    memcpy(otAddr.mFields.m32, addr.Addr, sizeof(otAddr.mFields.m32));
    return otAddr;
}

inline Inet::IPAddress ToIPAddress(const otIp6Address & otAddr)
{
    Inet::IPAddress addr;
    memcpy(addr.Addr, otAddr.mFields.m32, sizeof(addr.Addr));
    return addr;
}

inline Inet::IPPrefix ToIPPrefix(const otIp6Prefix & otPrefix)
{
    Inet::IPPrefix prefix;
    prefix.IPAddr = ToIPAddress(otPrefix.mPrefix);
    prefix.Length = otPrefix.mLength;
    return prefix;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
