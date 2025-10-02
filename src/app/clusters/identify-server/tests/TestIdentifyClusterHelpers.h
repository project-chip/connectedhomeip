/*
 *
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

#pragma once

#include <gtest/gtest.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/clusters/testing/AttributeTesting.h>

// Helper function to write a value to an attribute.
template <typename T>
CHIP_ERROR WriteAttribute(chip::app::Clusters::IdentifyCluster & cluster, const chip::app::ConcreteDataAttributePath & path, const T & value)
{
    chip::app::Testing::WriteOperation writeOperation(path);
    chip::app::AttributeValueDecoder decoder = writeOperation.DecoderFor(value);
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}

// Helper function to invoke a command and return the result.
template <typename T>
std::optional<chip::app::DataModel::ActionReturnStatus> InvokeCommand(chip::app::Clusters::IdentifyCluster & cluster, chip::CommandId commandId, const T & data)
{
    constexpr chip::EndpointId kEndpointId       = 1;
    const chip::app::DataModel::InvokeRequest request = { .path = { kEndpointId, chip::app::Clusters::Identify::Id, commandId } };

    uint8_t buffer[128];
    chip::TLV::TLVWriter tlvWriter;
    tlvWriter.Init(buffer);
    EXPECT_EQ(data.Encode(tlvWriter, chip::TLV::AnonymousTag()), CHIP_NO_ERROR);

    chip::TLV::TLVReader tlvReader;
    tlvReader.Init(buffer, tlvWriter.GetLengthWritten());
    EXPECT_EQ(tlvReader.Next(), CHIP_NO_ERROR);

    return cluster.InvokeCommand(request, tlvReader, nullptr);
}

// Helper function to read an attribute and decode its value.
template <typename T>
CHIP_ERROR ReadAttribute(chip::app::Clusters::IdentifyCluster & cluster, const chip::app::ConcreteDataAttributePath & path, T & value)
{
    chip::app::Testing::ReadOperation readOperation(path);
    std::unique_ptr<chip::app::AttributeValueEncoder> encoder = readOperation.StartEncoding();
    ReturnErrorOnFailure(cluster.ReadAttribute(readOperation.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(readOperation.FinishEncoding());

    std::vector<chip::app::Testing::DecodedAttributeData> attributeData;
    ReturnErrorOnFailure(readOperation.GetEncodedIBs().Decode(attributeData));
    VerifyOrReturnError(attributeData.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    return chip::app::DataModel::Decode(attributeData[0].dataReader, value);
}

class TestTimerDelegate : public chip::app::reporting::ReportScheduler::TimerDelegate
{
public:
    using TimerContext = chip::app::reporting::TimerContext;
    // using namespace chip::app;

    CHIP_ERROR StartTimer(TimerContext * context, chip::System::Clock::Timeout aTimeout) override
    {
        mTimerContext = context;
        mTimerTimeout = mMockSystemTimestamp + aTimeout;
        return CHIP_NO_ERROR;
    }

    void CancelTimer(TimerContext * context) override
    {
        mTimerContext = nullptr;
        mTimerTimeout = chip::System::Clock::Milliseconds64(0x7FFFFFFFFFFFFFFF);
    }

    bool IsTimerActive(TimerContext * context) override { return mTimerContext != nullptr && mTimerTimeout > mMockSystemTimestamp; }

    chip::System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return mMockSystemTimestamp; }

    void AdvanceClock(chip::System::Clock::Timeout aTimeout)
    {
        mMockSystemTimestamp += aTimeout;
        if (mTimerContext && mMockSystemTimestamp >= mTimerTimeout)
        {
            mTimerContext->TimerFired();
        }
    }

private:
    TimerContext * mTimerContext                  = nullptr;
    chip::System::Clock::Timestamp mTimerTimeout        = chip::System::Clock::Milliseconds64(0x7FFFFFFFFFFFFFFF);
    chip::System::Clock::Timestamp mMockSystemTimestamp = chip::System::Clock::Milliseconds64(0);
};