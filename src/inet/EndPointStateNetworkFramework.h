/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *  Shared state for Network Framework implementations of TCPEndPoint and UDPEndPoint.
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/IPAddress.h>

#include <Network/Network.h>

namespace chip {
namespace Inet {

class DLL_EXPORT EndPointStateNetworkFramework
{
protected:
    EndPointStateNetworkFramework() {}

    nw_parameters_t mParameters;
    IPAddressType mAddrType; /**< Protocol family, i.e. IPv4 or IPv6. */
};

} // namespace Inet
} // namespace chip
