/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/operational-state-server/operational-state-cluster-objects.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::OperationalState;

namespace {

class TestOperationalStateClusterObjects : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalStateConstructorWithOnlyStateID)
{
    using namespace chip::app::Clusters::OperationalState;
    // General state: Stopped
    GenericOperationalState operationalStateStopped(to_underlying(OperationalStateEnum::kStopped));
    EXPECT_EQ(operationalStateStopped.operationalStateID, to_underlying(OperationalStateEnum::kStopped));
    EXPECT_FALSE(operationalStateStopped.operationalStateLabel.HasValue());

    // General state: Running
    GenericOperationalState operationalStateRunning(to_underlying(OperationalStateEnum::kRunning));
    EXPECT_EQ(operationalStateRunning.operationalStateID, to_underlying(OperationalStateEnum::kRunning));
    EXPECT_FALSE(operationalStateRunning.operationalStateLabel.HasValue());

    // General state: Paused
    GenericOperationalState operationalStatePaused(to_underlying(OperationalStateEnum::kPaused));
    EXPECT_EQ(operationalStatePaused.operationalStateID, to_underlying(OperationalStateEnum::kPaused));
    EXPECT_FALSE(operationalStatePaused.operationalStateLabel.HasValue());

    // General state: Error
    GenericOperationalState operationalStateError(to_underlying(OperationalStateEnum::kError));
    EXPECT_EQ(operationalStateError.operationalStateID, to_underlying(OperationalStateEnum::kError));
    EXPECT_FALSE(operationalStateError.operationalStateLabel.HasValue());
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalStateConstructorWithStateIDAndStateLabel)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };

    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    // ManufacturerStates state, label len = 9:
    GenericOperationalState operationalState(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                             Optional<CharSpan>(CharSpan::fromCharString(buffer)));

    EXPECT_EQ(operationalState.operationalStateID, to_underlying(ManufactureOperationalStateEnum::kRebooting));
    EXPECT_TRUE(operationalState.operationalStateLabel.HasValue());
    EXPECT_EQ(operationalState.operationalStateLabel.Value().size(), strlen(buffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer, strlen(buffer)), 0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalStateCopyConstructor)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };

    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    GenericOperationalState srcOperationalState(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                                Optional<CharSpan>(CharSpan::fromCharString(buffer)));

    GenericOperationalState desOperationalState(srcOperationalState);

    EXPECT_EQ(desOperationalState.operationalStateID, srcOperationalState.operationalStateID);
    EXPECT_TRUE(desOperationalState.operationalStateLabel.HasValue());
    EXPECT_EQ(desOperationalState.operationalStateLabel.Value().size(), srcOperationalState.operationalStateLabel.Value().size());
    EXPECT_EQ(memcmp(const_cast<char *>(desOperationalState.operationalStateLabel.Value().data()),
                     const_cast<char *>(srcOperationalState.operationalStateLabel.Value().data()),
                     desOperationalState.operationalStateLabel.Value().size()),
              0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalStateCopyAssignment)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };

    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    GenericOperationalState srcOperationalState(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                                Optional<CharSpan>(CharSpan::fromCharString(buffer)));

    GenericOperationalState desOperationalState = srcOperationalState;

    EXPECT_EQ(desOperationalState.operationalStateID, srcOperationalState.operationalStateID);
    EXPECT_TRUE(desOperationalState.operationalStateLabel.HasValue());
    EXPECT_EQ(desOperationalState.operationalStateLabel.Value().size(), srcOperationalState.operationalStateLabel.Value().size());
    EXPECT_EQ(memcmp(const_cast<char *>(desOperationalState.operationalStateLabel.Value().data()),
                     const_cast<char *>(srcOperationalState.operationalStateLabel.Value().data()),
                     desOperationalState.operationalStateLabel.Value().size()),
              0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalStateFuncSet)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };

    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    // init state
    GenericOperationalState operationalState(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                             Optional<CharSpan>(CharSpan::fromCharString(buffer)));

    // change state without label
    operationalState.Set(to_underlying(OperationalStateEnum::kStopped));
    EXPECT_EQ(operationalState.operationalStateID, to_underlying(OperationalStateEnum::kStopped));
    EXPECT_FALSE(operationalState.operationalStateLabel.HasValue());

    // change state with label
    operationalState.Set(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                         Optional<CharSpan>(CharSpan::fromCharString(buffer)));
    EXPECT_EQ(operationalState.operationalStateID, to_underlying(ManufactureOperationalStateEnum::kRebooting));
    EXPECT_TRUE(operationalState.operationalStateLabel.HasValue());
    EXPECT_EQ(operationalState.operationalStateLabel.Value().size(), strlen(buffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer, strlen(buffer)), 0);

    // change state with label, label len = kOperationalStateLabelMaxSize
    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = 1;
    }
    operationalState.Set(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                         Optional<CharSpan>(CharSpan(buffer, sizeof(buffer))));
    EXPECT_EQ(operationalState.operationalStateID, to_underlying(ManufactureOperationalStateEnum::kRebooting));
    EXPECT_TRUE(operationalState.operationalStateLabel.HasValue());
    EXPECT_EQ(operationalState.operationalStateLabel.Value().size(), sizeof(buffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer, sizeof(buffer)), 0);

    // change state with label, label len larger than kOperationalStateLabelMaxSize
    char buffer2[kOperationalStateLabelMaxSize + 1];

    for (size_t i = 0; i < sizeof(buffer2); i++)
    {
        buffer2[i] = 1;
    }
    operationalState.Set(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                         Optional<CharSpan>(CharSpan(buffer2, sizeof(buffer2))));
    EXPECT_EQ(operationalState.operationalStateID, to_underlying(ManufactureOperationalStateEnum::kRebooting));
    EXPECT_TRUE(operationalState.operationalStateLabel.HasValue());
    EXPECT_EQ(operationalState.operationalStateLabel.Value().size(), kOperationalStateLabelMaxSize);
    EXPECT_EQ(
        memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer2, kOperationalStateLabelMaxSize),
        0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalErrorConstructorWithOnlyStateID)
{
    using namespace chip::app::Clusters::OperationalState;
    // General errors: NoError
    GenericOperationalError operationalErrorNoErr(to_underlying(ErrorStateEnum::kNoError));

    EXPECT_EQ(operationalErrorNoErr.errorStateID, to_underlying(ErrorStateEnum::kNoError));
    EXPECT_FALSE(operationalErrorNoErr.errorStateLabel.HasValue());
    EXPECT_FALSE(operationalErrorNoErr.errorStateDetails.HasValue());

    // General errors: UnableToStartOrResume
    GenericOperationalError operationalErrorUnableToStartOrResume(to_underlying(ErrorStateEnum::kUnableToStartOrResume));

    EXPECT_EQ(operationalErrorUnableToStartOrResume.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_FALSE(operationalErrorUnableToStartOrResume.errorStateLabel.HasValue());
    EXPECT_FALSE(operationalErrorUnableToStartOrResume.errorStateDetails.HasValue());

    // General errors: UnableToCompleteOperation
    GenericOperationalError operationalErrorkUnableToCompleteOperation(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));

    EXPECT_EQ(operationalErrorkUnableToCompleteOperation.errorStateID, to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    EXPECT_FALSE(operationalErrorkUnableToCompleteOperation.errorStateLabel.HasValue());
    EXPECT_FALSE(operationalErrorkUnableToCompleteOperation.errorStateDetails.HasValue());

    // General errors: CommandInvalidInState
    GenericOperationalError operationalErrorCommandInvalidInState(to_underlying(ErrorStateEnum::kCommandInvalidInState));

    EXPECT_EQ(operationalErrorCommandInvalidInState.errorStateID, to_underlying(ErrorStateEnum::kCommandInvalidInState));
    EXPECT_FALSE(operationalErrorCommandInvalidInState.errorStateLabel.HasValue());
    EXPECT_FALSE(operationalErrorCommandInvalidInState.errorStateDetails.HasValue());
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalErrorConstructorWithStateIDAndStateLabel)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalErrorEnum : uint8_t
    {
        kLowBattery = 0x81,
    };

    char labelBuffer[kOperationalErrorLabelMaxSize] = "low battery";

    // ManufacturerStates error with label, label len = 11:
    GenericOperationalError operationalError(to_underlying(ManufactureOperationalErrorEnum::kLowBattery),
                                             Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ManufactureOperationalErrorEnum::kLowBattery));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), strlen(labelBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)), 0);
    EXPECT_FALSE(operationalError.errorStateDetails.HasValue());
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalErrorConstructorWithFullParam)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalErrorEnum : uint8_t
    {
        kLowBattery = 0x81,
    };

    // ManufacturerStates error with label(label len = 11) and detail (len = 25):
    char labelBuffer[kOperationalErrorLabelMaxSize]    = "low battery";
    char detailBuffer[kOperationalErrorDetailsMaxSize] = "Please plug in for charge";

    GenericOperationalError operationalError(to_underlying(ManufactureOperationalErrorEnum::kLowBattery),
                                             Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)),
                                             Optional<CharSpan>(CharSpan::fromCharString(detailBuffer)));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ManufactureOperationalErrorEnum::kLowBattery));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), strlen(labelBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)), 0);

    EXPECT_TRUE(operationalError.errorStateDetails.HasValue());
    EXPECT_EQ(operationalError.errorStateDetails.Value().size(), strlen(detailBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateDetails.Value().data()), detailBuffer, strlen(detailBuffer)), 0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalErrorCopyConstructor)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalErrorEnum : uint8_t
    {
        kLowBattery = 0x81,
    };

    // ManufacturerStates error with label(label len = 11) and detail (len = 25):
    char labelBuffer[kOperationalErrorLabelMaxSize]    = "low battery";
    char detailBuffer[kOperationalErrorDetailsMaxSize] = "Please plug in for charge";

    GenericOperationalError srcOperationalError(to_underlying(ManufactureOperationalErrorEnum::kLowBattery),
                                                Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)),
                                                Optional<CharSpan>(CharSpan::fromCharString(detailBuffer)));

    // call copy constructor
    GenericOperationalError desOperationalError(srcOperationalError);
    EXPECT_EQ(desOperationalError.errorStateID, srcOperationalError.errorStateID);
    EXPECT_TRUE(desOperationalError.errorStateLabel.HasValue());
    EXPECT_EQ(desOperationalError.errorStateLabel.Value().size(), srcOperationalError.errorStateLabel.Value().size());
    EXPECT_EQ(memcmp(const_cast<char *>(desOperationalError.errorStateLabel.Value().data()),
                     const_cast<char *>(srcOperationalError.errorStateLabel.Value().data()),
                     desOperationalError.errorStateLabel.Value().size()),
              0);

    EXPECT_TRUE(desOperationalError.errorStateDetails.HasValue());
    EXPECT_EQ(desOperationalError.errorStateDetails.Value().size(), srcOperationalError.errorStateDetails.Value().size());
    EXPECT_EQ(memcmp(const_cast<char *>(desOperationalError.errorStateDetails.Value().data()),
                     const_cast<char *>(srcOperationalError.errorStateDetails.Value().data()),
                     desOperationalError.errorStateDetails.Value().size()),
              0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalErrorCopyAssignment)
{
    using namespace chip::app::Clusters::OperationalState;

    enum class ManufactureOperationalErrorEnum : uint8_t
    {
        kLowBattery = 0x81,
    };

    // ManufacturerStates error with label(label len = 11) and detail (len = 25):
    char labelBuffer[kOperationalErrorLabelMaxSize]    = "low battery";
    char detailBuffer[kOperationalErrorDetailsMaxSize] = "Please plug in for charge";

    GenericOperationalError srcOperationalError(to_underlying(ManufactureOperationalErrorEnum::kLowBattery),
                                                Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)),
                                                Optional<CharSpan>(CharSpan::fromCharString(detailBuffer)));

    // call copy assignment
    GenericOperationalError desOperationalError = srcOperationalError;
    EXPECT_EQ(desOperationalError.errorStateID, srcOperationalError.errorStateID);
    EXPECT_TRUE(desOperationalError.errorStateLabel.HasValue());
    EXPECT_EQ(desOperationalError.errorStateLabel.Value().size(), srcOperationalError.errorStateLabel.Value().size());
    EXPECT_EQ(memcmp(const_cast<char *>(desOperationalError.errorStateLabel.Value().data()),
                     const_cast<char *>(srcOperationalError.errorStateLabel.Value().data()),
                     desOperationalError.errorStateLabel.Value().size()),
              0);

    EXPECT_TRUE(desOperationalError.errorStateDetails.HasValue());
    EXPECT_EQ(desOperationalError.errorStateDetails.Value().size(), srcOperationalError.errorStateDetails.Value().size());
    EXPECT_EQ(memcmp(const_cast<char *>(desOperationalError.errorStateDetails.Value().data()),
                     const_cast<char *>(srcOperationalError.errorStateDetails.Value().data()),
                     desOperationalError.errorStateDetails.Value().size()),
              0);
}

TEST_F(TestOperationalStateClusterObjects, TestStructGenericOperationalErrorFuncSet)
{
    using namespace chip::app::Clusters::OperationalState;
    enum class ManufactureOperationalErrorEnum : uint8_t
    {
        kLowBattery = 0x81,
    };

    // ManufacturerStates error with label(label len = 11) and detail (len = 25):
    char labelBuffer[kOperationalErrorLabelMaxSize]    = "low battery";
    char detailBuffer[kOperationalErrorDetailsMaxSize] = "Please plug in for charge";

    // General errors: NoError
    GenericOperationalError operationalError(to_underlying(ErrorStateEnum::kNoError));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kNoError));
    EXPECT_FALSE(operationalError.errorStateLabel.HasValue());
    EXPECT_FALSE(operationalError.errorStateDetails.HasValue());

    // call Set with stateId
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_FALSE(operationalError.errorStateLabel.HasValue());
    EXPECT_FALSE(operationalError.errorStateDetails.HasValue());

    // call Set with stateId and StateLabel
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), strlen(labelBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)), 0);
    EXPECT_FALSE(operationalError.errorStateDetails.HasValue());

    // call Set with stateId, StateLabel and StateDetails
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)),
                         Optional<CharSpan>(CharSpan::fromCharString(detailBuffer)));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), strlen(labelBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)), 0);

    EXPECT_TRUE(operationalError.errorStateDetails.HasValue());
    EXPECT_EQ(operationalError.errorStateDetails.Value().size(), strlen(detailBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateDetails.Value().data()), detailBuffer, strlen(detailBuffer)), 0);

    // change state with label, label len = kOperationalStateLabelMaxSize
    for (size_t i = 0; i < sizeof(labelBuffer); i++)
    {
        labelBuffer[i] = 1;
    }
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan(labelBuffer, sizeof(labelBuffer))));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), sizeof(labelBuffer));
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, sizeof(labelBuffer)), 0);
    EXPECT_FALSE(operationalError.errorStateDetails.HasValue());

    // change state with label, label len = kOperationalStateLabelMaxSize + 1
    char labelBuffer2[kOperationalErrorLabelMaxSize + 1];
    for (size_t i = 0; i < sizeof(labelBuffer2); i++)
    {
        labelBuffer2[i] = 2;
    }
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan(labelBuffer2, sizeof(labelBuffer2))));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), kOperationalErrorLabelMaxSize);
    EXPECT_EQ(
        memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer2, kOperationalErrorLabelMaxSize),
        0);
    EXPECT_FALSE(operationalError.errorStateDetails.HasValue());

    // change state with label and details, details len = kOperationalErrorDetailsMaxSize + 1
    char detailBuffer2[kOperationalErrorDetailsMaxSize + 1];
    for (size_t i = 0; i < sizeof(detailBuffer2); i++)
    {
        detailBuffer2[i] = 3;
    }
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan(labelBuffer2, sizeof(labelBuffer2))),
                         Optional<CharSpan>(CharSpan(detailBuffer2, sizeof(detailBuffer2))));

    EXPECT_EQ(operationalError.errorStateID, to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    EXPECT_TRUE(operationalError.errorStateLabel.HasValue());
    EXPECT_EQ(operationalError.errorStateLabel.Value().size(), kOperationalErrorLabelMaxSize);
    EXPECT_EQ(
        memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer2, kOperationalErrorLabelMaxSize),
        0);

    EXPECT_TRUE(operationalError.errorStateDetails.HasValue());

    EXPECT_EQ(operationalError.errorStateDetails.Value().size(), kOperationalErrorDetailsMaxSize);
    EXPECT_EQ(memcmp(const_cast<char *>(operationalError.errorStateDetails.Value().data()), detailBuffer2,
                     kOperationalErrorDetailsMaxSize),
              0);
}

} // namespace
