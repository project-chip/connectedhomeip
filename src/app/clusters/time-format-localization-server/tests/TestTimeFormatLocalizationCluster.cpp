/*
 *    Copyright (c) 2025 Project CHIP Authors
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

 #include <pw_unit_test/framework.h>

#include <app/clusters/time-format-localization-server/time-format-localization-logic.h>

#include <lib/support/ReadOnlyBuffer.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <clusters/TimeFormatLocalization/Metadata.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>

#include <clusters/TimeFormatLocalization/Attributes.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>

#include "SampleDeviceProvider.h"


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

struct TestTimeFormatLocalizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestTimeFormatLocalizationCluster, AttributeTest)
{
    {
        BitFlags<TimeFormatLocalization::Feature> features {0};

        TimeFormatLocalizationLogic onlyMandatory(features);

        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commandsBuilder;

        // No features enabled
        ASSERT_EQ(onlyMandatory.GetFeatureMap(), BitFlags<TimeFormatLocalization::Feature>{0});

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(onlyMandatory.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedAttributes;
        ASSERT_EQ(expectedAttributes.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedAttributes.AppendElements({TimeFormatLocalization::Attributes::HourFormat::kMetadataEntry}), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedAttributes.TakeBuffer()));
    }

    {
        // No attributes enabled
         BitFlags<TimeFormatLocalization::Feature> features {0};
         features.Set(TimeFormatLocalization::Feature::kCalendarFormat);

        TimeFormatLocalizationLogic onlyMandatory(features);

        // CalendarFormat feature enabled.
        ASSERT_EQ(onlyMandatory.GetFeatureMap(), BitFlags<TimeFormatLocalization::Feature>{TimeFormatLocalization::Feature::kCalendarFormat});

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
        ASSERT_EQ(onlyMandatory.Attributes(attributesBuilder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedAttributes;
        ASSERT_EQ(expectedAttributes.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_EQ(expectedAttributes.AppendElements({TimeFormatLocalization::Attributes::HourFormat::kMetadataEntry}), CHIP_NO_ERROR);
        ASSERT_EQ(expectedAttributes.AppendElements({TimeFormatLocalization::Attributes::ActiveCalendarType::kMetadataEntry}), CHIP_NO_ERROR);
        ASSERT_EQ(expectedAttributes.AppendElements({TimeFormatLocalization::Attributes::SupportedCalendarTypes::kMetadataEntry}), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedAttributes.TakeBuffer()));
    }
}

TEST_F(TestTimeFormatLocalizationCluster, ListCalendarTest)
{
    // TODO: This seems to work to test the ListCalendar using a Encoder/Decoder with TLV. Probably there is an easier way.
    uint8_t buf[1024];
    TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buf);

    AttributeReportIBs::Builder builder;
    builder.Init(&tlvWriter);

    ConcreteAttributePath path(0, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id);
    ConcreteReadAttributePath readPath(path);
    DataVersion version{0};
    Access::SubjectDescriptor descriptor;
    AttributeValueEncoder encoder(builder, descriptor, path, version);

    BitFlags<TimeFormatLocalization::Feature> features {0};
    features.Set(TimeFormatLocalization::Feature::kCalendarFormat);

    // Save old provider
    DeviceLayer::DeviceInfoProvider * oldProvider = DeviceLayer::GetDeviceInfoProvider();
    // Set new SampleProvider
    DeviceLayer::SampleDeviceProvider testProvider;
    DeviceLayer::SetDeviceInfoProvider(&testProvider);

    TimeFormatLocalizationLogic clusterSim(features);

    EXPECT_EQ(clusterSim.GetSupportedCalendarTypes(encoder), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buf);

    TLV::TLVReader attrReportsReader;
    TLV::TLVReader attrReportReader;
    TLV::TLVReader attrDataReader;

    reader.Next();
    reader.OpenContainer(attrReportsReader);

    attrReportsReader.Next();
    attrReportsReader.OpenContainer(attrReportReader);

    attrReportReader.Next();
    attrReportReader.OpenContainer(attrDataReader);

    // We're now in the attribute data IB, skip to the desired tag, we want TagNum = 2
    attrDataReader.Next();
    for (int i = 0; i < 3 && !(IsContextTag(attrDataReader.GetTag()) && TagNumFromTag(attrDataReader.GetTag()) == 2); ++i)
    {
        attrDataReader.Next();
    }
    EXPECT_TRUE(IsContextTag(attrDataReader.GetTag()));
    EXPECT_EQ(TagNumFromTag(attrDataReader.GetTag()), 2u);

    TimeFormatLocalization::Attributes::SupportedCalendarTypes::TypeInfo::DecodableType list;
    TimeFormatLocalization::CalendarTypeEnum calendarType;
    CHIP_ERROR err = list.Decode(attrDataReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto iter = list.begin();

    EXPECT_TRUE(iter.Next());
    calendarType = iter.GetValue();
    EXPECT_EQ(calendarType, TimeFormatLocalization::CalendarTypeEnum::kGregorian);
    EXPECT_TRUE(iter.Next());
    calendarType = iter.GetValue();
    EXPECT_EQ(calendarType, TimeFormatLocalization::CalendarTypeEnum::kChinese);
    EXPECT_TRUE(iter.Next());
    calendarType = iter.GetValue();
    EXPECT_EQ(calendarType, TimeFormatLocalization::CalendarTypeEnum::kJapanese);

    // Revert provider to old state
    DeviceLayer::SetDeviceInfoProvider(oldProvider);
}

TEST_F(TestTimeFormatLocalizationCluster, WriteAttributes)
{
    class TestAttributeProvider : public AttributePersistenceProvider
    {
        public:
        TestAttributeProvider()
        {
            currentCalendar = TimeFormatLocalization::CalendarTypeEnum::kChinese;
            currentHourFormat = TimeFormatLocalization::HourFormatEnum::k24hr;
        }

        ~TestAttributeProvider() = default;

        CHIP_ERROR WriteValue(const ConcreteAttributePath & aPath, const ByteSpan & aValue)
        {
            ConcreteAttributePath hourPath = {0, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id };
            ConcreteAttributePath calendarPath = {0, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id };

            if(hourPath == aPath)
            {
                EXPECT_EQ(aValue.size(), sizeof(currentHourFormat));
                memcpy(&currentHourFormat, aValue.data(), sizeof(currentHourFormat));
            }

            if(calendarPath == aPath)
            {
                EXPECT_EQ(aValue.size(), sizeof(currentCalendar));
                memcpy(&currentCalendar, aValue.data(), sizeof(currentCalendar));
            }

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR ReadValue(const ConcreteAttributePath & aPath, MutableByteSpan & aValue)
        {
            ConcreteAttributePath hourPath = {0, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::HourFormat::Id };
            ConcreteAttributePath calendarPath = {0, TimeFormatLocalization::Id, TimeFormatLocalization::Attributes::ActiveCalendarType::Id };

            if(hourPath == aPath)
            {
                EXPECT_EQ(aValue.size(), sizeof(currentHourFormat));
                memcpy(aValue.data(), &currentHourFormat, sizeof(currentHourFormat));
            }

            if(calendarPath == aPath)
            {
                EXPECT_EQ(aValue.size(), sizeof(currentCalendar));
                memcpy(aValue.data(), &currentCalendar, sizeof(currentCalendar));
            }

            return CHIP_NO_ERROR;
        }

        private:
        TimeFormatLocalization::CalendarTypeEnum currentCalendar;
        TimeFormatLocalization::HourFormatEnum currentHourFormat;
    };

    BitFlags<TimeFormatLocalization::Feature> features {0};
    features.Set(TimeFormatLocalization::Feature::kCalendarFormat);
    // Save old provider
    DeviceLayer::DeviceInfoProvider * oldProvider = DeviceLayer::GetDeviceInfoProvider();
    // Set new SampleProvider
    DeviceLayer::SampleDeviceProvider testProvider;
    DeviceLayer::SetDeviceInfoProvider(&testProvider);

    TimeFormatLocalizationLogic clusterSim(features);

    TestAttributeProvider testAttrProvider;

    clusterSim.Startup(&testAttrProvider);

    EXPECT_EQ(clusterSim.GetHourFormat(), TimeFormatLocalization::HourFormatEnum::k24hr);
    EXPECT_EQ(clusterSim.GetActiveCalendarType(), TimeFormatLocalization::CalendarTypeEnum::kChinese);

    clusterSim.setHourFormat(TimeFormatLocalization::HourFormatEnum::k12hr);
    EXPECT_EQ(clusterSim.GetHourFormat(), TimeFormatLocalization::HourFormatEnum::k12hr);

    clusterSim.setActiveCalendarType(TimeFormatLocalization::CalendarTypeEnum::kGregorian);
    EXPECT_EQ(clusterSim.GetActiveCalendarType(), TimeFormatLocalization::CalendarTypeEnum::kGregorian);

    // Revert provider to old state
    DeviceLayer::SetDeviceInfoProvider(oldProvider);

    EXPECT_TRUE(true);
}
}