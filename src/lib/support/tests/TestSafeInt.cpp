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

#include <lib/support/SafeInt.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;

static void TestCanCastTo_Int8(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, CanCastTo<uint8_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint8_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint8_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint8_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint8_t>(255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-1));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-127));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-128));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-129));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint8_t>(-9223372036854775807ll));

    NL_TEST_ASSERT(inSuite, CanCastTo<int8_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<int8_t>(127));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(128));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(129));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, CanCastTo<int8_t>(-1));
    NL_TEST_ASSERT(inSuite, CanCastTo<int8_t>(-127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int8_t>(-128));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-129));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int8_t>(-9223372036854775807ll));
}

static void TestCanCastTo_Int16(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(255));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(256));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint16_t>(65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-1));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-127));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-128));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-129));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint16_t>(-9223372036854775807ll));

    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(255));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(256));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-1));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-128));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-129));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-255));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-256));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<int16_t>(-32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int16_t>(-9223372036854775807ll));
}

static void TestCanCastTo_Int32(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(255));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(256));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(65535));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(65536));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint32_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-1));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-127));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-128));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-129));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint32_t>(-9223372036854775807ll));

    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(255));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(256));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(65535));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(65536));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-1));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-128));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-129));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-255));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-256));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-65535));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-65536));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int32_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int32_t>(-9223372036854775807ll));
}

static void TestCanCastTo_Int64(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(255));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(256));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(65535));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(65536));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, CanCastTo<uint64_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-1));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-127));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-128));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-129));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-255));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-256));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-32767));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-32768));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-32769));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-65535));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-65536));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, !CanCastTo<uint64_t>(-9223372036854775807ll));

    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(0));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(128));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(129));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(255));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(256));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(65535));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(65536));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(2147483647ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(2147483648ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(2147483649ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(4294967295ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(4294967296ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(9223372036854775807ull));
    NL_TEST_ASSERT(inSuite, !CanCastTo<int64_t>(9223372036854775808ull));

    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-1));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-127));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-128));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-129));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-255));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-256));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-32767));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-32768));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-32769));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-65535));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-65536));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-2147483647ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-2147483648ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-2147483649ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-4294967295ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-4294967296ll));
    NL_TEST_ASSERT(inSuite, CanCastTo<int64_t>(-9223372036854775807ll));
}

static void TestCastToSigned(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, CastToSigned(static_cast<uint8_t>(5)) == 5);
    NL_TEST_ASSERT(inSuite, CastToSigned(static_cast<uint8_t>(-5)) == -5);
    NL_TEST_ASSERT(inSuite, CastToSigned(static_cast<uint8_t>(254)) == -2);
    NL_TEST_ASSERT(inSuite, CastToSigned(static_cast<uint8_t>(65533)) == -3);
    NL_TEST_ASSERT(inSuite, CastToSigned(static_cast<uint16_t>(254)) == 254);
    NL_TEST_ASSERT(inSuite, CastToSigned(static_cast<uint16_t>(65533)) == -3);
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestCanCastTo_Int8),  NL_TEST_DEF_FN(TestCanCastTo_Int16),
                                 NL_TEST_DEF_FN(TestCanCastTo_Int32), NL_TEST_DEF_FN(TestCanCastTo_Int64),
                                 NL_TEST_DEF_FN(TestCastToSigned),    NL_TEST_SENTINEL() };

int TestSafeInt(void)
{
    nlTestSuite theSuite = { "CHIP SafeInt tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSafeInt)
