/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <lib/core/TLVReader.h>
#include <pw_unit_test/framework.h>

#include <type_traits>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

using Protocols::InteractionModel::Status;

class TestCommodityTariffCommands : public ::testing::Test
{
protected:
    template <typename Request>
    Status Invoke(Instance & instance, Delegate & delegate, CommandId commandId, const Request & request)
    {
        instance.mServerTariffAttrsCtx.mTariffProvider = &delegate;

        Testing::MockCommandHandler commandHandler;
        ConcreteCommandPath commandPath(1, Id, commandId);
        TLV::TLVReader reader;
        CommandHandlerInterface::HandlerContext context(commandHandler, commandPath, reader);

        if constexpr (std::is_same_v<Request, Commands::GetTariffComponent::DecodableType>)
        {
            instance.HandleGetTariffComponent(context, request);
        }
        else
        {
            instance.HandleGetDayEntry(context, request);
        }

        EXPECT_TRUE(commandHandler.HasStatus());
        EXPECT_FALSE(commandHandler.HasResponse());
        return commandHandler.GetLastStatus().status.GetStatus();
    }
};

TEST_F(TestCommodityTariffCommands, GetTariffComponentReturnsNotFoundForNullList)
{
    Delegate delegate;
    Instance instance(1, delegate, Feature::kPricing);
    Commands::GetTariffComponent::DecodableType request;
    request.tariffComponentID = 1;

    EXPECT_EQ(Invoke(instance, delegate, Commands::GetTariffComponent::Id, request), Status::NotFound);
}

TEST_F(TestCommodityTariffCommands, GetDayEntryReturnsNotFoundForNullList)
{
    Delegate delegate;
    Instance instance(1, delegate, Feature::kPricing);
    Commands::GetDayEntry::DecodableType request;
    request.dayEntryID = 1;

    EXPECT_EQ(Invoke(instance, delegate, Commands::GetDayEntry::Id, request), Status::NotFound);
}

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
