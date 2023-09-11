/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "IPPacketInfo.h"

namespace chip {
namespace Inet {

void IPPacketInfo::Clear()
{
    SrcAddress  = IPAddress::Any;
    DestAddress = IPAddress::Any;
    Interface   = InterfaceId::Null();
    SrcPort     = 0;
    DestPort    = 0;
}

} // namespace Inet
} // namespace chip
