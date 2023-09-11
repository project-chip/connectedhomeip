/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *  Shared state for LwIP implementations of TCPEndPoint and UDPEndPoint.
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/IPAddress.h>

struct udp_pcb;
struct tcp_pcb;

namespace chip {
namespace Inet {

/**
 * Definitions shared by all LwIP EndPoint classes.
 */
class DLL_EXPORT EndPointStateLwIP
{
protected:
    EndPointStateLwIP() : mLwIPEndPointType(LwIPEndPointType::Unknown) {}

    enum class LwIPEndPointType : uint8_t
    {
        Unknown = 0,
        UDP     = 1,
        TCP     = 2
    } mLwIPEndPointType;
};

} // namespace Inet
} // namespace chip
