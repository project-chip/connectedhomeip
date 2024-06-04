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

#pragma once

#include <app/ReadClient.h>

namespace chip {
namespace Test {

/**
 * @brief Class acts as an accessor to private methods of the ReadClient class without needing to give friend access to
 *        each individual test.
 *        This is not a Global API and should only be used for (Unit) Testing.
 */

using namespace app;

class ReadClientTestAccess
{

public:
    ReadClientTestAccess(ReadClient * aReadClient) : mpReadClient(aReadClient) {}
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload)
    {
        return mpReadClient->OnMessageReceived(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }

    Messaging::ExchangeHolder & GetExchange() { return mpReadClient->mExchange; }
    Messaging::ExchangeManager * GetExchangeMgr() { return mpReadClient->mpExchangeMgr; }
    CHIP_ERROR ProcessReportData(System::PacketBufferHandle && aPayload, ReadClient::ReportType aReportType)
    {
        return mpReadClient->ProcessReportData(std::move(aPayload), aReportType);
    }

    CHIP_ERROR GenerateAttributePaths(AttributePathIBs::Builder & aAttributePathIBsBuilder,
                                      const Span<AttributePathParams> & aAttributePaths)
    {
        return mpReadClient->GenerateAttributePaths(aAttributePathIBsBuilder, aAttributePaths);
    }

    CHIP_ERROR GenerateEventPaths(EventPathIBs::Builder & aEventPathsBuilder, const Span<EventPathParams> & aEventPaths)
    {
        return mpReadClient->GenerateEventPaths(aEventPathsBuilder, aEventPaths);
    }

    CHIP_ERROR SendSubscribeRequest(const ReadPrepareParams & aSubscribePrepareParams)
    {
        return mpReadClient->SendSubscribeRequest(aSubscribePrepareParams);
    }

    SubscriptionId GetSubscriptionId() { return mpReadClient->mSubscriptionId; }
    void MoveToState(const ReadClient::ClientState aTargetState) { mpReadClient->MoveToState(aTargetState); }

    bool IsIdle() const { return mpReadClient->IsIdle(); }

private:
    ReadClient * mpReadClient = nullptr;
};

} // namespace Test
} // namespace chip
