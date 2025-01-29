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
#include "AddressPolicy.h"

#include <lib/support/CodeUtils.h>

namespace mdns {
namespace Minimal {

namespace {
AddressPolicy * gAddressPolicy = nullptr;
} // namespace

// This will be resolved at link time if a default policy is set
#ifndef CHIP_MINMDNS_NONE_POLICY
AddressPolicy * GetDefaultAddressPolicy();
#endif

AddressPolicy * GetAddressPolicy()
{
#ifndef CHIP_MINMDNS_NONE_POLICY
    // The GetDefaultAddressPolicy() function should be defined by a compile-defined default policy.
    if (gAddressPolicy == nullptr)
    {
        auto p = GetDefaultAddressPolicy();
        VerifyOrDie(p != nullptr);
        SetAddressPolicy(p);
    }
#endif
    return gAddressPolicy;
}

void SetAddressPolicy(AddressPolicy * policy)
{
    VerifyOrDie(policy != nullptr);
    gAddressPolicy = policy;
}

} // namespace Minimal
} // namespace mdns
