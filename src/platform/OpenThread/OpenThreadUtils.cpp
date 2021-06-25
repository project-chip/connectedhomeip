/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Utility functions for working with OpenThread.
 */

#include "OpenThreadUtils.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <openthread/error.h>

#include <cstdio>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Map an OpenThread error into the OpenChip error space.
 */
CHIP_ERROR MapOpenThreadError(otError otErr)
{
    return (otErr == OT_ERROR_NONE) ? CHIP_NO_ERROR : CHIP_CONFIG_OPENTHREAD_ERROR_MIN + (CHIP_ERROR) otErr;
}

/**
 * Given an OpenChip error value that represents an OpenThread error, returns a
 * human-readable NULL-terminated C string describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not an OpenThread error.
 *
 */
bool FormatOpenThreadError(char * buf, uint16_t bufSize, int32_t err)
{
    if (err < CHIP_CONFIG_OPENTHREAD_ERROR_MIN || err > CHIP_CONFIG_OPENTHREAD_ERROR_MAX)
    {
        return false;
    }

#if CHIP_CONFIG_SHORT_ERROR_STR
    const char * desc = NULL;
#else  // CHIP_CONFIG_SHORT_ERROR_STR
    otError otErr     = (otError)(err - CHIP_CONFIG_OPENTHREAD_ERROR_MIN);
    const char * desc = otThreadErrorToString(otErr);
#endif // CHIP_CONFIG_SHORT_ERROR_STR

    chip::FormatError(buf, bufSize, "OpenThread", err, desc);

    return true;
}

/**
 * Register a text error formatter for OpenThread errors.
 */
void RegisterOpenThreadErrorFormatter(void)
{
    static ErrorFormatter sOpenThreadErrorFormatter = { FormatOpenThreadError, NULL };

    RegisterErrorFormatter(&sOpenThreadErrorFormatter);
}

/**
 * Log information related to a state change in the OpenThread stack.
 *
 * NB: This function *must* be called with the Thread stack lock held.
 */
void LogOpenThreadStateChange(otInstance * otInst, uint32_t flags)
{
#if CHIP_DETAIL_LOGGING

    const uint32_t kParamsChanged = (OT_CHANGED_THREAD_NETWORK_NAME | OT_CHANGED_THREAD_PANID | OT_CHANGED_THREAD_EXT_PANID |
                                     OT_CHANGED_THREAD_CHANNEL | OT_CHANGED_MASTER_KEY | OT_CHANGED_PSKC);

    static char strBuf[64];

    ChipLogDetail(DeviceLayer, "OpenThread State Changed (Flags: 0x%08" PRIx32 ")", flags);
    if ((flags & OT_CHANGED_THREAD_ROLE) != 0)
    {
        ChipLogDetail(DeviceLayer, "   Device Role: %s", OpenThreadRoleToStr(otThreadGetDeviceRole(otInst)));
    }
    if ((flags & kParamsChanged) != 0)
    {
        ChipLogDetail(DeviceLayer, "   Network Name: %s", otThreadGetNetworkName(otInst));
        ChipLogDetail(DeviceLayer, "   PAN Id: 0x%04X", otLinkGetPanId(otInst));
        {
            const otExtendedPanId * exPanId = otThreadGetExtendedPanId(otInst);
            snprintf(strBuf, sizeof(strBuf), "0x%02X%02X%02X%02X%02X%02X%02X%02X", exPanId->m8[0], exPanId->m8[1], exPanId->m8[2],
                     exPanId->m8[3], exPanId->m8[4], exPanId->m8[5], exPanId->m8[6], exPanId->m8[7]);
            ChipLogDetail(DeviceLayer, "   Extended PAN Id: %s", strBuf);
        }
        ChipLogDetail(DeviceLayer, "   Channel: %d", otLinkGetChannel(otInst));
        {
            const otMeshLocalPrefix * otMeshPrefix = otThreadGetMeshLocalPrefix(otInst);
            chip::Inet::IPAddress meshPrefix;
            memset(meshPrefix.Addr, 0, sizeof(meshPrefix.Addr));
            memcpy(meshPrefix.Addr, otMeshPrefix->m8, sizeof(otMeshPrefix->m8));
            meshPrefix.ToString(strBuf);
            ChipLogDetail(DeviceLayer, "   Mesh Prefix: %s/64", strBuf);
        }
#if CHIP_CONFIG_SECURITY_TEST_MODE
        {
            const otMasterKey * otKey = otThreadGetMasterKey(otInst);
            for (int i = 0; i < OT_MASTER_KEY_SIZE; i++)
                snprintf(&strBuf[i * 2], 3, "%02X", otKey->m8[i]);
            ChipLogDetail(DeviceLayer, "   Master Key: %s", strBuf);
        }
#endif // CHIP_CONFIG_SECURITY_TEST_MODE
    }
    if ((flags & OT_CHANGED_THREAD_PARTITION_ID) != 0)
    {
        ChipLogDetail(DeviceLayer, "   Partition Id: 0x%" PRIX32, otThreadGetPartitionId(otInst));
    }
    if ((flags & (OT_CHANGED_IP6_ADDRESS_ADDED | OT_CHANGED_IP6_ADDRESS_REMOVED)) != 0)
    {
        ChipLogDetail(DeviceLayer, "   Thread Unicast Addresses:");
        for (const otNetifAddress * addr = otIp6GetUnicastAddresses(otInst); addr != NULL; addr = addr->mNext)
        {
            chip::Inet::IPAddress ipAddr;
            memcpy(ipAddr.Addr, addr->mAddress.mFields.m32, sizeof(ipAddr.Addr));

            ipAddr.ToString(strBuf);

            ChipLogDetail(DeviceLayer, "        %s/%d%s%s%s", strBuf, addr->mPrefixLength, addr->mValid ? " valid" : "",
                          addr->mPreferred ? " preferred" : "", addr->mRloc ? " rloc" : "");
        }
    }

#endif // CHIP_DETAIL_LOGGING
}

void LogOpenThreadPacket(const char * titleStr, otMessage * pkt)
{
#if CHIP_DETAIL_LOGGING

    char srcStr[50], destStr[50], typeBuf[20];
    const char * type = typeBuf;
    IPAddress addr;
    uint8_t headerData[44];
    uint16_t pktLen;

    const uint8_t & IPv6_NextHeader     = headerData[6];
    const uint8_t * const IPv6_SrcAddr  = headerData + 8;
    const uint8_t * const IPv6_DestAddr = headerData + 24;
    const uint8_t * const IPv6_SrcPort  = headerData + 40;
    const uint8_t * const IPv6_DestPort = headerData + 42;
    const uint8_t & ICMPv6_Type         = headerData[40];
    const uint8_t & ICMPv6_Code         = headerData[41];

    constexpr uint8_t kIPProto_UDP    = 17;
    constexpr uint8_t kIPProto_TCP    = 6;
    constexpr uint8_t kIPProto_ICMPv6 = 58;

    constexpr uint8_t kICMPType_EchoRequest  = 128;
    constexpr uint8_t kICMPType_EchoResponse = 129;

    pktLen = otMessageGetLength(pkt);

    if (pktLen >= sizeof(headerData))
    {
        otMessageRead(pkt, 0, headerData, sizeof(headerData));

        memcpy(addr.Addr, IPv6_SrcAddr, 16);
        addr.ToString(srcStr);

        memcpy(addr.Addr, IPv6_DestAddr, 16);
        addr.ToString(destStr);

        if (IPv6_NextHeader == kIPProto_UDP)
        {
            type = "UDP";
        }
        else if (IPv6_NextHeader == kIPProto_TCP)
        {
            type = "TCP";
        }
        else if (IPv6_NextHeader == kIPProto_ICMPv6)
        {
            if (ICMPv6_Type == kICMPType_EchoRequest)
            {
                type = "ICMPv6 Echo Request";
            }
            else if (ICMPv6_Type == kICMPType_EchoResponse)
            {
                type = "ICMPv6 Echo Response";
            }
            else
            {
                snprintf(typeBuf, sizeof(typeBuf), "ICMPv6 %" PRIu8 ",%" PRIu8, ICMPv6_Type, ICMPv6_Code);
            }
        }
        else
        {
            snprintf(typeBuf, sizeof(typeBuf), "IP proto %" PRIu8, IPv6_NextHeader);
        }

        if (IPv6_NextHeader == kIPProto_UDP || IPv6_NextHeader == kIPProto_TCP)
        {
            snprintf(srcStr + strlen(srcStr), 13, ", port %" PRIu16, Encoding::BigEndian::Get16(IPv6_SrcPort));
            snprintf(destStr + strlen(destStr), 13, ", port %" PRIu16, Encoding::BigEndian::Get16(IPv6_DestPort));
        }

        ChipLogDetail(DeviceLayer, "%s: %s, len %" PRIu16, titleStr, type, pktLen);
        ChipLogDetail(DeviceLayer, "    src  %s", srcStr);
        ChipLogDetail(DeviceLayer, "    dest %s", destStr);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "%s: %s, len %" PRIu16, titleStr, "(decode error)", pktLen);
    }

#endif // CHIP_DETAIL_LOGGING
}

bool IsOpenThreadMeshLocalAddress(otInstance * otInst, const IPAddress & addr)
{
    const otMeshLocalPrefix * otMeshPrefix = otThreadGetMeshLocalPrefix(otInst);

    return otMeshPrefix != NULL && memcmp(otMeshPrefix->m8, addr.Addr, OT_MESH_LOCAL_PREFIX_SIZE) == 0;
}

const char * OpenThreadRoleToStr(otDeviceRole role)
{
    switch (role)
    {
    case OT_DEVICE_ROLE_DISABLED:
        return "DISABLED";
    case OT_DEVICE_ROLE_DETACHED:
        return "DETACHED";
    case OT_DEVICE_ROLE_CHILD:
        return "CHILD";
    case OT_DEVICE_ROLE_ROUTER:
        return "ROUTER";
    case OT_DEVICE_ROLE_LEADER:
        return "LEADER";
    default:
        return "(unknown)";
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
