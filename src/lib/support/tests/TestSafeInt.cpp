/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a unit test suite for CHIP SafeInt functions
 *
 */

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/SafeInt.h>

using namespace chip;

TEST(TestSafeInt, TestCanCastTo_Int8)
{
    EXPECT_TRUE(CanCastTo<uint8_t>(0));
    EXPECT_TRUE(CanCastTo<uint8_t>(127));
    EXPECT_TRUE(CanCastTo<uint8_t>(128));
    EXPECT_TRUE(CanCastTo<uint8_t>(129));
    EXPECT_TRUE(CanCastTo<uint8_t>(255));
    EXPECT_FALSE(CanCastTo<uint8_t>(256));
    EXPECT_FALSE(CanCastTo<uint8_t>(32767));
    EXPECT_FALSE(CanCastTo<uint8_t>(32768));
    EXPECT_FALSE(CanCastTo<uint8_t>(32769));
    EXPECT_FALSE(CanCastTo<uint8_t>(65535));
    EXPECT_FALSE(CanCastTo<uint8_t>(65536));
    EXPECT_FALSE(CanCastTo<uint8_t>(2147483647ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(2147483648ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(2147483649ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(4294967295ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(4294967296ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<uint8_t>(9223372036854775808ull));

    EXPECT_FALSE(CanCastTo<uint8_t>(-1));
    EXPECT_FALSE(CanCastTo<uint8_t>(-127));
    EXPECT_FALSE(CanCastTo<uint8_t>(-128));
    EXPECT_FALSE(CanCastTo<uint8_t>(-129));
    EXPECT_FALSE(CanCastTo<uint8_t>(-255));
    EXPECT_FALSE(CanCastTo<uint8_t>(-256));
    EXPECT_FALSE(CanCastTo<uint8_t>(-32767));
    EXPECT_FALSE(CanCastTo<uint8_t>(-32768));
    EXPECT_FALSE(CanCastTo<uint8_t>(-32769));
    EXPECT_FALSE(CanCastTo<uint8_t>(-65535));
    EXPECT_FALSE(CanCastTo<uint8_t>(-65536));
    EXPECT_FALSE(CanCastTo<uint8_t>(-2147483647ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<uint8_t>(-9223372036854775807ll));

    EXPECT_TRUE(CanCastTo<int8_t>(0));
    EXPECT_TRUE(CanCastTo<int8_t>(127));
    EXPECT_FALSE(CanCastTo<int8_t>(128));
    EXPECT_FALSE(CanCastTo<int8_t>(129));
    EXPECT_FALSE(CanCastTo<int8_t>(255));
    EXPECT_FALSE(CanCastTo<int8_t>(256));
    EXPECT_FALSE(CanCastTo<int8_t>(32767));
    EXPECT_FALSE(CanCastTo<int8_t>(32768));
    EXPECT_FALSE(CanCastTo<int8_t>(32769));
    EXPECT_FALSE(CanCastTo<int8_t>(65535));
    EXPECT_FALSE(CanCastTo<int8_t>(65536));
    EXPECT_FALSE(CanCastTo<int8_t>(2147483647ll));
    EXPECT_FALSE(CanCastTo<int8_t>(2147483648ll));
    EXPECT_FALSE(CanCastTo<int8_t>(2147483649ll));
    EXPECT_FALSE(CanCastTo<int8_t>(4294967295ll));
    EXPECT_FALSE(CanCastTo<int8_t>(4294967296ll));
    EXPECT_FALSE(CanCastTo<int8_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<int8_t>(9223372036854775808ull));

    EXPECT_TRUE(CanCastTo<int8_t>(-1));
    EXPECT_TRUE(CanCastTo<int8_t>(-127));
    EXPECT_TRUE(CanCastTo<int8_t>(-128));
    EXPECT_FALSE(CanCastTo<int8_t>(-129));
    EXPECT_FALSE(CanCastTo<int8_t>(-255));
    EXPECT_FALSE(CanCastTo<int8_t>(-256));
    EXPECT_FALSE(CanCastTo<int8_t>(-32767));
    EXPECT_FALSE(CanCastTo<int8_t>(-32768));
    EXPECT_FALSE(CanCastTo<int8_t>(-32769));
    EXPECT_FALSE(CanCastTo<int8_t>(-65535));
    EXPECT_FALSE(CanCastTo<int8_t>(-65536));
    EXPECT_FALSE(CanCastTo<int8_t>(-2147483647ll));
    EXPECT_FALSE(CanCastTo<int8_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<int8_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<int8_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<int8_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<int8_t>(-9223372036854775807ll));
}

TEST(TestSafeInt, TestCanCastTo_Int16)
{
    EXPECT_TRUE(CanCastTo<uint16_t>(0));
    EXPECT_TRUE(CanCastTo<uint16_t>(127));
    EXPECT_TRUE(CanCastTo<uint16_t>(128));
    EXPECT_TRUE(CanCastTo<uint16_t>(129));
    EXPECT_TRUE(CanCastTo<uint16_t>(255));
    EXPECT_TRUE(CanCastTo<uint16_t>(256));
    EXPECT_TRUE(CanCastTo<uint16_t>(32767));
    EXPECT_TRUE(CanCastTo<uint16_t>(32768));
    EXPECT_TRUE(CanCastTo<uint16_t>(32769));
    EXPECT_TRUE(CanCastTo<uint16_t>(65535));
    EXPECT_FALSE(CanCastTo<uint16_t>(65536));
    EXPECT_FALSE(CanCastTo<uint16_t>(2147483647ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(2147483648ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(2147483649ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(4294967295ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(4294967296ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<uint16_t>(9223372036854775808ull));

    EXPECT_FALSE(CanCastTo<uint16_t>(-1));
    EXPECT_FALSE(CanCastTo<uint16_t>(-127));
    EXPECT_FALSE(CanCastTo<uint16_t>(-128));
    EXPECT_FALSE(CanCastTo<uint16_t>(-129));
    EXPECT_FALSE(CanCastTo<uint16_t>(-255));
    EXPECT_FALSE(CanCastTo<uint16_t>(-256));
    EXPECT_FALSE(CanCastTo<uint16_t>(-32767));
    EXPECT_FALSE(CanCastTo<uint16_t>(-32768));
    EXPECT_FALSE(CanCastTo<uint16_t>(-32769));
    EXPECT_FALSE(CanCastTo<uint16_t>(-65535));
    EXPECT_FALSE(CanCastTo<uint16_t>(-65536));
    EXPECT_FALSE(CanCastTo<uint16_t>(-2147483647ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<uint16_t>(-9223372036854775807ll));

    EXPECT_TRUE(CanCastTo<int16_t>(0));
    EXPECT_TRUE(CanCastTo<int16_t>(127));
    EXPECT_TRUE(CanCastTo<int16_t>(128));
    EXPECT_TRUE(CanCastTo<int16_t>(129));
    EXPECT_TRUE(CanCastTo<int16_t>(255));
    EXPECT_TRUE(CanCastTo<int16_t>(256));
    EXPECT_TRUE(CanCastTo<int16_t>(32767));
    EXPECT_FALSE(CanCastTo<int16_t>(32768));
    EXPECT_FALSE(CanCastTo<int16_t>(32769));
    EXPECT_FALSE(CanCastTo<int16_t>(65535));
    EXPECT_FALSE(CanCastTo<int16_t>(65536));
    EXPECT_FALSE(CanCastTo<int16_t>(2147483647ll));
    EXPECT_FALSE(CanCastTo<int16_t>(2147483648ll));
    EXPECT_FALSE(CanCastTo<int16_t>(2147483649ll));
    EXPECT_FALSE(CanCastTo<int16_t>(4294967295ll));
    EXPECT_FALSE(CanCastTo<int16_t>(4294967296ll));
    EXPECT_FALSE(CanCastTo<int16_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<int16_t>(9223372036854775808ull));

    EXPECT_TRUE(CanCastTo<int16_t>(-1));
    EXPECT_TRUE(CanCastTo<int16_t>(-127));
    EXPECT_TRUE(CanCastTo<int16_t>(-128));
    EXPECT_TRUE(CanCastTo<int16_t>(-129));
    EXPECT_TRUE(CanCastTo<int16_t>(-255));
    EXPECT_TRUE(CanCastTo<int16_t>(-256));
    EXPECT_TRUE(CanCastTo<int16_t>(-32767));
    EXPECT_TRUE(CanCastTo<int16_t>(-32768));
    EXPECT_FALSE(CanCastTo<int16_t>(-32769));
    EXPECT_FALSE(CanCastTo<int16_t>(-65535));
    EXPECT_FALSE(CanCastTo<int16_t>(-65536));
    EXPECT_FALSE(CanCastTo<int16_t>(-2147483647ll));
    EXPECT_FALSE(CanCastTo<int16_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<int16_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<int16_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<int16_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<int16_t>(-9223372036854775807ll));
}

TEST(TestSafeInt, TestCanCastTo_Int32)
{
    EXPECT_TRUE(CanCastTo<uint32_t>(0));
    EXPECT_TRUE(CanCastTo<uint32_t>(127));
    EXPECT_TRUE(CanCastTo<uint32_t>(128));
    EXPECT_TRUE(CanCastTo<uint32_t>(129));
    EXPECT_TRUE(CanCastTo<uint32_t>(255));
    EXPECT_TRUE(CanCastTo<uint32_t>(256));
    EXPECT_TRUE(CanCastTo<uint32_t>(32767));
    EXPECT_TRUE(CanCastTo<uint32_t>(32768));
    EXPECT_TRUE(CanCastTo<uint32_t>(32769));
    EXPECT_TRUE(CanCastTo<uint32_t>(65535));
    EXPECT_TRUE(CanCastTo<uint32_t>(65536));
    EXPECT_TRUE(CanCastTo<uint32_t>(2147483647ll));
    EXPECT_TRUE(CanCastTo<uint32_t>(2147483648ll));
    EXPECT_TRUE(CanCastTo<uint32_t>(2147483649ll));
    EXPECT_TRUE(CanCastTo<uint32_t>(4294967295ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(4294967296ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<uint32_t>(9223372036854775808ull));

    EXPECT_FALSE(CanCastTo<uint32_t>(-1));
    EXPECT_FALSE(CanCastTo<uint32_t>(-127));
    EXPECT_FALSE(CanCastTo<uint32_t>(-128));
    EXPECT_FALSE(CanCastTo<uint32_t>(-129));
    EXPECT_FALSE(CanCastTo<uint32_t>(-255));
    EXPECT_FALSE(CanCastTo<uint32_t>(-256));
    EXPECT_FALSE(CanCastTo<uint32_t>(-32767));
    EXPECT_FALSE(CanCastTo<uint32_t>(-32768));
    EXPECT_FALSE(CanCastTo<uint32_t>(-32769));
    EXPECT_FALSE(CanCastTo<uint32_t>(-65535));
    EXPECT_FALSE(CanCastTo<uint32_t>(-65536));
    EXPECT_FALSE(CanCastTo<uint32_t>(-2147483647ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<uint32_t>(-9223372036854775807ll));

    EXPECT_TRUE(CanCastTo<int32_t>(0));
    EXPECT_TRUE(CanCastTo<int32_t>(127));
    EXPECT_TRUE(CanCastTo<int32_t>(128));
    EXPECT_TRUE(CanCastTo<int32_t>(129));
    EXPECT_TRUE(CanCastTo<int32_t>(255));
    EXPECT_TRUE(CanCastTo<int32_t>(256));
    EXPECT_TRUE(CanCastTo<int32_t>(32767));
    EXPECT_TRUE(CanCastTo<int32_t>(32768));
    EXPECT_TRUE(CanCastTo<int32_t>(32769));
    EXPECT_TRUE(CanCastTo<int32_t>(65535));
    EXPECT_TRUE(CanCastTo<int32_t>(65536));
    EXPECT_TRUE(CanCastTo<int32_t>(2147483647ll));
    EXPECT_FALSE(CanCastTo<int32_t>(2147483648ll));
    EXPECT_FALSE(CanCastTo<int32_t>(2147483649ll));
    EXPECT_FALSE(CanCastTo<int32_t>(4294967295ll));
    EXPECT_FALSE(CanCastTo<int32_t>(4294967296ll));
    EXPECT_FALSE(CanCastTo<int32_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<int32_t>(9223372036854775808ull));

    EXPECT_TRUE(CanCastTo<int32_t>(-1));
    EXPECT_TRUE(CanCastTo<int32_t>(-127));
    EXPECT_TRUE(CanCastTo<int32_t>(-128));
    EXPECT_TRUE(CanCastTo<int32_t>(-129));
    EXPECT_TRUE(CanCastTo<int32_t>(-255));
    EXPECT_TRUE(CanCastTo<int32_t>(-256));
    EXPECT_TRUE(CanCastTo<int32_t>(-32767));
    EXPECT_TRUE(CanCastTo<int32_t>(-32768));
    EXPECT_TRUE(CanCastTo<int32_t>(-32769));
    EXPECT_TRUE(CanCastTo<int32_t>(-65535));
    EXPECT_TRUE(CanCastTo<int32_t>(-65536));
    EXPECT_TRUE(CanCastTo<int32_t>(-2147483647ll));
    EXPECT_TRUE(CanCastTo<int32_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<int32_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<int32_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<int32_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<int32_t>(-9223372036854775807ll));
}

TEST(TestSafeInt, TestCanCastTo_Int64)
{
    EXPECT_TRUE(CanCastTo<uint64_t>(0));
    EXPECT_TRUE(CanCastTo<uint64_t>(127));
    EXPECT_TRUE(CanCastTo<uint64_t>(128));
    EXPECT_TRUE(CanCastTo<uint64_t>(129));
    EXPECT_TRUE(CanCastTo<uint64_t>(255));
    EXPECT_TRUE(CanCastTo<uint64_t>(256));
    EXPECT_TRUE(CanCastTo<uint64_t>(32767));
    EXPECT_TRUE(CanCastTo<uint64_t>(32768));
    EXPECT_TRUE(CanCastTo<uint64_t>(32769));
    EXPECT_TRUE(CanCastTo<uint64_t>(65535));
    EXPECT_TRUE(CanCastTo<uint64_t>(65536));
    EXPECT_TRUE(CanCastTo<uint64_t>(2147483647ll));
    EXPECT_TRUE(CanCastTo<uint64_t>(2147483648ll));
    EXPECT_TRUE(CanCastTo<uint64_t>(2147483649ll));
    EXPECT_TRUE(CanCastTo<uint64_t>(4294967295ll));
    EXPECT_TRUE(CanCastTo<uint64_t>(4294967296ll));
    EXPECT_TRUE(CanCastTo<uint64_t>(9223372036854775807ull));
    EXPECT_TRUE(CanCastTo<uint64_t>(9223372036854775808ull));

    EXPECT_FALSE(CanCastTo<uint64_t>(-1));
    EXPECT_FALSE(CanCastTo<uint64_t>(-127));
    EXPECT_FALSE(CanCastTo<uint64_t>(-128));
    EXPECT_FALSE(CanCastTo<uint64_t>(-129));
    EXPECT_FALSE(CanCastTo<uint64_t>(-255));
    EXPECT_FALSE(CanCastTo<uint64_t>(-256));
    EXPECT_FALSE(CanCastTo<uint64_t>(-32767));
    EXPECT_FALSE(CanCastTo<uint64_t>(-32768));
    EXPECT_FALSE(CanCastTo<uint64_t>(-32769));
    EXPECT_FALSE(CanCastTo<uint64_t>(-65535));
    EXPECT_FALSE(CanCastTo<uint64_t>(-65536));
    EXPECT_FALSE(CanCastTo<uint64_t>(-2147483647ll));
    EXPECT_FALSE(CanCastTo<uint64_t>(-2147483648ll));
    EXPECT_FALSE(CanCastTo<uint64_t>(-2147483649ll));
    EXPECT_FALSE(CanCastTo<uint64_t>(-4294967295ll));
    EXPECT_FALSE(CanCastTo<uint64_t>(-4294967296ll));
    EXPECT_FALSE(CanCastTo<uint64_t>(-9223372036854775807ll));

    EXPECT_TRUE(CanCastTo<int64_t>(0));
    EXPECT_TRUE(CanCastTo<int64_t>(127));
    EXPECT_TRUE(CanCastTo<int64_t>(128));
    EXPECT_TRUE(CanCastTo<int64_t>(129));
    EXPECT_TRUE(CanCastTo<int64_t>(255));
    EXPECT_TRUE(CanCastTo<int64_t>(256));
    EXPECT_TRUE(CanCastTo<int64_t>(32767));
    EXPECT_TRUE(CanCastTo<int64_t>(32768));
    EXPECT_TRUE(CanCastTo<int64_t>(32769));
    EXPECT_TRUE(CanCastTo<int64_t>(65535));
    EXPECT_TRUE(CanCastTo<int64_t>(65536));
    EXPECT_TRUE(CanCastTo<int64_t>(2147483647ll));
    EXPECT_TRUE(CanCastTo<int64_t>(2147483648ll));
    EXPECT_TRUE(CanCastTo<int64_t>(2147483649ll));
    EXPECT_TRUE(CanCastTo<int64_t>(4294967295ll));
    EXPECT_TRUE(CanCastTo<int64_t>(4294967296ll));
    EXPECT_TRUE(CanCastTo<int64_t>(9223372036854775807ull));
    EXPECT_FALSE(CanCastTo<int64_t>(9223372036854775808ull));

    EXPECT_TRUE(CanCastTo<int64_t>(-1));
    EXPECT_TRUE(CanCastTo<int64_t>(-127));
    EXPECT_TRUE(CanCastTo<int64_t>(-128));
    EXPECT_TRUE(CanCastTo<int64_t>(-129));
    EXPECT_TRUE(CanCastTo<int64_t>(-255));
    EXPECT_TRUE(CanCastTo<int64_t>(-256));
    EXPECT_TRUE(CanCastTo<int64_t>(-32767));
    EXPECT_TRUE(CanCastTo<int64_t>(-32768));
    EXPECT_TRUE(CanCastTo<int64_t>(-32769));
    EXPECT_TRUE(CanCastTo<int64_t>(-65535));
    EXPECT_TRUE(CanCastTo<int64_t>(-65536));
    EXPECT_TRUE(CanCastTo<int64_t>(-2147483647ll));
    EXPECT_TRUE(CanCastTo<int64_t>(-2147483648ll));
    EXPECT_TRUE(CanCastTo<int64_t>(-2147483649ll));
    EXPECT_TRUE(CanCastTo<int64_t>(-4294967295ll));
    EXPECT_TRUE(CanCastTo<int64_t>(-4294967296ll));
    EXPECT_TRUE(CanCastTo<int64_t>(-9223372036854775807ll));
}

TEST(TestSafeInt, TestCastToSigned)
{
    EXPECT_EQ(CastToSigned(static_cast<uint8_t>(5)), 5);
    EXPECT_EQ(CastToSigned(static_cast<uint8_t>(-5)), -5);
    EXPECT_EQ(CastToSigned(static_cast<uint8_t>(254)), -2);
    EXPECT_EQ(CastToSigned(static_cast<uint8_t>(65533)), -3);
    EXPECT_EQ(CastToSigned(static_cast<uint16_t>(254)), 254);
    EXPECT_EQ(CastToSigned(static_cast<uint16_t>(65533)), -3);
}
