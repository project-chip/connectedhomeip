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

#include <app/clusters/resource-monitoring-server/ResourceMonitoringCluster.h>

#include <app/clusters/testing/AttributeTesting.h>

#include <functional>
#include <optional>
#include <type_traits>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

namespace {

template <typename OPT, typename... ARGS>
auto dispatch_to_optional_fn(OPT const & opt, ARGS &&... args) -> decltype(std::invoke(*opt, std::forward<ARGS>(args)...))
{
    using ReturnType = decltype(std::invoke(*opt, std::forward<ARGS>(args)...));

    if constexpr (std::is_invocable_v<std::decay_t<decltype(*opt)>, ARGS...>)
    {

        if (opt.has_value())
        {

            return std::invoke(*opt, std::forward<ARGS>(args)...);
        }
    }

    return ReturnType{};
}

struct MockDelegate : public ResourceMonitoring::Delegate
{
    CHIP_ERROR Init() { return dispatch_to_optional_fn(mInit); };

    Protocols::InteractionModel::Status OnResetCondition() { return dispatch_to_optional_fn(mOnResetCondition); };

    Protocols::InteractionModel::Status PreResetCondition() { return dispatch_to_optional_fn(mPreResetCondition); };

    Protocols::InteractionModel::Status PostResetCondition() { return dispatch_to_optional_fn(mPostResetCondition); };

    std::optional<std::function<CHIP_ERROR()>> mInit;
    std::optional<std::function<Protocols::InteractionModel::Status()>> mOnResetCondition;
    std::optional<std::function<Protocols::InteractionModel::Status()>> mPreResetCondition;
    std::optional<std::function<Protocols::InteractionModel::Status()>> mPostResetCondition;
};

struct TestResourceMonitoringCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    MockDelegate mDelegate;
};

// constexpr EndpointId kEndpointWitResourceMonitoring = 1;

using namespace HepaFilterMonitoring::Attributes;

TEST_F(TestResourceMonitoringCluster, AttributeTest) {}

} // namespace
