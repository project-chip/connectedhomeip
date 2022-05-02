/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *      This file implements a unit test suite for the Configuration Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

namespace {

// =================================
//      Unit tests
// =================================

static void TestPlatformMgr_Init(nlTestSuite * inSuite, void * inContext)
{
    // ConfigurationManager is initialized from PlatformManager indirectly
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

#if !defined(NDEBUG)
static void TestPlatformMgr_RunUnitTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ConfigurationMgr().RunUnitTests();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}
#endif

static void TestConfigurationMgr_SerialNumber(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[64];
    const char * serialNumber = "89051AAZZ236";

    err = ConfigurationMgr().StoreSerialNumber(serialNumber, strlen(serialNumber));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetSerialNumber(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strlen(buf) == 12);
    NL_TEST_ASSERT(inSuite, strcmp(buf, serialNumber) == 0);

    err = ConfigurationMgr().StoreSerialNumber(serialNumber, 5);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetSerialNumber(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strlen(buf) == 5);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "89051") == 0);
}

static void TestConfigurationMgr_UniqueId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[64];
    const char * uniqueId = "67MXAZ012RT8UE";

    err = ConfigurationMgr().StoreUniqueId(uniqueId, strlen(uniqueId));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetUniqueId(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strlen(buf) == 14);
    NL_TEST_ASSERT(inSuite, strcmp(buf, uniqueId) == 0);

    err = ConfigurationMgr().StoreUniqueId(uniqueId, 7);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetUniqueId(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strlen(buf) == 7);
    NL_TEST_ASSERT(inSuite, strcmp(buf, "67MXAZ0") == 0);
}

static void TestConfigurationMgr_ManufacturingDate(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    const char * mfgDate = "2008/09/20";
    uint16_t year;
    uint8_t month;
    uint8_t dayOfMonth;

    err = ConfigurationMgr().StoreManufacturingDate(mfgDate, strlen(mfgDate));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetManufacturingDate(year, month, dayOfMonth);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, year == 2008);
    NL_TEST_ASSERT(inSuite, month == 9);
    NL_TEST_ASSERT(inSuite, dayOfMonth == 20);
}

static void TestConfigurationMgr_HardwareVersion(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t hardwareVer;

    err = ConfigurationMgr().StoreHardwareVersion(1234);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetHardwareVersion(hardwareVer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, hardwareVer == 1234);
}

static void TestConfigurationMgr_CountryCode(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[8];
    size_t countryCodeLen    = 0;
    const char * countryCode = "US";

    err = ConfigurationMgr().StoreCountryCode(countryCode, strlen(countryCode));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetCountryCode(buf, 8, countryCodeLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, countryCodeLen == strlen(countryCode));
    NL_TEST_ASSERT(inSuite, strcmp(buf, countryCode) == 0);
}

static void TestConfigurationMgr_GetPrimaryMACAddress(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t macBuffer8Bytes[8];
    uint8_t macBuffer6Bytes[6];
    MutableByteSpan mac8Bytes(macBuffer8Bytes);
    MutableByteSpan mac6Bytes(macBuffer6Bytes);

    err = ConfigurationMgr().GetPrimaryMACAddress(mac8Bytes);
    if (mac8Bytes.size() != ConfigurationManager::kPrimaryMACAddressLength)
    {
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    }

    err = ConfigurationMgr().GetPrimaryMACAddress(mac6Bytes);
    if (mac6Bytes.size() != ConfigurationManager::kPrimaryMACAddressLength)
    {
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
    }

    // NOTICE for above:
    //   no validation for CHIP_NO_ERROR:
    //    - there is no guarantee in CI that a valid IP address exists,
    //      expecially if running in emulators (zephyr and qemu)
}

static void TestConfigurationMgr_GetFailSafeArmed(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    bool failSafeArmed = false;

    err = ConfigurationMgr().SetFailSafeArmed(true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ConfigurationMgr().GetFailSafeArmed(failSafeArmed);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, failSafeArmed == true);

    err = ConfigurationMgr().SetFailSafeArmed(false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("Test PlatformMgr::Init", TestPlatformMgr_Init),
#if !defined(NDEBUG)
    NL_TEST_DEF("Test PlatformMgr::RunUnitTest", TestPlatformMgr_RunUnitTest),
#endif
    NL_TEST_DEF("Test ConfigurationMgr::SerialNumber", TestConfigurationMgr_SerialNumber),
    NL_TEST_DEF("Test ConfigurationMgr::UniqueId", TestConfigurationMgr_UniqueId),
    NL_TEST_DEF("Test ConfigurationMgr::ManufacturingDate", TestConfigurationMgr_ManufacturingDate),
    NL_TEST_DEF("Test ConfigurationMgr::HardwareVersion", TestConfigurationMgr_HardwareVersion),
    NL_TEST_DEF("Test ConfigurationMgr::CountryCode", TestConfigurationMgr_CountryCode),
    NL_TEST_DEF("Test ConfigurationMgr::GetPrimaryMACAddress", TestConfigurationMgr_GetPrimaryMACAddress),
    NL_TEST_DEF("Test ConfigurationMgr::GetFailSafeArmed", TestConfigurationMgr_GetFailSafeArmed),
    NL_TEST_SENTINEL()
};

/**
 *  Set up the test suite.
 */
int TestConfigurationMgr_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestConfigurationMgr_Teardown(void * inContext)
{
    PlatformMgr().Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestConfigurationMgr()
{
    nlTestSuite theSuite = { "ConfigurationMgr tests", &sTests[0], TestConfigurationMgr_Setup, TestConfigurationMgr_Teardown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestConfigurationMgr)
