/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/server-cluster/OptionalAttributeSet.h>
#include <lib/core/DataModelTypes.h>
#include <pw_unit_test/framework.h>

namespace chip::app {

using namespace chip::app;

namespace {

TEST(TestOptionalAttributeSet, TestAttributeSet)
{
    AttributeSet set;

    EXPECT_FALSE(set.IsSet(1));
    EXPECT_FALSE(set.IsSet(2));
    EXPECT_FALSE(set.IsSet(3));

    set.ForceSet<1>();
    EXPECT_TRUE(set.IsSet(1));
    EXPECT_FALSE(set.IsSet(2));
    EXPECT_FALSE(set.IsSet(3));

    set.ForceSet<3>();
    EXPECT_TRUE(set.IsSet(1));
    EXPECT_FALSE(set.IsSet(2));
    EXPECT_TRUE(set.IsSet(3));
}

TEST(TestOptionalAttributeSet, TestOptionalAttributeSet)
{
    using Supported = OptionalAttributeSet<1, 3, 5, 7>;

    Supported supported;

    EXPECT_FALSE(supported.IsSet(1));
    EXPECT_FALSE(supported.IsSet(3));
    EXPECT_FALSE(supported.IsSet(5));
    EXPECT_FALSE(supported.IsSet(7));

    supported.Set<1>();
    EXPECT_TRUE(supported.IsSet(1));

    supported.Set<5>();
    EXPECT_TRUE(supported.IsSet(5));

    // These would not compile
    // supported.Set<2>();
    // supported.Set<4>();

    // We can check unsupported flags
    EXPECT_FALSE(supported.IsSet(2));
    EXPECT_FALSE(supported.IsSet(4));
}

TEST(TestOptionalAttributeSet, TestEmptyOptionalAttributeSet)
{
    using Supported = OptionalAttributeSet<>;

    Supported supported;

    // can query attributes that are not supported
    EXPECT_FALSE(supported.IsSet(1));
    EXPECT_FALSE(supported.IsSet(2));

    // These would not compile
    // supported.Set<1>();
    // supported.Set<2>();
}

TEST(TestOptionalAttributeSet, TestBitInitAndRestrictions)
{
    {
        using Supported = OptionalAttributeSet<>;
        Supported supported(0xFF);

        // No bits can be set because no bits are supported
        EXPECT_FALSE(supported.IsSet(1));
        EXPECT_FALSE(supported.IsSet(2));
    }

    {
        using Supported = OptionalAttributeSet<1>;
        Supported supported(0xFF);

        // Only bit 1 can be set
        EXPECT_TRUE(supported.IsSet(1));
        EXPECT_FALSE(supported.IsSet(2));
    }

    {
        using Supported = OptionalAttributeSet<1, 3, 5>;
        Supported supported(0x9);

        // Only bits 1, 3, and 5 can be set. However, the constructor
        // explicitly sets 0xD == 0b1101, so bit 3 is set (and bit 2
        // and bit 0 are not allowed to be set).
        EXPECT_FALSE(supported.IsSet(0));
        EXPECT_FALSE(supported.IsSet(1));
        EXPECT_FALSE(supported.IsSet(2));
        EXPECT_TRUE(supported.IsSet(3));
        EXPECT_FALSE(supported.IsSet(4));
        EXPECT_FALSE(supported.IsSet(5));
    }
}

} // namespace
} // namespace chip::app
