/*
 *
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

#include <app/icd/server/DefaultICDCheckInBackOffStrategy.h>
#include <app/icd/server/ICDMonitoringTable.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/ClusterEnums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::IcdManagement;

using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

namespace {

TEST(TestDefaultICDCheckInBackOffStrategy, TestShouldSendCheckInMessagePermanentClient)
{
    TestSessionKeystoreImpl keystore;
    ICDMonitoringEntry entry(&keystore);

    entry.clientType = ClientTypeEnum::kPermanent;

    DefaultICDCheckInBackOffStrategy strategy;
    EXPECT_TRUE(strategy.ShouldSendCheckInMessage(entry));
}

TEST(TestDefaultICDCheckInBackOffStrategy, TestShouldSendCheckInMessageEphemeralClient)
{
    TestSessionKeystoreImpl keystore;
    ICDMonitoringEntry entry(&keystore);

    entry.clientType = ClientTypeEnum::kEphemeral;

    DefaultICDCheckInBackOffStrategy strategy;
    EXPECT_FALSE(strategy.ShouldSendCheckInMessage(entry));
}

} // namespace
