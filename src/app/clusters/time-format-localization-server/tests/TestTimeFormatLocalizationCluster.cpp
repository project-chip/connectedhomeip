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

#include <app/clusters/time-format-localization-server/TimeFormatLocalizationCluster.h>

#include <app/ConcreteClusterPath.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <clusters/TimeFormatLocalization/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <clusters/TimeFormatLocalization/Attributes.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>

#include "SampleDeviceProvider.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Testing::IsAttributesListEqualTo;

namespace {

struct TestTimeFormatLocalizationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestTimeFormatLocalizationCluster, AttributeTest)
{
    {
        BitFlags<TimeFormatLocalization::Feature> features{ 0 };

        TimeFormatLocalizationCluster onlyMandatory(kRootEndpointId, features, TimeFormatLocalization::HourFormatEnum::k12hr,
                                                    TimeFormatLocalization::CalendarTypeEnum::kBuddhist);

        // Test attributes listing with no features enabled
        ASSERT_TRUE(IsAttributesListEqualTo(onlyMandatory,
                                            {
                                                TimeFormatLocalization::Attributes::HourFormat::kMetadataEntry,
                                            }));
    }

    {
        BitFlags<TimeFormatLocalization::Feature> features{ TimeFormatLocalization::Feature::kCalendarFormat };

        TimeFormatLocalizationCluster withCalendarFeature(kRootEndpointId, features, TimeFormatLocalization::HourFormatEnum::k12hr,
                                                          TimeFormatLocalization::CalendarTypeEnum::kBuddhist);

        // Test attributes listing with CalendarFormat feature enabled
        ASSERT_TRUE(IsAttributesListEqualTo(withCalendarFeature,
                                            {
                                                TimeFormatLocalization::Attributes::HourFormat::kMetadataEntry,
                                                TimeFormatLocalization::Attributes::ActiveCalendarType::kMetadataEntry,
                                                TimeFormatLocalization::Attributes::SupportedCalendarTypes::kMetadataEntry,
                                            }));
    }
}

} // namespace
