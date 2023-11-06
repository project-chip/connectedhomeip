/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>

namespace mdns {
namespace Minimal {

class IpAddressIterator
{
public:
    virtual ~IpAddressIterator() = default;

    /// Fetch the next IP address in the iterator.
    ///
    /// Returns true if an IP address is available, false if the iteration is complete.
    virtual bool Next(chip::Inet::IPAddress & dest) = 0;
};

} // namespace Minimal
} // namespace mdns
