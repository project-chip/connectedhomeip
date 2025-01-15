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

#pragma once

/// test-interaction-model-api was created to consolidate and centralize stub functions that are used by the Interaction Model
/// during unit-testing.

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/WriteHandler.h>
#include <app/data-model-provider/Provider.h>
#include <app/util/attribute-storage.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

/**
 * Helper macro we can use to pretend we got a reply from the server in cases
 * when the reply was actually dropped due to us not wanting the client's state
 * machine to advance.
 *
 * When this macro is used, the client has sent a message and is waiting for an
 * ack+response, and the server has sent a response that got dropped and is
 * waiting for an ack (and maybe a response).
 *
 * What this macro then needs to do is:
 *
 * 1. Pretend that the client got an ack (and clear out the corresponding ack
 *    state).
 * 2. Pretend that the client got a message from the server, with the id of the
 *    message that was dropped, which requires an ack, so the client will send
 *    that ack in its next message.
 *
 * a macro was chosen so that we get useful line numbers on assertion failures
 *
 * This macro is used by the code in TestWriteInteraction.cpp, TestReadInteraction.cpp
 *  and TestCommandInteraction.cpp
 */
#define PretendWeGotReplyFromServer(aContext, aClientExchange)                                                                     \
    {                                                                                                                              \
        Messaging::ReliableMessageMgr * localRm    = (aContext).GetExchangeManager().GetReliableMessageMgr();                      \
        Messaging::ExchangeContext * localExchange = aClientExchange;                                                              \
        EXPECT_EQ(localRm->TestGetCountRetransTable(), 2);                                                                         \
                                                                                                                                   \
        localRm->ClearRetransTable(localExchange);                                                                                 \
        EXPECT_EQ(localRm->TestGetCountRetransTable(), 1);                                                                         \
                                                                                                                                   \
        localRm->EnumerateRetransTable([localExchange](auto * entry) {                                                             \
            localExchange->SetPendingPeerAckMessageCounter(entry->retainedBuf.GetMessageCounter());                                \
            return Loop::Break;                                                                                                    \
        });                                                                                                                        \
    }

namespace chip {
namespace Test {

constexpr chip::ClusterId kTestDeniedClusterId1  = 1000;
constexpr chip::ClusterId kTestDeniedClusterId2  = 3;
constexpr chip::ClusterId kTestClusterId         = 6;
constexpr uint8_t kTestFieldValue1               = 1;
constexpr chip::EndpointId kTestEndpointId       = 1;
constexpr chip::DataVersion kTestDataVersion1    = 3;
constexpr chip::DataVersion kRejectedDataVersion = 1;

extern uint8_t attributeDataTLV[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
extern size_t attributeDataTLVLen;

} // namespace Test
namespace app {

bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion);

void DispatchSingleClusterCommand(const ConcreteCommandPath & aRequestCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj);

/// A customized class for read/write/invoke that matches functionality
/// with the ember-compatibility-functions functionality here.
///
/// TODO: these functions currently redirect to ember functions, so could
///       be merged with DataModelFixtures.h/cpp as well. This is not done since
///       if we remove the direct ember dependency from IM, we can implement
///       distinct functional classes.
/// TODO items for above:
///      - once IM only supports DataModel
///      - break ember-overrides in this h/cpp file
class TestImCustomDataModel : public CodegenDataModelProvider
{
public:
    static TestImCustomDataModel & Instance();

    CHIP_ERROR Shutdown() override { return CHIP_NO_ERROR; }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> Invoke(const DataModel::InvokeRequest & request,
                                                        chip::TLV::TLVReader & input_arguments, CommandHandler * handler) override;
};

} // namespace app
} // namespace chip
