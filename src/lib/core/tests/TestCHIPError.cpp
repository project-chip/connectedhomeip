/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <string>

#include <lib/core/CHIPError.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void RangeConstructor(nlTestSuite * inSuite, void * /*inContext*/)
{
    ChipError error(ChipError::Range::kSDK, /*value=*/1, "file_name", /*line=*/2);
#if CHIP_CONFIG_ERROR_SOURCE
    NL_TEST_ASSERT(inSuite, std::string(error.GetFile()) == "file_name");
    NL_TEST_ASSERT(inSuite, error.GetLine() == 2);
#if __cplusplus >= 202002L
    std::source_location location = error.GetSourceLocation();
    NL_TEST_ASSERT(inSuite, error.line() == 37);
    NL_TEST_ASSERT(inSuite, error.file_name() == "TestCHIPError.cpp");
#endif // __cplusplus >= 202002L
#endif // CHIP_CONFIG_ERROR_SOURCE
}

void SdkPartConstructor(nlTestSuite * inSuite, void * /*inContext*/)
{
    ChipError error(ChipError::SdkPart::kCore, /*code=*/1, "file_name", /*line=*/2);
#if CHIP_CONFIG_ERROR_SOURCE
    NL_TEST_ASSERT(inSuite, std::string(error.GetFile()) == "file_name");
    NL_TEST_ASSERT(inSuite, error.GetLine() == 2);
#if __cplusplus >= 202002L
    std::source_location location = error.GetSourceLocation();
    NL_TEST_ASSERT(inSuite, error.line() == 49);
    NL_TEST_ASSERT(inSuite, error.file_name() == "TestCHIPError.cpp");
#endif // __cplusplus >= 202002L
#endif // CHIP_CONFIG_ERROR_SOURCE
}

void StorageTypeConstructor(nlTestSuite * inSuite, void * /*inContext*/)
{
    ChipError error(/*error=*/1, "file_name", /*line=*/2);
    NL_TEST_ASSERT(inSuite, error.AsInteger() == 1);
#if CHIP_CONFIG_ERROR_SOURCE
    NL_TEST_ASSERT(inSuite, std::string(error.GetFile()) == "file_name");
    NL_TEST_ASSERT(inSuite, error.GetLine() == 2);
#if __cplusplus >= 202002L
    std::source_location location = error.GetSourceLocation();
    NL_TEST_ASSERT(inSuite, error.line() == 61);
    NL_TEST_ASSERT(inSuite, error.file_name() == "TestCHIPError.cpp");
#endif // __cplusplus >= 202002L
#endif // CHIP_CONFIG_ERROR_SOURCE
}

/**
 *   Test Suite. It lists all the test functions.
 */

static const nlTest sTests[] = { NL_TEST_DEF("RangeConstructor", RangeConstructor),
                                 NL_TEST_DEF("SdkPartConstructor", SdkPartConstructor),
                                 NL_TEST_DEF("StorageTypeConstructor", StorageTypeConstructor), NL_TEST_SENTINEL() };

} // namespace

int TestCHIPError()
{
    nlTestSuite theSuite = { "Test CHIP_ERROR string conversions", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestCHIPError)
