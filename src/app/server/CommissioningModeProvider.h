/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/Advertiser.h>

namespace chip {
namespace app {

/**
 * A way to ask what commissioning mode DnssdServer should be advertising.  This
 * needs to match the actual commissioning mode of the device (i.e. whether the
 * device will accept an attempt to establish a PASE session).
 */
class DLL_EXPORT CommissioningModeProvider
{
public:
    virtual Dnssd::CommissioningMode GetCommissioningMode() const = 0;

    virtual ~CommissioningModeProvider() {}
};

} // namespace app
} // namespace chip
