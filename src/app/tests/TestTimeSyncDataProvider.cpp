/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/time-synchronization-server/TimeSyncDataProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::TimeSynchronization;
using chip::TimeSyncDataProvider;

using TrustedTimeSource = app::Clusters::TimeSynchronization::Structs::TrustedTimeSourceStruct::Type;
using TimeZoneList      = Span<TimeSyncDataProvider::TimeSyncDataProvider::TimeZoneStore>;
using TimeZone          = app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type;
using DSTOffsetList     = app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type>;
using DSTOffset         = app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type;

namespace {

void TestTrustedTimeSourceStoreLoad(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TrustedTimeSource tts = { chip::FabricIndex(1), chip::NodeId(20), chip::EndpointId(0) };

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.StoreTrustedTimeSource(tts));

    TrustedTimeSource retrievedTrustedTimeSource;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.LoadTrustedTimeSource(retrievedTrustedTimeSource));
    NL_TEST_ASSERT(inSuite, retrievedTrustedTimeSource.fabricIndex == chip::FabricIndex(1));
    NL_TEST_ASSERT(inSuite, retrievedTrustedTimeSource.nodeID == chip::NodeId(20));
    NL_TEST_ASSERT(inSuite, retrievedTrustedTimeSource.endpoint == chip::EndpointId(0));
}

void TestTrustedTimeSourceEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TrustedTimeSource tts;

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == timeSyncDataProv.LoadTrustedTimeSource(tts));
}

void TestDefaultNTPStoreLoad(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    char ntp[10] = "localhost";
    chip::CharSpan defaultNTP(ntp);

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.StoreDefaultNtp(defaultNTP));

    char buf[5];
    chip::MutableCharSpan getDefaultNtp(buf);

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_BUFFER_TOO_SMALL == timeSyncDataProv.LoadDefaultNtp(getDefaultNtp));
    NL_TEST_ASSERT(inSuite, getDefaultNtp.size() == 5);

    char buf1[20];
    chip::MutableCharSpan getDefaultNtp1(buf1);

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.LoadDefaultNtp(getDefaultNtp1));
    NL_TEST_ASSERT(inSuite, getDefaultNtp1.size() == 10);
}

void TestDefaultNTPEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    chip::MutableCharSpan defaultNTP;

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == timeSyncDataProv.LoadDefaultNtp(defaultNTP));
}

void TestTimeZoneStoreLoad(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    const auto makeTimeZone = [](int32_t offset = 0, uint64_t validAt = 0, char * name = nullptr) {
        TimeSyncDataProvider::TimeZoneStore tzS;
        tzS.timeZone.offset  = offset;
        tzS.timeZone.validAt = validAt;
        if (name != nullptr)
        {
            Platform::CopyString(tzS.name, name);
            tzS.timeZone.name.SetValue(chip::CharSpan::fromCharString(tzS.name));
        }
        return tzS;
    };
    char tzShort[]                             = "LA";
    char tzLong[]                              = "MunichOnTheLongRiverOfIsarInNiceSummerWeatherWithAugustinerBeer";
    char tzBerlin[]                            = "Berlin";
    TimeSyncDataProvider::TimeZoneStore tzS[3] = { makeTimeZone(1, 1, tzShort), makeTimeZone(2, 2, tzLong),
                                                   makeTimeZone(3, 3, tzBerlin) };
    TimeZoneList tzL(tzS);
    NL_TEST_ASSERT(inSuite, tzL.size() == 3);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.StoreTimeZone(tzL));

    TimeSyncDataProvider::TimeZoneStore emptyTzS[3] = { makeTimeZone(), makeTimeZone(), makeTimeZone() };

    tzL = TimeZoneList(emptyTzS);
    TimeSyncDataProvider::TimeZoneObj tzObj{ tzL, 3 };
    NL_TEST_ASSERT(inSuite, tzL.size() == 3);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.LoadTimeZone(tzObj));
    NL_TEST_ASSERT(inSuite, tzObj.validSize == 3);

    NL_TEST_ASSERT(inSuite, !tzL.empty());

    if (!tzL.empty())
    {
        auto & tz = tzL[0].timeZone;
        NL_TEST_ASSERT(inSuite, tz.offset == 1);
        NL_TEST_ASSERT(inSuite, tz.validAt == 1);
        NL_TEST_ASSERT(inSuite, tz.name.HasValue());
        NL_TEST_ASSERT(inSuite, tz.name.Value().size() == 2);

        tzL = tzL.SubSpan(1);
    }

    if (!tzL.empty())
    {
        auto & tz = tzL[0].timeZone;
        NL_TEST_ASSERT(inSuite, tz.offset == 2);
        NL_TEST_ASSERT(inSuite, tz.validAt == 2);
        NL_TEST_ASSERT(inSuite, tz.name.HasValue());
        NL_TEST_ASSERT(inSuite, tz.name.Value().size() == 63);

        tzL = tzL.SubSpan(1);
    }

    if (!tzL.empty())
    {
        auto & tz = tzL[0].timeZone;
        NL_TEST_ASSERT(inSuite, tz.offset == 3);
        NL_TEST_ASSERT(inSuite, tz.validAt == 3);
        NL_TEST_ASSERT(inSuite, tz.name.HasValue());
        NL_TEST_ASSERT(inSuite, tz.name.Value().size() == 6);

        tzL = tzL.SubSpan(1);
    }

    NL_TEST_ASSERT(inSuite, tzL.empty());
}

void TestTimeZoneEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TimeSyncDataProvider::TimeZoneObj timeZoneObj;

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == timeSyncDataProv.LoadTimeZone(timeZoneObj));
    NL_TEST_ASSERT(inSuite, !timeZoneObj.timeZoneList.begin());
    NL_TEST_ASSERT(inSuite, timeZoneObj.validSize == 0);
}

void TestDSTOffset(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    const auto makeDSTOffset = [](int32_t offset = 0, uint64_t validStarting = 0, uint64_t validUntil = 0) {
        DSTOffset dst;
        dst.offset        = offset;
        dst.validStarting = validStarting;
        if (validUntil)
            dst.validUntil.SetNonNull(validUntil);
        return dst;
    };
    DSTOffset dstS[3] = { makeDSTOffset(1, 1, 2), makeDSTOffset(2, 2, 3), makeDSTOffset(3, 3) };
    DSTOffsetList dstL(dstS);
    TimeSyncDataProvider::DSTOffsetObj dstObj{ dstL, 3 };
    NL_TEST_ASSERT(inSuite, dstObj.validSize == 3);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.StoreDSTOffset(dstL));

    DSTOffset emtpyDstS[3] = { makeDSTOffset(), makeDSTOffset(), makeDSTOffset() };

    dstObj.dstOffsetList = DSTOffsetList(emtpyDstS);
    dstObj.validSize     = 0;
    NL_TEST_ASSERT(inSuite, dstObj.dstOffsetList.size() == 3);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == timeSyncDataProv.LoadDSTOffset(dstObj));
    NL_TEST_ASSERT(inSuite, dstObj.validSize == 3);

    NL_TEST_ASSERT(inSuite, !dstObj.dstOffsetList.empty());

    if (!dstObj.dstOffsetList.empty())
    {
        auto & dst = dstObj.dstOffsetList.data()[0];
        NL_TEST_ASSERT(inSuite, dst.offset == 1);
        NL_TEST_ASSERT(inSuite, dst.validStarting == 1);
        NL_TEST_ASSERT(inSuite, !dst.validUntil.IsNull());
        NL_TEST_ASSERT(inSuite, dst.validUntil.Value() == 2);

        dstObj.dstOffsetList = dstObj.dstOffsetList.SubSpan(1);
    }

    if (!dstObj.dstOffsetList.empty())
    {
        auto & dst = dstObj.dstOffsetList.data()[0];
        NL_TEST_ASSERT(inSuite, dst.offset == 2);
        NL_TEST_ASSERT(inSuite, dst.validStarting == 2);
        NL_TEST_ASSERT(inSuite, !dst.validUntil.IsNull());
        NL_TEST_ASSERT(inSuite, dst.validUntil.Value() == 3);

        dstObj.dstOffsetList = dstObj.dstOffsetList.SubSpan(1);
    }

    if (!dstObj.dstOffsetList.empty())
    {
        auto & dst = dstObj.dstOffsetList.data()[0];
        NL_TEST_ASSERT(inSuite, dst.offset == 3);
        NL_TEST_ASSERT(inSuite, dst.validStarting == 3);
        NL_TEST_ASSERT(inSuite, dst.validUntil.IsNull());

        dstObj.dstOffsetList = dstObj.dstOffsetList.SubSpan(1);
    }

    NL_TEST_ASSERT(inSuite, dstObj.dstOffsetList.empty());
}

void TestDSTOffsetEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TimeSyncDataProvider::DSTOffsetObj dstObj;

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == timeSyncDataProv.LoadDSTOffset(dstObj));
    NL_TEST_ASSERT(inSuite, !dstObj.dstOffsetList.begin());
    NL_TEST_ASSERT(inSuite, dstObj.validSize == 0);
}

const nlTest sTests[] = { NL_TEST_DEF("Test TrustedTimeSource store load", TestTrustedTimeSourceStoreLoad),
                          NL_TEST_DEF("Test TrustedTimeSource empty", TestTrustedTimeSourceEmpty),
                          NL_TEST_DEF("Test default NTP store load", TestDefaultNTPStoreLoad),
                          NL_TEST_DEF("Test default NTP empty", TestDefaultNTPEmpty),
                          NL_TEST_DEF("Test time zone store load", TestTimeZoneStoreLoad),
                          NL_TEST_DEF("Test time zone (empty list)", TestTimeZoneEmpty),
                          NL_TEST_DEF("Test DSTOffset", TestDSTOffset),
                          NL_TEST_DEF("Test DSTOffset (empty list)", TestDSTOffsetEmpty),
                          NL_TEST_SENTINEL() };

int TestSetup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);
    return SUCCESS;
}

int TestTearDown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

int TestTimeSyncDataProvider()
{
    nlTestSuite theSuite = { "Time Sync data provider tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTimeSyncDataProvider)
