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

#include <cmath>
#include <functional>
#include <string>

#include <lib/support/UnitTestRegistration.h>
#include <lib/support/value_parser/ValueParserComplex.h>
#include <lib/support/value_parser/ValueParserScalar.h>

#include <nlunit-test.h>

using chip::ByteSpan;
using chip::CharSpan;
using chip::Optional;
using chip::app::DataModel::List;
using chip::app::DataModel::Nullable;
using chip::ValueParser::ParseValue;

enum class TestEnum
{
    TEST_VALUE_0 = 0,
    TEST_VALUE_1,
};

enum class TestBitFlag
{
    TEST_BIT0 = 0,
    TEST_BIT1 = 1,
    TEST_BIT2 = 2,
    TEST_BIT3 = 4,
};

template <typename T>
static void AssertValueParseFail(nlTestSuite * inSuite, const char * valueString)
{
    T parsedValue;
    std::vector<std::function<void(void)>> freeFunctions;

    CHIP_ERROR error = ParseValue(valueString, parsedValue, freeFunctions);
    NL_TEST_ASSERT(inSuite, error == CHIP_ERROR_INVALID_ARGUMENT);
    for (auto && f : freeFunctions)
    {
        f();
    }
}

template <typename T>
static void AssertValueParseEquals(nlTestSuite * inSuite, const char * valueString, const T & expectedValue)
{
    T parsedValue;
    std::vector<std::function<void(void)>> freeFunctions;

    CHIP_ERROR error = ParseValue(valueString, parsedValue, freeFunctions);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, parsedValue == expectedValue);
    for (auto && f : freeFunctions)
    {
        f();
    }
}

template <typename T>
static void AssertValueParseSpanEquals(nlTestSuite * inSuite, const char * valueString, const T & expectedValue)
{
    T parsedValue;
    std::vector<std::function<void(void)>> freeFunctions;

    CHIP_ERROR error = ParseValue(valueString, parsedValue, freeFunctions);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, parsedValue.data_equal(expectedValue));
    for (auto && f : freeFunctions)
    {
        f();
    }
}

static void AssertValueParseEquals(nlTestSuite * inSuite, const char * valueString, float expectedValue)
{
    float parsedValue;
    std::vector<std::function<void(void)>> freeFunctions;

    CHIP_ERROR error = ParseValue(valueString, parsedValue, freeFunctions);
    NL_TEST_ASSERT(inSuite, error == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, fabs(expectedValue - parsedValue) < 1e-5);
    for (auto && f : freeFunctions)
    {
        f();
    }
}

static void TestParseInteger(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<int>(inSuite, "");
    AssertValueParseFail<int>(inSuite, " ");
    AssertValueParseFail<int>(inSuite, "abc");
    AssertValueParseEquals<int>(inSuite, "1", 1);
    AssertValueParseEquals<int>(inSuite, "-1", -1);
    AssertValueParseFail<unsigned>(inSuite, "");
    AssertValueParseFail<unsigned>(inSuite, " ");
    AssertValueParseFail<unsigned>(inSuite, "abc");
    AssertValueParseFail<unsigned>(inSuite, "-1");
    AssertValueParseEquals<unsigned>(inSuite, "1", 1);
}

static void TestParseFloat(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<float>(inSuite, "");
    AssertValueParseFail<float>(inSuite, " ");
    AssertValueParseFail<float>(inSuite, "abc");
    AssertValueParseEquals(inSuite, "1.5", static_cast<float>(1.5));
}

static void TestParseBool(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<bool>(inSuite, "");
    AssertValueParseFail<bool>(inSuite, " ");
    AssertValueParseFail<bool>(inSuite, "abc");
    AssertValueParseFail<bool>(inSuite, "1");
    AssertValueParseFail<bool>(inSuite, "0");
    AssertValueParseFail<bool>(inSuite, "truee");
    AssertValueParseEquals(inSuite, "true", true);
    AssertValueParseEquals(inSuite, "false", false);
}

static void TestParseBitFlag(nlTestSuite * inSuite, void * inContext)
{
    using TestBitFlagType = chip::BitFlags<TestBitFlag, int>;
    AssertValueParseFail<TestBitFlagType>(inSuite, "");
    AssertValueParseFail<TestBitFlagType>(inSuite, " ");
    AssertValueParseFail<TestBitFlagType>(inSuite, "abc");
    TestBitFlagType flag;
    flag.Set(TestBitFlag::TEST_BIT1);
    flag.Set(TestBitFlag::TEST_BIT2);
    AssertValueParseEquals<TestBitFlagType>(inSuite, "3", flag);
}

static void TestParseEnum(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<TestEnum>(inSuite, "");
    AssertValueParseFail<TestEnum>(inSuite, " ");
    AssertValueParseFail<TestEnum>(inSuite, "abc");
    AssertValueParseEquals(inSuite, "0", TestEnum::TEST_VALUE_0);
    AssertValueParseEquals(inSuite, "1", TestEnum::TEST_VALUE_1);
}

static void TestParseCharSpan(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseSpanEquals(inSuite, "", CharSpan());
    char expected[] = { 'a', 'b', 'c' };
    AssertValueParseSpanEquals(inSuite, "abc", CharSpan(expected));
}

static void TestParseByteSpan(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<chip::ByteSpan>(inSuite, "xx");
    AssertValueParseFail<chip::ByteSpan>(inSuite, "hex:00112");
    AssertValueParseFail<chip::ByteSpan>(inSuite, "001122");
    AssertValueParseSpanEquals(inSuite, "hex:", ByteSpan());
    uint8_t expected[] = { 0x00, 0x11, 0x22 };
    AssertValueParseSpanEquals(inSuite, "hex:001122", ByteSpan(expected));
}

static void TestParseOptional(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<Optional<int>>(inSuite, "abc");
    AssertValueParseEquals(inSuite, "", Optional<int>());
    AssertValueParseEquals(inSuite, "1", Optional<int>(1));
}

static void TestParseNullable(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<Nullable<int>>(inSuite, "abc");
    AssertValueParseFail<Nullable<int>>(inSuite, "");
    AssertValueParseEquals(inSuite, "1", Nullable<int>(1));
}

static void TestParseList(nlTestSuite * inSuite, void * inContext)
{
    AssertValueParseFail<List<int>>(inSuite, "");
    AssertValueParseFail<List<int>>(inSuite, "[");
    AssertValueParseFail<List<int>>(inSuite, "[,]");
    AssertValueParseFail<List<int>>(inSuite, "]");
    AssertValueParseFail<List<int>>(inSuite, ",");
    AssertValueParseFail<List<int>>(inSuite, "[1,");
    int expected[] = { 1, 2, 3 };
    AssertValueParseSpanEquals(inSuite, "[1, 2, 3]", List<int>(expected));
    AssertValueParseSpanEquals(inSuite, "[   1,2, 3  ]", List<int>(expected));
    AssertValueParseSpanEquals(inSuite, "[ 1, 2, 3,]", List<int>(expected));
    AssertValueParseSpanEquals(inSuite, "[1,2,3]", List<int>(expected));
}

#define NL_TEST_DEF_FN(fn) NL_TEST_DEF("Test " #fn, fn)
/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF_FN(TestParseInteger),
                                 NL_TEST_DEF_FN(TestParseFloat),
                                 NL_TEST_DEF_FN(TestParseBool),
                                 NL_TEST_DEF_FN(TestParseBitFlag),
                                 NL_TEST_DEF_FN(TestParseEnum),
                                 NL_TEST_DEF_FN(TestParseCharSpan),
                                 NL_TEST_DEF_FN(TestParseByteSpan),
                                 NL_TEST_DEF_FN(TestParseOptional),
                                 NL_TEST_DEF_FN(TestParseNullable),
                                 NL_TEST_DEF_FN(TestParseList),
                                 NL_TEST_SENTINEL() };

int TestValueParser(void)
{
    nlTestSuite theSuite = { "CHIP value parser tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestValueParser)
