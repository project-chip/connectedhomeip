/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/tests/suites/pixit/PixitTypeTransformer.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <stdlib.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestStringPixit(nlTestSuite * inSuite, const std::string & pixit)
{
    std::string val = "testtest";

    PixitTypeTransformer::PixitValueHolder ret = PixitTypeTransformer::TransformToType(pixit, val);
    NL_TEST_ASSERT(inSuite, ret.Valid());
    if (!ret.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, ret.Is<std::string>());
    NL_TEST_ASSERT(inSuite, ret.Get<std::string>() == val);
}

template <typename T>
void TestNumberPixit(nlTestSuite * inSuite, const std::string & pixit)
{
    T testVal                 = 65;
    T hexVal                  = 0x1F;
    std::string testValStr    = std::to_string(testVal);
    std::string hexValStr     = "0x1F";
    std::string invalidValStr = "hhhhh";

    std::string biggestValStr  = std::to_string(std::numeric_limits<T>::max());
    std::string smallestValStr = std::to_string(std::numeric_limits<T>::min());

    std::string tooBigValStr   = biggestValStr;
    std::string tooSmallValStr = smallestValStr;

    // Dirty hack to add 1 to the long long number (used for everything)
    // Only works because there are no numbers ending in 9
    tooBigValStr.back()++;
    if (std::numeric_limits<T>::min() < 0)
    {
        tooSmallValStr.back()++;
    }
    else
    {
        tooSmallValStr = "-1";
    }
    PixitTypeTransformer::PixitValueHolder val;
    val = PixitTypeTransformer::TransformToType(pixit, testValStr);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<T>() == testVal);

    val = PixitTypeTransformer::TransformToType(pixit, hexValStr);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<T>() == hexVal);

    val = PixitTypeTransformer::TransformToType(pixit, biggestValStr);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<T>() == std::numeric_limits<T>::max());

    val = PixitTypeTransformer::TransformToType(pixit, smallestValStr);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<T>() == std::numeric_limits<T>::min());

    val = PixitTypeTransformer::TransformToType(pixit, tooBigValStr);
    NL_TEST_ASSERT(inSuite, !val.Valid());

    val = PixitTypeTransformer::TransformToType(pixit, tooSmallValStr);
    NL_TEST_ASSERT(inSuite, !val.Valid());

    val = PixitTypeTransformer::TransformToType(pixit, invalidValStr);
    NL_TEST_ASSERT(inSuite, !val.Valid());
}

void TestBoolPixit(nlTestSuite * inSuite, const std::string & pixit)
{
    std::string trueAs1            = "1";
    std::string trueAsStringLower  = "true";
    std::string trueAsStringUpper  = "TRUE";
    std::string trueAsStringMixed  = "True";
    std::string falseAs0           = "0";
    std::string falseAsStringLower = "false";
    std::string falseAsStringUpper = "FALSE";
    std::string falseAsStringMixed = "False";

    PixitTypeTransformer::PixitValueHolder val;
    val = PixitTypeTransformer::TransformToType(pixit, trueAs1);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == true);

    val = PixitTypeTransformer::TransformToType(pixit, trueAsStringLower);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == true);

    val = PixitTypeTransformer::TransformToType(pixit, trueAsStringUpper);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == true);

    val = PixitTypeTransformer::TransformToType(pixit, trueAsStringMixed);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == true);

    val = PixitTypeTransformer::TransformToType(pixit, falseAs0);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == false);

    val = PixitTypeTransformer::TransformToType(pixit, falseAsStringLower);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == false);

    val = PixitTypeTransformer::TransformToType(pixit, falseAsStringUpper);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == false);

    val = PixitTypeTransformer::TransformToType(pixit, falseAsStringMixed);
    NL_TEST_ASSERT(inSuite, val.Valid());
    if (!val.Valid())
    {
        return;
    }
    NL_TEST_ASSERT(inSuite, val.Get<bool>() == false);
}

void TestStringPixit(nlTestSuite * inSuite, void * inContext)
{
    TestStringPixit(inSuite, "PIXIT.UNITTEST.STRING.1");
}

void TestPixitLowercase(nlTestSuite * inSuite, void * inContext)
{
    TestStringPixit(inSuite, "PIXIT.UNITTEST.string.1");
}

void TestUint8Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<uint8_t>(inSuite, "PIXIT.UNITTEST.UINT8.1");
}

void TestUint16Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<uint16_t>(inSuite, "PIXIT.UNITTEST.UINT16.1");
}

void TestUint32Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<uint32_t>(inSuite, "PIXIT.UNITTEST.UINT32.1");
}

void TestUint64Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<uint64_t>(inSuite, "PIXIT.UNITTEST.UINT64.1");
}

void TestInt8Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<int8_t>(inSuite, "PIXIT.UNITTEST.INT8.1");
}

void TestInt16Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<int16_t>(inSuite, "PIXIT.UNITTEST.INT16.1");
}

void TestInt32Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<int32_t>(inSuite, "PIXIT.UNITTEST.INT32.1");
}

void TestInt64Pixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<int64_t>(inSuite, "PIXIT.UNITTEST.INT64.1");
}

void TestSizetPixit(nlTestSuite * inSuite, void * inContext)
{
    TestNumberPixit<size_t>(inSuite, "PIXIT.UNITTEST.SIZET.1");
}

void TestBoolPixit(nlTestSuite * inSuite, void * inContext)
{
    TestBoolPixit(inSuite, "PIXIT.UNITTEST.BOOL.T");
}

void TestIncorrectPixit(nlTestSuite * inSuite, void * inContext)
{
    PixitTypeTransformer::PixitValueHolder val = PixitTypeTransformer::TransformToType("PIXIT.UNITTEST.UNUSED", "testsarefun");
    NL_TEST_ASSERT(inSuite, !val.Valid());
}

} // namespace

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("TestStringPixit", TestStringPixit),       NL_TEST_DEF("TestPixitLowercase", TestPixitLowercase),
    NL_TEST_DEF("TestUint8Pixit", TestUint8Pixit),         NL_TEST_DEF("TestUint16Pixit", TestUint16Pixit),
    NL_TEST_DEF("TestUint32Pixit", TestUint32Pixit),       NL_TEST_DEF("TestUint64Pixit", TestUint64Pixit),
    NL_TEST_DEF("TestInt8Pixit", TestInt8Pixit),           NL_TEST_DEF("TestInt16Pixit", TestInt16Pixit),
    NL_TEST_DEF("TestInt32Pixit", TestInt32Pixit),         NL_TEST_DEF("TestInt64Pixit", TestInt64Pixit),
    NL_TEST_DEF("TestSizetPixit", TestSizetPixit),         NL_TEST_DEF("TestBoolPixit", TestBoolPixit),
    NL_TEST_DEF("TestIncorrectPixit", TestIncorrectPixit), NL_TEST_SENTINEL()
};

nlTestSuite sSuite = {
    "TestPixit",
    &sTests[0],
    nullptr,
    nullptr,
};

int TestPixitTypeTransformer()
{
    nlTestRunner(&sSuite, nullptr);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPixitTypeTransformer);
