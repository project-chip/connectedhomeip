/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <app/ClusterStateCache.h>
#include <app/ReadHandler.h>
#include <app/tests/AppTestContext.h>

namespace chip {
namespace test {

class TestReadInteractionBase : public app::ReadHandler::ApplicationCallback
{
public:
    using TestContext = chip::Test::AppContext;

    TestReadInteractionBase() {}

    virtual ~TestReadInteractionBase() {}

protected:
    static constexpr uint16_t kTestMinInterval = 33;
    static constexpr uint16_t kTestMaxInterval = 66;

    CHIP_ERROR OnSubscriptionRequested(app::ReadHandler & aReadHandler, Transport::SecureSession & aSecureSession)
    {
        VerifyOrReturnError(!mEmitSubscriptionError, CHIP_ERROR_INVALID_ARGUMENT);

        if (mAlterSubscriptionIntervals)
        {
            ReturnErrorOnFailure(aReadHandler.SetReportingIntervals(kTestMinInterval, kTestMaxInterval));
        }
        return CHIP_NO_ERROR;
    }

    void OnSubscriptionEstablished(app::ReadHandler & aReadHandler) { mNumActiveSubscriptions++; }

    void OnSubscriptionTerminated(app::ReadHandler & aReadHandler) { mNumActiveSubscriptions--; }

    // Issue the given number of reads in parallel and wait for them all to
    // succeed.
    static void MultipleReadHelper(nlTestSuite * apSuite, TestContext & aCtx, size_t aReadCount);

    // Establish the given number of subscriptions, then issue the given number
    // of reads in parallel and wait for them all to succeed.
    static void SubscribeThenReadHelper(nlTestSuite * apSuite, TestContext & aCtx, size_t aSubscribeCount, size_t aReadCount);

    bool mEmitSubscriptionError      = false;
    int32_t mNumActiveSubscriptions  = 0;
    bool mAlterSubscriptionIntervals = false;
};

class MockInteractionModelApp : public chip::app::ClusterStateCache::Callback
{
public:
    void OnEventData(const chip::app::EventHeader & aEventHeader, chip::TLV::TLVReader * apData,
                     const chip::app::StatusIB * apStatus) override
    {}

    void OnAttributeData(const chip::app::ConcreteDataAttributePath & aPath, chip::TLV::TLVReader * apData,
                         const chip::app::StatusIB & status) override
    {
        if (status.mStatus == chip::Protocols::InteractionModel::Status::Success)
        {
            mNumAttributeResponse++;
        }
    }

    void OnError(CHIP_ERROR aError) override
    {
        mError     = aError;
        mReadError = true;
    }

    void OnDone() override {}

    void OnDeallocatePaths(chip::app::ReadPrepareParams && aReadPrepareParams) override
    {
        if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpAttributePathParamsList;
        }

        if (aReadPrepareParams.mpEventPathParamsList != nullptr)
        {
            delete[] aReadPrepareParams.mpEventPathParamsList;
        }

        if (aReadPrepareParams.mpDataVersionFilterList != nullptr)
        {
            delete[] aReadPrepareParams.mpDataVersionFilterList;
        }
    }

    int mNumAttributeResponse = 0;
    bool mReadError           = false;
    CHIP_ERROR mError         = CHIP_NO_ERROR;
};

} // namespace test
} // namespace chip
