/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include "core/CHIPError.h"
#include "inet/IPAddress.h"
#include "inet/InetInterface.h"

namespace chip {
namespace Mdns {

class ScannerDelegate
{
public:
    virtual void HandleNodeResolved(CHIP_ERROR error, uint64_t nodeId, uint64_t fabricId, const Inet::IPAddress & address) = 0;
    virtual ~ScannerDelegate() {}
};

class Scanner
{
public:
    virtual ~Scanner() {}

    // Register the callback delegate to the scanner.
    virtual CHIP_ERROR RegisterScannerDelegate(ScannerDelegate * delegate) = 0;

    // Passively subscribe an operational node.
    virtual CHIP_ERROR SubscribeNode(uint64_t nodeId, uint64_t fabricId, Inet::InterfaceId interface = INET_NULL_INTERFACEID,
                                     Inet::IPAddressType addressType = Inet::kIPAddressType_Any) = 0;
    // Actively trigger a mdns query for an operational node.
    virtual CHIP_ERROR ResolveNode(uint64_t nodeId, uint64_t fabricId, Inet::InterfaceId interface = INET_NULL_INTERFACEID,
                                   Inet::IPAddressType addressType = Inet::kIPAddressType_Any) = 0;
    // Unsubscribe an operational node. Not calling this function after a node is no longer of interest
    // may cause resources to leak.
    virtual CHIP_ERROR UnsubscribeNode(uint64_t nodeId, uint64_t fabricId) = 0;

    static Scanner & Instance();
};

} // namespace Mdns
} // namespace chip
