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

#include <app/tests/suites/pixit/PixitReader.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>

#include <stdlib.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestPixitReaderAllUnitTest(nlTestSuite * inSuite, void * inContext)
{
    PixitReader::PixitMap pixitMap = PixitReader::PopulateDefaults();
    std::string this_file          = __FILE__;
    std::string dir_path           = this_file.substr(0, this_file.rfind("/"));

    CHIP_ERROR err = PixitReader::UpdateFromFile(dir_path + "/all-unit-test.pixit", pixitMap);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.STRING.1"].Get<std::string>() == "file1");
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.STRING.2"].Get<std::string>() == "file2");

    // Unsigned
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT8.1"].Get<uint8_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT8.2"].Get<uint8_t>() == 4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT16.1"].Get<uint16_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT16.2"].Get<uint16_t>() == 4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT32.1"].Get<uint32_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT32.2"].Get<uint32_t>() == 4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT64.1"].Get<uint64_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.UINT64.2"].Get<uint64_t>() == 4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.SIZET.1"].Get<size_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.SIZET.2"].Get<size_t>() == 4);

    // Signed
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT8.1"].Get<int8_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT8.2"].Get<int8_t>() == -4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT16.1"].Get<int16_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT16.2"].Get<int16_t>() == -4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT32.1"].Get<int32_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT32.2"].Get<int32_t>() == -4);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT64.1"].Get<int64_t>() == 3);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.INT64.2"].Get<int64_t>() == -4);

    // Bools
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.BOOL.T"].Get<bool>() == true);
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.BOOL.F"].Get<bool>() == false);
}

void TestPixitReaderString(nlTestSuite * inSuite, void * inContext)
{
    PixitReader::PixitMap pixitMap = PixitReader::PopulateDefaults();
    std::string this_file          = __FILE__;
    std::string dir_path           = this_file.substr(0, this_file.rfind("/"));

    CHIP_ERROR err = PixitReader::UpdateFromFile(dir_path + "/strings-quotes-spaces.pixit", pixitMap);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.STRING.1"].Get<std::string>() == "file 1");
    NL_TEST_ASSERT(inSuite, pixitMap["PIXIT.UNITTEST.STRING.2"].Get<std::string>() == "file 2");
}

void TestPixitReaderMissing(nlTestSuite * inSuite, void * inContext)
{
    PixitReader::PixitMap pixitMap = PixitReader::PopulateDefaults();
    std::string this_file          = __FILE__;
    std::string dir_path           = this_file.substr(0, this_file.rfind("/"));

    CHIP_ERROR err = PixitReader::UpdateFromFile(dir_path + "/missing-key.pixit", pixitMap);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_READ_FAILED);
    err = PixitReader::UpdateFromFile(dir_path + "/missing-value.pixit", pixitMap);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_READ_FAILED);
}

} // namespace

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("TestPixitReaderAllUnitTest", TestPixitReaderAllUnitTest),
                                 NL_TEST_DEF("TestPixitReaderString", TestPixitReaderString),
                                 NL_TEST_DEF("TestPixitReaderMissing", TestPixitReaderMissing), NL_TEST_SENTINEL() };

nlTestSuite sSuite = {
    "TestPixit",
    &sTests[0],
    nullptr,
    nullptr,
};

int TestPixitReader()
{
    nlTestRunner(&sSuite, nullptr);
    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestPixitReader);
