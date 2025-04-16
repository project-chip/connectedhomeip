/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/ota-provider/ota-provider-cluster.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

namespace {

static constexpr chip::EndpointId kRootEndpointId = 0;

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;
using chip::Protocols::InteractionModel::Status;

class MockDelegate : public OTAProviderDelegate
{
public:
    void HandleQueryImage(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                          const OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) override
    {}

    void
    HandleApplyUpdateRequest(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             const OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) override
    {}

    void
    HandleNotifyUpdateApplied(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) override
    {}
};

TEST(TestOtaProviderLogic, QueryImageValidation)
{
    OtaProviderLogic logic;
    const ConcreteCommandPath kCommandPath{ kRootEndpointId, OtaSoftwareUpdateProvider::Id, QueryImage::Id };

    QueryImage::DecodableType input;

    // without a delegate, command is unsupported
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), Status::UnsupportedCommand);

    MockDelegate mockDelegate;
    logic.SetDelegate(&mockDelegate);

    // location MUST be 2 bytes
    input.location = MakeOptional("abc_too_large"_span);
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), Status::InvalidCommand);

    // valid location (empty is ok)
    input.location = NullOptional;
    EXPECT_EQ(logic.QueryImage(kCommandPath, input, nullptr /* handler */), std::nullopt);
    // nullopt means delegate was called
}

} // namespace
