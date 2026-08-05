/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <pw_unit_test/framework.h>

#include <lib/support/Span.h>

#include "../../include/TargetVideoPlayerInfo.h"

#include <cstring>

using namespace chip;

namespace {

// SetMACAddress() copies sizeof(mMACAddressBuf) (== 2 * kPrimaryMACAddressLength, at most 16) bytes
// from the input regardless of the passed span length, so the seeds must be at least that long.
constexpr char kMacA[] = "AABBCCDDEEFF0011223344556677";
constexpr char kMacB[] = "ffffffffffffffffffffffffffff";

// Before the rule-of-three fix, the implicitly-defaulted copy duplicated mMACAddress's {ptr,len}
// verbatim, so the copy's span aliased the SOURCE's mMACAddressBuf. These assert the copy's span is
// re-bound into its OWN buffer (independent of the source).

TEST(TestTargetVideoPlayerInfoCopy, CopyAssignReBindsMacSpanToOwnBuffer)
{
    TargetVideoPlayerInfo source;
    source.SetMACAddress(CharSpan(kMacA, strlen(kMacA)));

    TargetVideoPlayerInfo copy;
    copy = source;

    // The copy's MAC span must point into the copy's own buffer, not the source's.
    EXPECT_NE(copy.GetMACAddress()->data(), source.GetMACAddress()->data());

    // ...and it must keep its own bytes after the source's buffer is overwritten.
    const size_t macLen = copy.GetMACAddress()->size();
    source.SetMACAddress(CharSpan(kMacB, strlen(kMacB)));
    EXPECT_EQ(0, memcmp(copy.GetMACAddress()->data(), kMacA, macLen));
}

TEST(TestTargetVideoPlayerInfoCopy, CopyConstructReBindsMacSpanToOwnBuffer)
{
    TargetVideoPlayerInfo source;
    source.SetMACAddress(CharSpan(kMacA, strlen(kMacA)));

    TargetVideoPlayerInfo copy(source);

    EXPECT_NE(copy.GetMACAddress()->data(), source.GetMACAddress()->data());

    const size_t macLen = copy.GetMACAddress()->size();
    source.SetMACAddress(CharSpan(kMacB, strlen(kMacB)));
    EXPECT_EQ(0, memcmp(copy.GetMACAddress()->data(), kMacA, macLen));
}

} // namespace
