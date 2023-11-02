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

#include <platform/BuildTime.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

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

static void TestPlatformMgr_RunUnitTest(nlTestSuite * inSuite, void * inContext)
{
#if CHIP_DEVICE_LAYER_TARGET_OPEN_IOT_SDK
    // TODO: Fix RunUnitTests() for Open IOT SDK.
    // Previously, TestPlatformMgr_RunUnitTest was only run if !NDEBUG while the Open IOT SDK
    // test runner was built with NDEBUG set.
    return;
#endif

    ConfigurationMgr().RunUnitTests();
}

static void TestConfigurationMgr_SerialNumber(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[64];
    const char * serialNumber = "89051AAZZ236";

    err = ConfigurationMgr().StoreSerialNumber(serialNumber, strlen(serialNumber));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = GetDeviceInstanceInfoProvider()->GetSerialNumber(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strlen(buf) == 12);
    NL_TEST_ASSERT(inSuite, strcmp(buf, serialNumber) == 0);

    err = ConfigurationMgr().StoreSerialNumber(serialNumber, 5);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = GetDeviceInstanceInfoProvider()->GetSerialNumber(buf, 64);
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

    err = GetDeviceInstanceInfoProvider()->GetManufacturingDate(year, month, dayOfMonth);
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

    err = GetDeviceInstanceInfoProvider()->GetHardwareVersion(hardwareVer);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, hardwareVer == 1234);
}

static int SnprintfBuildDate(char * s, size_t n, uint16_t year, uint8_t month, uint8_t day)
{
    // Print the calendar date to a human readable string as would
    // given from the __DATE__ macro.
    const char * monthString = nullptr;
    switch (month)
    {
    case 1:
        monthString = "Jan";
        break;
    case 2:
        monthString = "Feb";
        break;
    case 3:
        monthString = "Mar";
        break;
    case 4:
        monthString = "Apr";
        break;
    case 5:
        monthString = "May";
        break;
    case 6:
        monthString = "Jun";
        break;
    case 7:
        monthString = "Jul";
        break;
    case 8:
        monthString = "Aug";
        break;
    case 9:
        monthString = "Sep";
        break;
    case 10:
        monthString = "Oct";
        break;
    case 11:
        monthString = "Nov";
        break;
    case 12:
        monthString = "Dec";
        break;
    }
    if (monthString == nullptr)
    {
        return -1;
    }
    return snprintf(s, n, "%s %2u %u", monthString, day, year);
}

static int SnprintfBuildDate(char * s, size_t n, System::Clock::Seconds32 chipEpochBuildTime)
{
    // Convert to a calendar date-time.
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    ChipEpochToCalendarTime(chipEpochBuildTime.count(), year, month, day, hour, minute, second);
    return SnprintfBuildDate(s, n, year, month, day);
}

static int SnprintfBuildTimeOfDay(char * s, size_t n, uint8_t hour, uint8_t minute, uint8_t second)
{
    // Print the time of day to a human readable string as would
    // given from the __TIME__ macro.
    return snprintf(s, n, "%02u:%02u:%02u", hour, minute, second);
}

static int SnprintfBuildTimeOfDay(char * s, size_t n, System::Clock::Seconds32 chipEpochBuildTime)
{
    // Convert to a calendar date-time.
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    ChipEpochToCalendarTime(chipEpochBuildTime.count(), year, month, day, hour, minute, second);
    return SnprintfBuildTimeOfDay(s, n, hour, minute, second);
}

static void TestConfigurationMgr_FirmwareBuildTime(nlTestSuite * inSuite, void * inContext)
{
    // Read the firmware build time from the configuration manager.
    // This is referenced to the CHIP epoch.
    System::Clock::Seconds32 chipEpochTime;
    NL_TEST_ASSERT(inSuite, ConfigurationMgr().GetFirmwareBuildChipEpochTime(chipEpochTime) == CHIP_NO_ERROR);

    // Override the hard-coded build time with the setter and verify operation.
    System::Clock::Seconds32 overrideValue = System::Clock::Seconds32(rand());
    NL_TEST_ASSERT(inSuite, ConfigurationMgr().SetFirmwareBuildChipEpochTime(overrideValue) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, ConfigurationMgr().GetFirmwareBuildChipEpochTime(chipEpochTime) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, overrideValue == chipEpochTime);

    // Verify that the BuildTime.h parser can parse current CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE / TIME.
    do
    {
        const char * date      = CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE;
        const char * timeOfDay = CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME;

        // Check that strings look good.
        NL_TEST_ASSERT(inSuite, !BUILD_DATE_IS_BAD(date));
        NL_TEST_ASSERT(inSuite, !BUILD_TIME_IS_BAD(timeOfDay));
        if (BUILD_DATE_IS_BAD(date) || BUILD_TIME_IS_BAD(timeOfDay))
        {
            break;
        }

        // Parse.
        uint16_t year  = COMPUTE_BUILD_YEAR(date);
        uint8_t month  = COMPUTE_BUILD_MONTH(date);
        uint8_t day    = COMPUTE_BUILD_DAY(date);
        uint8_t hour   = COMPUTE_BUILD_HOUR(timeOfDay);
        uint8_t minute = COMPUTE_BUILD_MIN(timeOfDay);
        uint8_t second = COMPUTE_BUILD_SEC(timeOfDay);

        // Print the date to a string as would be given by the __DATE__ macro.
        char parsedDate[14] = { 0 }; // strlen("Jan 000 00000") == 13
        {
            int printed;
            printed = SnprintfBuildDate(parsedDate, sizeof(parsedDate), year, month, day);
            NL_TEST_ASSERT(inSuite, printed > 0 && printed < static_cast<int>(sizeof(parsedDate)));
        }

        // Print the time of day to a straing as would be given by the __TIME__ macro.
        char parsedTimeOfDay[12] = { 0 }; // strlen("000:000:000") == 11
        {
            int printed;
            printed = SnprintfBuildTimeOfDay(parsedTimeOfDay, sizeof(parsedTimeOfDay), hour, minute, second);
            NL_TEST_ASSERT(inSuite, printed > 0 && printed < static_cast<int>(sizeof(parsedTimeOfDay)));
        }

        // Verify match.
        NL_TEST_ASSERT(inSuite, strcmp(date, parsedDate) == 0);
        NL_TEST_ASSERT(inSuite, strcmp(timeOfDay, parsedTimeOfDay) == 0);
    } while (false);

    // Generate random chip epoch times and verify that our BuildTime.h parser
    // macros also work for these.
    for (int i = 0; i < 10000; ++i)
    {
        char date[14]      = { 0 }; // strlen("Jan 000 00000") == 13
        char timeOfDay[12] = { 0 }; // strlen("000:000:000") == 11

        chipEpochTime = System::Clock::Seconds32(rand());

        // rand() will only give us [0, 0x7FFFFFFF].  Give us coverage for
        // times in the upper half of the chip epoch time range as well.
        chipEpochTime = i % 2 ? chipEpochTime : System::Clock::Seconds32(chipEpochTime.count() | 0x80000000);

        // Print the date to a string as would be given by the __DATE__ macro.
        {
            int printed;
            printed = SnprintfBuildDate(date, sizeof(date), chipEpochTime);
            NL_TEST_ASSERT(inSuite, printed > 0 && printed < static_cast<int>(sizeof(date)));
        }

        // Print the time of day to a straing as would be given by the __TIME__ macro.
        {
            int printed;
            printed = SnprintfBuildTimeOfDay(timeOfDay, sizeof(timeOfDay), chipEpochTime);
            NL_TEST_ASSERT(inSuite, printed > 0 && printed < static_cast<int>(sizeof(timeOfDay)));
        }

        // Check that strings look good.
        NL_TEST_ASSERT(inSuite, !BUILD_DATE_IS_BAD(date));
        NL_TEST_ASSERT(inSuite, !BUILD_TIME_IS_BAD(timeOfDay));
        if (BUILD_DATE_IS_BAD(date) || BUILD_TIME_IS_BAD(timeOfDay))
        {
            continue;
        }

        // Convert from chip epoch seconds to calendar time.
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        ChipEpochToCalendarTime(chipEpochTime.count(), year, month, day, hour, minute, second);

        // Verify that our BuildTime.h macros can correctly parse the date / time strings.
        NL_TEST_ASSERT(inSuite, year == COMPUTE_BUILD_YEAR(date));
        NL_TEST_ASSERT(inSuite, month == COMPUTE_BUILD_MONTH(date));
        NL_TEST_ASSERT(inSuite, day == COMPUTE_BUILD_DAY(date));
        NL_TEST_ASSERT(inSuite, hour == COMPUTE_BUILD_HOUR(timeOfDay));
        NL_TEST_ASSERT(inSuite, minute == COMPUTE_BUILD_MIN(timeOfDay));
        NL_TEST_ASSERT(inSuite, second == COMPUTE_BUILD_SEC(timeOfDay));
    }
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

static void TestConfigurationMgr_GetVendorName(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char buf[64];

    err = GetDeviceInstanceInfoProvider()->GetVendorName(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strlen(buf) > 0 && strlen(buf) <= ConfigurationManager::kMaxVendorNameLength);
}

static void TestConfigurationMgr_GetVendorId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t vendorId;

    err = GetDeviceInstanceInfoProvider()->GetVendorId(vendorId);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, vendorId >= 0 && vendorId <= 0xfff4);
}

static void TestConfigurationMgr_GetProductName(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char buf[64];

    err = GetDeviceInstanceInfoProvider()->GetProductName(buf, 64);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strlen(buf) > 0 && strlen(buf) <= ConfigurationManager::kMaxProductNameLength);
}

static void TestConfigurationMgr_GetProductId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t productId;

    err = GetDeviceInstanceInfoProvider()->GetProductId(productId);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, productId >= 1 && productId <= 0xffff);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("Test PlatformMgr::Init", TestPlatformMgr_Init),
    NL_TEST_DEF("Test PlatformMgr::RunUnitTest", TestPlatformMgr_RunUnitTest),
    NL_TEST_DEF("Test ConfigurationMgr::SerialNumber", TestConfigurationMgr_SerialNumber),
    NL_TEST_DEF("Test ConfigurationMgr::UniqueId", TestConfigurationMgr_UniqueId),
    NL_TEST_DEF("Test ConfigurationMgr::ManufacturingDate", TestConfigurationMgr_ManufacturingDate),
    NL_TEST_DEF("Test ConfigurationMgr::HardwareVersion", TestConfigurationMgr_HardwareVersion),
    NL_TEST_DEF("Test ConfigurationMgr::FirmwareBuildTime", TestConfigurationMgr_FirmwareBuildTime),
    NL_TEST_DEF("Test ConfigurationMgr::CountryCode", TestConfigurationMgr_CountryCode),
    NL_TEST_DEF("Test ConfigurationMgr::GetPrimaryMACAddress", TestConfigurationMgr_GetPrimaryMACAddress),
    NL_TEST_DEF("Test ConfigurationMgr::GetFailSafeArmed", TestConfigurationMgr_GetFailSafeArmed),
    NL_TEST_DEF("Test ConfigurationMgr::GetVendorName", TestConfigurationMgr_GetVendorName),
    NL_TEST_DEF("Test ConfigurationMgr::GetVendorId", TestConfigurationMgr_GetVendorId),
    NL_TEST_DEF("Test ConfigurationMgr::GetProductName", TestConfigurationMgr_GetProductName),
    NL_TEST_DEF("Test ConfigurationMgr::GetProductId", TestConfigurationMgr_GetProductId),
    NL_TEST_SENTINEL(),
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

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestConfigurationMgr)
