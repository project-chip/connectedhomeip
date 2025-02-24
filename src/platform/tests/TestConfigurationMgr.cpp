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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
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

struct TestConfigurationMgr : ::testing::Test
{
    static void SetUpTestSuite()
    {
        // ConfigurationManager is initialized from PlatformManager indirectly
        CHIP_ERROR err = chip::Platform::MemoryInit();
        EXPECT_EQ(err, CHIP_NO_ERROR);
        err = PlatformMgr().InitChipStack();
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }
};

TEST_F(TestConfigurationMgr, RunUnitTest)
{
#if CHIP_DEVICE_LAYER_TARGET_OPEN_IOT_SDK
    // TODO: Fix RunUnitTests() for Open IOT SDK.
    // Previously, TestPlatformMgr_RunUnitTest was only run if !NDEBUG while the Open IOT SDK
    // test runner was built with NDEBUG set.
    return;
#endif

    ConfigurationMgr().RunUnitTests();
}

TEST_F(TestConfigurationMgr, SerialNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[64];
    const char * serialNumber = "89051AAZZ236";

    err = ConfigurationMgr().StoreSerialNumber(serialNumber, strlen(serialNumber));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = GetDeviceInstanceInfoProvider()->GetSerialNumber(buf, 64);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strlen(buf), 12u);
    EXPECT_STREQ(buf, serialNumber);

    err = ConfigurationMgr().StoreSerialNumber(serialNumber, 5);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = GetDeviceInstanceInfoProvider()->GetSerialNumber(buf, 64);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strlen(buf), 5u);
    EXPECT_STREQ(buf, "89051");
}

TEST_F(TestConfigurationMgr, UniqueId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[64];
    const char * uniqueId = "67MXAZ012RT8UE";

    err = ConfigurationMgr().StoreUniqueId(uniqueId, strlen(uniqueId));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = ConfigurationMgr().GetUniqueId(buf, 64);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strlen(buf), 14u);
    EXPECT_STREQ(buf, uniqueId);

    err = ConfigurationMgr().StoreUniqueId(uniqueId, 7);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = ConfigurationMgr().GetUniqueId(buf, 64);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strlen(buf), 7u);
    EXPECT_STREQ(buf, "67MXAZ0");
}

TEST_F(TestConfigurationMgr, ManufacturingDate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    const char * mfgDate = "2008/09/20";
    uint16_t year;
    uint8_t month;
    uint8_t dayOfMonth;

    err = ConfigurationMgr().StoreManufacturingDate(mfgDate, strlen(mfgDate));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = GetDeviceInstanceInfoProvider()->GetManufacturingDate(year, month, dayOfMonth);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(year, 2008);
    EXPECT_EQ(month, 9);
    EXPECT_EQ(dayOfMonth, 20);
}

TEST_F(TestConfigurationMgr, HardwareVersion)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t hardwareVer;

    err = ConfigurationMgr().StoreHardwareVersion(1234);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = GetDeviceInstanceInfoProvider()->GetHardwareVersion(hardwareVer);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(hardwareVer, 1234);
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

TEST_F(TestConfigurationMgr, FirmwareBuildTime)
{
    // Read the firmware build time from the configuration manager.
    // This is referenced to the CHIP epoch.
    System::Clock::Seconds32 chipEpochTime;
    EXPECT_EQ(ConfigurationMgr().GetFirmwareBuildChipEpochTime(chipEpochTime), CHIP_NO_ERROR);

    // Override the hard-coded build time with the setter and verify operation.
    System::Clock::Seconds32 overrideValue = System::Clock::Seconds32(rand());
    EXPECT_EQ(ConfigurationMgr().SetFirmwareBuildChipEpochTime(overrideValue), CHIP_NO_ERROR);
    EXPECT_EQ(ConfigurationMgr().GetFirmwareBuildChipEpochTime(chipEpochTime), CHIP_NO_ERROR);
    EXPECT_EQ(overrideValue, chipEpochTime);

    // Verify that the BuildTime.h parser can parse current CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE / TIME.
    do
    {
        const char * date      = CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_DATE;
        const char * timeOfDay = CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_TIME;

        // Check that strings look good.
        EXPECT_FALSE(BUILD_DATE_IS_BAD(date));
        EXPECT_FALSE(BUILD_TIME_IS_BAD(timeOfDay));
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
            EXPECT_GT(printed, 0);
            EXPECT_LT(printed, static_cast<int>(sizeof(parsedDate)));
        }

        // Print the time of day to a straing as would be given by the __TIME__ macro.
        char parsedTimeOfDay[12] = { 0 }; // strlen("000:000:000") == 11
        {
            int printed;
            printed = SnprintfBuildTimeOfDay(parsedTimeOfDay, sizeof(parsedTimeOfDay), hour, minute, second);
            EXPECT_GT(printed, 0);
            EXPECT_LT(printed, static_cast<int>(sizeof(parsedTimeOfDay)));
        }

        // Verify match.
        EXPECT_STREQ(date, parsedDate);
        EXPECT_STREQ(timeOfDay, parsedTimeOfDay);
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
            EXPECT_GT(printed, 0);
            EXPECT_LT(printed, static_cast<int>(sizeof(date)));
        }

        // Print the time of day to a straing as would be given by the __TIME__ macro.
        {
            int printed;
            printed = SnprintfBuildTimeOfDay(timeOfDay, sizeof(timeOfDay), chipEpochTime);
            EXPECT_GT(printed, 0);
            EXPECT_LT(printed, static_cast<int>(sizeof(timeOfDay)));
        }

        // Check that strings look good.
        EXPECT_FALSE(BUILD_DATE_IS_BAD(date));
        EXPECT_FALSE(BUILD_TIME_IS_BAD(timeOfDay));
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
        EXPECT_EQ(year, COMPUTE_BUILD_YEAR(date));
        EXPECT_EQ(month, COMPUTE_BUILD_MONTH(date));
        EXPECT_EQ(day, COMPUTE_BUILD_DAY(date));
        EXPECT_EQ(hour, COMPUTE_BUILD_HOUR(timeOfDay));
        EXPECT_EQ(minute, COMPUTE_BUILD_MIN(timeOfDay));
        EXPECT_EQ(second, COMPUTE_BUILD_SEC(timeOfDay));
    }
}

TEST_F(TestConfigurationMgr, CountryCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char buf[8];
    size_t countryCodeLen    = 0;
    const char * countryCode = "US";

    err = ConfigurationMgr().StoreCountryCode(countryCode, strlen(countryCode));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = ConfigurationMgr().GetCountryCode(buf, 8, countryCodeLen);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(countryCodeLen, strlen(countryCode));
    EXPECT_STREQ(buf, countryCode);
}

TEST_F(TestConfigurationMgr, GetPrimaryMACAddress)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t macBuffer8Bytes[8];
    uint8_t macBuffer6Bytes[6];
    MutableByteSpan mac8Bytes(macBuffer8Bytes);
    MutableByteSpan mac6Bytes(macBuffer6Bytes);

    err = ConfigurationMgr().GetPrimaryMACAddress(mac8Bytes);
    if (sizeof(macBuffer8Bytes) != ConfigurationManager::kPrimaryMACAddressLength)
    {
        // Should have failed input validation
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }
    else if (mac8Bytes.size() != ConfigurationManager::kPrimaryMACAddressLength)
    {
        // This can happen if the primary address is Thread but then there is no
        // Thread address to be had and we fell back to Wi-Fi.
        EXPECT_EQ(mac8Bytes.size(), ConfigurationManager::kEthernetMACAddressLength);
    }

    err = ConfigurationMgr().GetPrimaryMACAddress(mac6Bytes);
    if (mac6Bytes.size() != ConfigurationManager::kPrimaryMACAddressLength)
    {
        EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
    }

    // NOTICE for above:
    //   no validation for CHIP_NO_ERROR:
    //    - there is no guarantee in CI that a valid IP address exists,
    //      expecially if running in emulators (zephyr and qemu)
}

TEST_F(TestConfigurationMgr, GetFailSafeArmed)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    bool failSafeArmed = false;

    err = ConfigurationMgr().SetFailSafeArmed(true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = ConfigurationMgr().GetFailSafeArmed(failSafeArmed);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(failSafeArmed, true);

    err = ConfigurationMgr().SetFailSafeArmed(false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestConfigurationMgr, GetVendorName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char buf[64];

    err = GetDeviceInstanceInfoProvider()->GetVendorName(buf, 64);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_GT(strlen(buf), 0u);
    EXPECT_LE(strlen(buf), ConfigurationManager::kMaxVendorNameLength);
}

TEST_F(TestConfigurationMgr, GetVendorId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t vendorId;

    err = GetDeviceInstanceInfoProvider()->GetVendorId(vendorId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_GE(vendorId, 0u);
    EXPECT_LE(vendorId, 0xfff4);
}

TEST_F(TestConfigurationMgr, GetProductName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char buf[64];

    err = GetDeviceInstanceInfoProvider()->GetProductName(buf, 64);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_GT(strlen(buf), 0u);
    EXPECT_LE(strlen(buf), ConfigurationManager::kMaxProductNameLength);
}

TEST_F(TestConfigurationMgr, GetProductId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t productId;

    err = GetDeviceInstanceInfoProvider()->GetProductId(productId);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_GE(productId, 1u);
    EXPECT_LE(productId, 0xffff);
}

} // namespace
