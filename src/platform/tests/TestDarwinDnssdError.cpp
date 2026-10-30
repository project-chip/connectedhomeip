/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <platform/Darwin/dnssd/DnssdError.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Dnssd::Error;

TEST(TestDarwinDnssdError, NonexistentNameMapsToNxdomain)
{
    EXPECT_EQ(ToChipError(kDNSServiceErr_NoSuchName), CHIP_ERROR_DNS_SD_NXDOMAIN);
    EXPECT_EQ(ToChipError(kDNSServiceErr_NoSuchRecord), CHIP_ERROR_DNS_SD_NXDOMAIN);
}

TEST(TestDarwinDnssdError, OtherErrorsUnchanged)
{
    EXPECT_EQ(ToChipError(kDNSServiceErr_NoError), CHIP_NO_ERROR);
    EXPECT_EQ(ToChipError(kDNSServiceErr_NoAuth), CHIP_ERROR_DNS_SD_UNAUTHORIZED);
    EXPECT_EQ(ToChipError(kDNSServiceErr_Unknown), CHIP_ERROR_INTERNAL);
}
