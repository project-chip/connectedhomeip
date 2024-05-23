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
