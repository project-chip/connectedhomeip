/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "AddressPolicy.h"

#include <lib/support/CodeUtils.h>

namespace mdns {
namespace Minimal {

namespace {
AddressPolicy * gAddressPolicy = nullptr;
} // namespace

AddressPolicy * GetAddressPolicy()
{
    VerifyOrDie(gAddressPolicy != nullptr);
    return gAddressPolicy;
}

void SetAddressPolicy(AddressPolicy * policy)
{
    VerifyOrDie(policy != nullptr);
    gAddressPolicy = policy;
}

} // namespace Minimal
} // namespace mdns
