/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "app-common/zap-generated/ids/Attributes.h"
#include "lib/core/TLVTags.h"
#include "protocols/interaction_model/Constants.h"
#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/BufferedReadCallback.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <app/tests/AppTestContext.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <vector>

using TestContext = chip::Test::AppContext;
using namespace chip::app;
using namespace chip;

namespace {

nlTestSuite * gSuite = nullptr;

struct ValidationInstruction
{
    enum ProcessingType
    {
        kValidChunk,
        kDiscardedChunk
    };

    enum ValidationType
    {
        kSimpleAttributeA,
        kSimpleAttributeB,
        kListAttributeC_Empty,
        kListAttributeC_NotEmpty,
        kListAttributeC_NotEmpty_Chunked,
        kListAttributeC_Error,
        kListAttributeD_Empty,
        kListAttributeD_NotEmpty,
        kListAttributeD_NotEmpty_Chunked,
        kListAttributeD_Error
    };

    ValidationType mValidationType;
    ProcessingType mProcessingType = kValidChunk;
};

using InstructionListType = std::vector<ValidationInstruction>;

class DataSeriesValidator : public BufferedReadCallback::Callback
{
public:
    DataSeriesValidator(std::vector<ValidationInstruction> validationInstructionList)
    {
        mInstructionList = validationInstructionList;
    }

    //
    // BufferedReadCallback::Callback
    //

    void OnReportBegin() override;
    void OnReportEnd() override;
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnDone(ReadClient *) override {}

    std::vector<ValidationInstruction> mInstructionList;
    uint32_t mCurrentInstruction = 0;
};

void DataSeriesValidator::OnReportBegin()
{
    mCurrentInstruction = 0;
}

void DataSeriesValidator::OnReportEnd() {}

void DataSeriesValidator::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                          const StatusIB & aStatus)
{
    uint32_t expectedListLength;

    while ((mCurrentInstruction < mInstructionList.size()) &&
           (mInstructionList[mCurrentInstruction].mProcessingType == ValidationInstruction::kDiscardedChunk))
    {
        mCurrentInstruction++;
    }

    if (mCurrentInstruction >= mInstructionList.size())
    {
        return;
    }

    switch (mInstructionList[mCurrentInstruction].mValidationType)
    {
    case ValidationInstruction::kSimpleAttributeA: {
        ChipLogProgress(DataManagement, "\t\t -- Validating A");

        Clusters::UnitTesting::Attributes::Int8u::TypeInfo::Type value;
        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::Int8u::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::NotList);
        NL_TEST_ASSERT(gSuite, DataModel::Decode(*apData, value) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, value == mCurrentInstruction);
        break;
    }

    case ValidationInstruction::kSimpleAttributeB: {
        ChipLogProgress(DataManagement, "\t\t -- Validating B");

        Clusters::UnitTesting::Attributes::Int32u::TypeInfo::Type value;
        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::Int32u::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::NotList);
        NL_TEST_ASSERT(gSuite, DataModel::Decode(*apData, value) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, value == mCurrentInstruction);
        break;
    }

    case ValidationInstruction::kListAttributeC_Empty: {
        ChipLogProgress(DataManagement, "\t\t -- Validating C[]");

        Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::DecodableType value;
        size_t len;

        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::ListStructOctetString::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll);
        NL_TEST_ASSERT(gSuite, DataModel::Decode(*apData, value) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, value.ComputeSize(&len) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, len == 0);

        break;
    }

    case ValidationInstruction::kListAttributeC_NotEmpty_Chunked:
    case ValidationInstruction::kListAttributeC_NotEmpty: {
        if (mInstructionList[mCurrentInstruction].mValidationType == ValidationInstruction::kListAttributeC_NotEmpty_Chunked)
        {
            expectedListLength = 512;
        }
        else
        {
            expectedListLength = 2;
        }

        ChipLogProgress(DataManagement, "\t\t -- Validating C[%" PRIu32 "]", expectedListLength);

        Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::DecodableType value;
        size_t len;

        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::ListStructOctetString::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll);
        NL_TEST_ASSERT(gSuite, DataModel::Decode(*apData, value) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, value.ComputeSize(&len) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, len == expectedListLength);

        auto iter = value.begin();

        uint32_t index = 0;
        while (iter.Next() && index < expectedListLength)
        {
            auto & iterValue = iter.GetValue();
            NL_TEST_ASSERT(gSuite, iterValue.member1 == (index));
            index++;
        }

        NL_TEST_ASSERT(gSuite, iter.GetStatus() == CHIP_NO_ERROR);
        break;
    }

    case ValidationInstruction::kListAttributeD_Empty: {
        ChipLogProgress(DataManagement, "\t\t -- Validating D[]");

        Clusters::UnitTesting::Attributes::ListInt8u::TypeInfo::DecodableType value;
        size_t len;

        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::ListInt8u::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll);
        NL_TEST_ASSERT(gSuite, DataModel::Decode(*apData, value) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, value.ComputeSize(&len) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, len == 0);

        break;
    }

    case ValidationInstruction::kListAttributeD_NotEmpty:
    case ValidationInstruction::kListAttributeD_NotEmpty_Chunked: {
        if (mInstructionList[mCurrentInstruction].mValidationType == ValidationInstruction::kListAttributeD_NotEmpty_Chunked)
        {
            expectedListLength = 512;
        }
        else
        {
            expectedListLength = 2;
        }

        ChipLogProgress(DataManagement, "\t\t -- Validating D[%" PRIu32 "]", expectedListLength);

        Clusters::UnitTesting::Attributes::ListInt8u::TypeInfo::DecodableType value;
        size_t len;

        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::ListInt8u::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll);
        NL_TEST_ASSERT(gSuite, DataModel::Decode(*apData, value) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, value.ComputeSize(&len) == CHIP_NO_ERROR);
        NL_TEST_ASSERT(gSuite, len == expectedListLength);

        auto iter = value.begin();

        uint8_t index = 0;
        while (iter.Next() && index < expectedListLength)
        {
            auto & iterValue = iter.GetValue();
            NL_TEST_ASSERT(gSuite, iterValue == (index));
            index++;
        }

        NL_TEST_ASSERT(gSuite, iter.GetStatus() == CHIP_NO_ERROR);
        break;
    }

    case ValidationInstruction::kListAttributeC_Error: {
        ChipLogProgress(DataManagement, "\t\t -- Validating C|e");

        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::ListStructOctetString::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll);
        NL_TEST_ASSERT(gSuite, aStatus.mStatus == Protocols::InteractionModel::Status::Failure);
        break;
    }

    case ValidationInstruction::kListAttributeD_Error: {
        ChipLogProgress(DataManagement, "\t\t -- Validating D|e");

        NL_TEST_ASSERT(gSuite,
                       aPath.mEndpointId == 0 && aPath.mClusterId == Clusters::UnitTesting::Id &&
                           aPath.mAttributeId == Clusters::UnitTesting::Attributes::ListInt8u::Id &&
                           aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll);
        NL_TEST_ASSERT(gSuite, aStatus.mStatus == Protocols::InteractionModel::Status::Failure);

        break;
    }

    default:
        break;
    }

    mCurrentInstruction++;
}

class DataSeriesGenerator
{
public:
    DataSeriesGenerator(BufferedReadCallback & readCallback, std::vector<ValidationInstruction> instructionList) :
        mReadCallback(readCallback), mInstructionList(instructionList)
    {}

    void Generate();

private:
    BufferedReadCallback & mReadCallback;
    std::vector<ValidationInstruction> mInstructionList;
};

void DataSeriesGenerator::Generate()
{
    System::PacketBufferHandle handle;
    System::PacketBufferTLVWriter writer;
    ConcreteDataAttributePath path(0, Clusters::UnitTesting::Id, 0);
    System::PacketBufferTLVReader reader;
    ReadClient::Callback * callback = &mReadCallback;
    StatusIB status;
    bool hasData;

    callback->OnReportBegin();

    uint8_t index = 0;
    for (auto & instruction : mInstructionList)
    {
        handle = System::PacketBufferHandle::New(1000);
        writer.Init(std::move(handle), true);
        status  = StatusIB();
        hasData = true;

        switch (instruction.mValidationType)
        {
        case ValidationInstruction::kSimpleAttributeA: {
            ChipLogProgress(DataManagement, "\t -- Generating A");

            Clusters::UnitTesting::Attributes::Int8u::TypeInfo::Type value = index;
            path.mAttributeId                                              = Clusters::UnitTesting::Attributes::Int8u::Id;
            path.mListOp                                                   = ConcreteDataAttributePath::ListOperation::NotList;
            NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);
            break;
        }

        case ValidationInstruction::kSimpleAttributeB: {
            ChipLogProgress(DataManagement, "\t -- Generating B");

            Clusters::UnitTesting::Attributes::Int32u::TypeInfo::Type value = index;
            path.mAttributeId                                               = Clusters::UnitTesting::Attributes::Int32u::Id;
            path.mListOp                                                    = ConcreteDataAttributePath::ListOperation::NotList;
            NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);
            break;
        }

        case ValidationInstruction::kListAttributeC_Empty: {
            ChipLogProgress(DataManagement, "\t -- Generating C[]");

            Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;
            path.mAttributeId = Clusters::UnitTesting::Attributes::ListStructOctetString::Id;
            path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);
            break;
        }

        case ValidationInstruction::kListAttributeC_NotEmpty: {
            ChipLogProgress(DataManagement, "\t -- Generating C[2]");

            Clusters::UnitTesting::Structs::TestListStructOctet::Type listData[2];
            Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value(listData);

            uint8_t index2 = 0;
            for (auto & item : listData)
            {
                item.member1 = index2;
                index2++;
            }

            path.mAttributeId = Clusters::UnitTesting::Attributes::ListStructOctetString::Id;
            path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;

            NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);
            break;
        }

        case ValidationInstruction::kListAttributeD_Empty: {
            ChipLogProgress(DataManagement, "\t -- Generating D[]");

            Clusters::UnitTesting::Attributes::ListInt8u::TypeInfo::Type value;
            path.mAttributeId = Clusters::UnitTesting::Attributes::ListInt8u::Id;
            path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);
            break;
        }

        case ValidationInstruction::kListAttributeD_NotEmpty: {
            ChipLogProgress(DataManagement, "\t -- Generating D[2]");

            uint8_t listData[2];
            Clusters::UnitTesting::Attributes::ListInt8u::TypeInfo::Type value(listData);

            uint8_t index2 = 0;
            for (auto & item : listData)
            {
                item = index2;
                index2++;
            }

            path.mAttributeId = Clusters::UnitTesting::Attributes::ListInt8u::Id;
            path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;

            NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);
            break;
        }

        case ValidationInstruction::kListAttributeC_Error: {
            ChipLogProgress(DataManagement, "\t -- Generating C|e");

            path.mAttributeId = Clusters::UnitTesting::Attributes::ListStructOctetString::Id;
            path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            status.mStatus    = Protocols::InteractionModel::Status::Failure;
            hasData           = false;
            callback->OnAttributeData(path, &reader, status);
            break;
        }

        case ValidationInstruction::kListAttributeD_Error: {
            ChipLogProgress(DataManagement, "\t -- Generating D|e");

            path.mAttributeId = Clusters::UnitTesting::Attributes::ListInt8u::Id;
            path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            status.mStatus    = Protocols::InteractionModel::Status::Failure;
            hasData           = false;
            callback->OnAttributeData(path, &reader, status);
            break;
        }

        case ValidationInstruction::kListAttributeC_NotEmpty_Chunked: {
            hasData = false;
            Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;

            {
                ChipLogProgress(DataManagement, "\t -- Generating C[]");

                path.mAttributeId = Clusters::UnitTesting::Attributes::ListStructOctetString::Id;
                path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;
                NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);

                writer.Finalize(&handle);
                reader.Init(std::move(handle));
                NL_TEST_ASSERT(gSuite, reader.Next() == CHIP_NO_ERROR);
                callback->OnAttributeData(path, &reader, status);
            }

            ChipLogProgress(DataManagement, "\t -- Generating C0..C512");

            for (int i = 0; i < 512; i++)
            {
                Clusters::UnitTesting::Structs::TestListStructOctet::Type listItem;

                handle = System::PacketBufferHandle::New(1000);
                writer.Init(std::move(handle), true);
                status = StatusIB();

                path.mAttributeId = Clusters::UnitTesting::Attributes::ListStructOctetString::Id;
                path.mListOp      = ConcreteDataAttributePath::ListOperation::AppendItem;

                listItem.member1 = (uint64_t) i;

                NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), listItem) == CHIP_NO_ERROR);

                writer.Finalize(&handle);
                reader.Init(std::move(handle));
                NL_TEST_ASSERT(gSuite, reader.Next() == CHIP_NO_ERROR);
                callback->OnAttributeData(path, &reader, status);
            }

            break;
        }

        case ValidationInstruction::kListAttributeD_NotEmpty_Chunked: {
            hasData = false;
            Clusters::UnitTesting::Attributes::ListInt8u::TypeInfo::Type value;

            {
                ChipLogProgress(DataManagement, "\t -- Generating D[]");

                path.mAttributeId = Clusters::UnitTesting::Attributes::ListInt8u::Id;
                path.mListOp      = ConcreteDataAttributePath::ListOperation::ReplaceAll;
                NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), value) == CHIP_NO_ERROR);

                writer.Finalize(&handle);
                reader.Init(std::move(handle));
                NL_TEST_ASSERT(gSuite, reader.Next() == CHIP_NO_ERROR);
                callback->OnAttributeData(path, &reader, status);
            }

            ChipLogProgress(DataManagement, "\t -- Generating D0..D512");

            for (int i = 0; i < 512; i++)
            {
                handle = System::PacketBufferHandle::New(1000);
                writer.Init(std::move(handle), true);
                status = StatusIB();

                path.mAttributeId = Clusters::UnitTesting::Attributes::ListInt8u::Id;
                path.mListOp      = ConcreteDataAttributePath::ListOperation::AppendItem;

                NL_TEST_ASSERT(gSuite, DataModel::Encode(writer, TLV::AnonymousTag(), (uint8_t) (i)) == CHIP_NO_ERROR);

                writer.Finalize(&handle);
                reader.Init(std::move(handle));
                NL_TEST_ASSERT(gSuite, reader.Next() == CHIP_NO_ERROR);
                callback->OnAttributeData(path, &reader, status);
            }

            break;
        }

        default:
            break;
        }

        if (hasData)
        {
            writer.Finalize(&handle);
            reader.Init(std::move(handle));
            NL_TEST_ASSERT(gSuite, reader.Next() == CHIP_NO_ERROR);
            callback->OnAttributeData(path, &reader, status);
        }

        index++;
    }

    callback->OnReportEnd();
}

void RunAndValidateSequence(std::vector<ValidationInstruction> instructionList)
{
    DataSeriesValidator validator(instructionList);
    BufferedReadCallback bufferedCallback(validator);
    DataSeriesGenerator generator(bufferedCallback, instructionList);
    generator.Generate();

    NL_TEST_ASSERT(gSuite, validator.mCurrentInstruction == instructionList.size());
}

void TestBufferedSequences(nlTestSuite * apSuite, void * apContext)
{
    ChipLogProgress(DataManagement, "Validating various sequences of attribute data IBs...");

    ChipLogProgress(DataManagement, "A --> A");
    RunAndValidateSequence({ { ValidationInstruction::kSimpleAttributeA } });

    ChipLogProgress(DataManagement, "A A --> A A");
    RunAndValidateSequence({ { ValidationInstruction::kSimpleAttributeA }, { ValidationInstruction::kSimpleAttributeA } });

    ChipLogProgress(DataManagement, "A B --> A B");
    RunAndValidateSequence({ { ValidationInstruction::kSimpleAttributeA }, { ValidationInstruction::kSimpleAttributeB } });

    ChipLogProgress(DataManagement, "A C[] --> A C[]");
    RunAndValidateSequence({ { ValidationInstruction::kSimpleAttributeA }, { ValidationInstruction::kListAttributeC_Empty } });

    ChipLogProgress(DataManagement, "C[] C[] --> C[]");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_Empty, ValidationInstruction::kDiscardedChunk },
                             { ValidationInstruction::kListAttributeC_Empty } });

    ChipLogProgress(DataManagement, "C[2] C[] --> C[]");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_NotEmpty, ValidationInstruction::kDiscardedChunk },
                             { ValidationInstruction::kListAttributeC_Empty } });

    ChipLogProgress(DataManagement, "C[] C[2] --> C[2]");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_Empty, ValidationInstruction::kDiscardedChunk },
                             { ValidationInstruction::kListAttributeC_NotEmpty } });

    ChipLogProgress(DataManagement, "C[] A C[2] --> C[] A C[2]");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_Empty },
                             { ValidationInstruction::kSimpleAttributeA },
                             { ValidationInstruction::kListAttributeC_NotEmpty } });

    ChipLogProgress(DataManagement, "C[] C[2] A --> C[] C[2] A");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_Empty },
                             { ValidationInstruction::kSimpleAttributeA },
                             { ValidationInstruction::kListAttributeC_NotEmpty } });

    ChipLogProgress(DataManagement, "C[] D[] --> C[] D[]");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_Empty }, { ValidationInstruction::kListAttributeD_Empty } });

    ChipLogProgress(DataManagement, "C[2] D[] --> C[2] D[]");
    RunAndValidateSequence(
        { { ValidationInstruction::kListAttributeC_NotEmpty }, { ValidationInstruction::kListAttributeD_Empty } });

    ChipLogProgress(DataManagement, "C[2] C|e --> C|e");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_NotEmpty, ValidationInstruction::kDiscardedChunk },
                             { ValidationInstruction::kListAttributeC_Error } });

    ChipLogProgress(DataManagement, "A C|e --> A C|e");
    RunAndValidateSequence({ { ValidationInstruction::kSimpleAttributeA }, { ValidationInstruction::kListAttributeC_Error } });

    ChipLogProgress(DataManagement, "C|e C[2] --> C|e C[2]");
    RunAndValidateSequence(
        { { ValidationInstruction::kListAttributeC_Error }, { ValidationInstruction::kListAttributeC_NotEmpty } });

    ChipLogProgress(DataManagement, "C[] C0 C1 --> C[2]");
    RunAndValidateSequence({ { ValidationInstruction::kListAttributeC_NotEmpty_Chunked } });

    ChipLogProgress(DataManagement, "C[] C0 C1 C[] --> C[]");
    RunAndValidateSequence({
        { ValidationInstruction::kListAttributeC_NotEmpty_Chunked, ValidationInstruction::kDiscardedChunk },
        { ValidationInstruction::kListAttributeC_Empty },
    });

    ChipLogProgress(DataManagement, "C[] C0 C1 D[] D0 D1 --> C[2] D[2]");
    RunAndValidateSequence({
        { ValidationInstruction::kListAttributeC_NotEmpty_Chunked },
        { ValidationInstruction::kListAttributeD_NotEmpty_Chunked },
    });
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestBufferedSequences", TestBufferedSequences),
    NL_TEST_SENTINEL()
};

nlTestSuite theSuite =
{
    "TestBufferedReadCallback",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};

}
// clang-format on

int TestBufferedReadCallback()
{
    gSuite = &theSuite;
    return chip::ExecuteTestsWithContext<TestContext>(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBufferedReadCallback)
