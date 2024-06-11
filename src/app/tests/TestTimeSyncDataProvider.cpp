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
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <pw_unit_test/framework.h>

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

class TestTimeSyncDataProvider : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestTimeSyncDataProvider, TestTrustedTimeSourceStoreLoad)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TrustedTimeSource tts = { chip::FabricIndex(1), chip::NodeId(20), chip::EndpointId(0) };

    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.StoreTrustedTimeSource(tts));

    TrustedTimeSource retrievedTrustedTimeSource;
    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.LoadTrustedTimeSource(retrievedTrustedTimeSource));
    EXPECT_EQ(retrievedTrustedTimeSource.fabricIndex, chip::FabricIndex(1));
    EXPECT_EQ(retrievedTrustedTimeSource.nodeID, chip::NodeId(20));
    EXPECT_EQ(retrievedTrustedTimeSource.endpoint, chip::EndpointId(0));
}

TEST_F(TestTimeSyncDataProvider, TestTrustedTimeSourceEmpty)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TrustedTimeSource tts;

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, timeSyncDataProv.LoadTrustedTimeSource(tts));
}

TEST_F(TestTimeSyncDataProvider, TestDefaultNTPStoreLoad)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    char ntp[10] = "localhost";
    chip::CharSpan defaultNTP(ntp);

    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.StoreDefaultNtp(defaultNTP));

    char buf[5];
    chip::MutableCharSpan getDefaultNtp(buf);

    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, timeSyncDataProv.LoadDefaultNtp(getDefaultNtp));
    EXPECT_EQ(getDefaultNtp.size(), 5u);

    char buf1[20];
    chip::MutableCharSpan getDefaultNtp1(buf1);

    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.LoadDefaultNtp(getDefaultNtp1));
    EXPECT_EQ(getDefaultNtp1.size(), 10u);
}

TEST_F(TestTimeSyncDataProvider, TestDefaultNTPEmpty)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    chip::MutableCharSpan defaultNTP;

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, timeSyncDataProv.LoadDefaultNtp(defaultNTP));
}

TEST_F(TestTimeSyncDataProvider, TestTimeZoneStoreLoad)
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
    EXPECT_EQ(tzL.size(), 3u);
    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.StoreTimeZone(tzL));

    TimeSyncDataProvider::TimeZoneStore emptyTzS[3] = { makeTimeZone(), makeTimeZone(), makeTimeZone() };

    tzL = TimeZoneList(emptyTzS);
    TimeSyncDataProvider::TimeZoneObj tzObj{ tzL, 3 };
    EXPECT_EQ(tzL.size(), 3u);
    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.LoadTimeZone(tzObj));
    EXPECT_EQ(tzObj.validSize, 3u);

    EXPECT_FALSE(tzL.empty());

    if (!tzL.empty())
    {
        auto & tz = tzL[0].timeZone;
        EXPECT_EQ(tz.offset, 1);
        EXPECT_EQ(tz.validAt, 1u);
        EXPECT_TRUE(tz.name.HasValue());
        EXPECT_EQ(tz.name.Value().size(), 2u);

        tzL = tzL.SubSpan(1);
    }

    if (!tzL.empty())
    {
        auto & tz = tzL[0].timeZone;
        EXPECT_EQ(tz.offset, 2);
        EXPECT_EQ(tz.validAt, 2u);
        EXPECT_TRUE(tz.name.HasValue());
        EXPECT_EQ(tz.name.Value().size(), 63u);

        tzL = tzL.SubSpan(1);
    }

    if (!tzL.empty())
    {
        auto & tz = tzL[0].timeZone;
        EXPECT_EQ(tz.offset, 3);
        EXPECT_EQ(tz.validAt, 3u);
        EXPECT_TRUE(tz.name.HasValue());
        EXPECT_EQ(tz.name.Value().size(), 6u);

        tzL = tzL.SubSpan(1);
    }

    EXPECT_TRUE(tzL.empty());
}

TEST_F(TestTimeSyncDataProvider, TestTimeZoneEmpty)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TimeSyncDataProvider::TimeZoneObj timeZoneObj;

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, timeSyncDataProv.LoadTimeZone(timeZoneObj));
    EXPECT_FALSE(timeZoneObj.timeZoneList.begin());
    EXPECT_EQ(timeZoneObj.validSize, 0u);
}

TEST_F(TestTimeSyncDataProvider, TestDSTOffset)
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
    EXPECT_EQ(dstObj.validSize, 3u);
    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.StoreDSTOffset(dstL));

    DSTOffset emtpyDstS[3] = { makeDSTOffset(), makeDSTOffset(), makeDSTOffset() };

    dstObj.dstOffsetList = DSTOffsetList(emtpyDstS);
    dstObj.validSize     = 0;
    EXPECT_EQ(dstObj.dstOffsetList.size(), 3u);
    EXPECT_EQ(CHIP_NO_ERROR, timeSyncDataProv.LoadDSTOffset(dstObj));
    EXPECT_EQ(dstObj.validSize, 3u);

    EXPECT_FALSE(dstObj.dstOffsetList.empty());

    if (!dstObj.dstOffsetList.empty())
    {
        auto & dst = dstObj.dstOffsetList.data()[0];
        EXPECT_EQ(dst.offset, 1);
        EXPECT_EQ(dst.validStarting, 1u);
        EXPECT_FALSE(dst.validUntil.IsNull());
        EXPECT_EQ(dst.validUntil.Value(), 2u);

        dstObj.dstOffsetList = dstObj.dstOffsetList.SubSpan(1);
    }

    if (!dstObj.dstOffsetList.empty())
    {
        auto & dst = dstObj.dstOffsetList.data()[0];
        EXPECT_EQ(dst.offset, 2);
        EXPECT_EQ(dst.validStarting, 2u);
        EXPECT_FALSE(dst.validUntil.IsNull());
        EXPECT_EQ(dst.validUntil.Value(), 3u);

        dstObj.dstOffsetList = dstObj.dstOffsetList.SubSpan(1);
    }

    if (!dstObj.dstOffsetList.empty())
    {
        auto & dst = dstObj.dstOffsetList.data()[0];
        EXPECT_EQ(dst.offset, 3);
        EXPECT_EQ(dst.validStarting, 3u);
        EXPECT_TRUE(dst.validUntil.IsNull());

        dstObj.dstOffsetList = dstObj.dstOffsetList.SubSpan(1);
    }

    EXPECT_TRUE(dstObj.dstOffsetList.empty());
}

TEST_F(TestTimeSyncDataProvider, TestDSTOffsetEmpty)
{
    TestPersistentStorageDelegate persistentStorage;
    TimeSyncDataProvider timeSyncDataProv;
    timeSyncDataProv.Init(persistentStorage);

    TimeSyncDataProvider::DSTOffsetObj dstObj;

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, timeSyncDataProv.LoadDSTOffset(dstObj));
    EXPECT_TRUE(dstObj.dstOffsetList.empty());
    EXPECT_EQ(dstObj.validSize, 0u);
}

} // namespace
