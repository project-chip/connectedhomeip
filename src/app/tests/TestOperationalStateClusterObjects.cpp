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
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::OperationalState;

namespace {

void TestStructGenericOperationalStateConstructorWithOnlyStateID(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app::Clusters::OperationalState;
    // General state: Stopped
    GenericOperationalState operationalStateStopped(to_underlying(OperationalStateEnum::kStopped));
    NL_TEST_ASSERT(inSuite, operationalStateStopped.operationalStateID == to_underlying(OperationalStateEnum::kStopped));
    NL_TEST_ASSERT(inSuite, operationalStateStopped.operationalStateLabel.HasValue() == false);

    // General state: Running
    GenericOperationalState operationalStateRunning(to_underlying(OperationalStateEnum::kRunning));
    NL_TEST_ASSERT(inSuite, operationalStateRunning.operationalStateID == to_underlying(OperationalStateEnum::kRunning));
    NL_TEST_ASSERT(inSuite, operationalStateRunning.operationalStateLabel.HasValue() == false);

    // General state: Paused
    GenericOperationalState operationalStatePaused(to_underlying(OperationalStateEnum::kPaused));
    NL_TEST_ASSERT(inSuite, operationalStatePaused.operationalStateID == to_underlying(OperationalStateEnum::kPaused));
    NL_TEST_ASSERT(inSuite, operationalStatePaused.operationalStateLabel.HasValue() == false);

    // General state: Error
    GenericOperationalState operationalStateError(to_underlying(OperationalStateEnum::kError));
    NL_TEST_ASSERT(inSuite, operationalStateError.operationalStateID == to_underlying(OperationalStateEnum::kError));
    NL_TEST_ASSERT(inSuite, operationalStateError.operationalStateLabel.HasValue() == false);
}

void TestStructGenericOperationalStateConstructorWithStateIDAndStateLabel(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, operationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kRebooting));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.Value().size() == strlen(buffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer, strlen(buffer)) == 0);
}

void TestStructGenericOperationalStateCopyConstructor(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, desOperationalState.operationalStateID == srcOperationalState.operationalStateID);
    NL_TEST_ASSERT(inSuite, desOperationalState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite,
                   desOperationalState.operationalStateLabel.Value().size() ==
                       srcOperationalState.operationalStateLabel.Value().size());
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(desOperationalState.operationalStateLabel.Value().data()),
                          const_cast<char *>(srcOperationalState.operationalStateLabel.Value().data()),
                          desOperationalState.operationalStateLabel.Value().size()) == 0);
}

void TestStructGenericOperationalStateCopyAssignment(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, desOperationalState.operationalStateID == srcOperationalState.operationalStateID);
    NL_TEST_ASSERT(inSuite, desOperationalState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite,
                   desOperationalState.operationalStateLabel.Value().size() ==
                       srcOperationalState.operationalStateLabel.Value().size());
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(desOperationalState.operationalStateLabel.Value().data()),
                          const_cast<char *>(srcOperationalState.operationalStateLabel.Value().data()),
                          desOperationalState.operationalStateLabel.Value().size()) == 0);
}

void TestStructGenericOperationalStateFuncSet(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateID == to_underlying(OperationalStateEnum::kStopped));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.HasValue() == false);

    // change state with label
    operationalState.Set(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                         Optional<CharSpan>(CharSpan::fromCharString(buffer)));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kRebooting));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.Value().size() == strlen(buffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer, strlen(buffer)) == 0);

    // change state with label, label len = kOperationalStateLabelMaxSize
    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = 1;
    }
    operationalState.Set(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                         Optional<CharSpan>(CharSpan(buffer, sizeof(buffer))));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kRebooting));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.Value().size() == sizeof(buffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer, sizeof(buffer)) == 0);

    // change state with label, label len larger than kOperationalStateLabelMaxSize
    char buffer2[kOperationalStateLabelMaxSize + 1];

    for (size_t i = 0; i < sizeof(buffer2); i++)
    {
        buffer2[i] = 1;
    }
    operationalState.Set(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                         Optional<CharSpan>(CharSpan(buffer2, sizeof(buffer2))));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kRebooting));
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalState.operationalStateLabel.Value().size() == kOperationalStateLabelMaxSize);
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalState.operationalStateLabel.Value().data()), buffer2,
                          kOperationalStateLabelMaxSize) == 0);
}

void TestStructGenericOperationalErrorConstructorWithOnlyStateID(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app::Clusters::OperationalState;
    // General errors: NoError
    GenericOperationalError operationalErrorNoErr(to_underlying(ErrorStateEnum::kNoError));

    NL_TEST_ASSERT(inSuite, operationalErrorNoErr.errorStateID == to_underlying(ErrorStateEnum::kNoError));
    NL_TEST_ASSERT(inSuite, operationalErrorNoErr.errorStateLabel.HasValue() == false);
    NL_TEST_ASSERT(inSuite, operationalErrorNoErr.errorStateDetails.HasValue() == false);

    // General errors: UnableToStartOrResume
    GenericOperationalError operationalErrorUnableToStartOrResume(to_underlying(ErrorStateEnum::kUnableToStartOrResume));

    NL_TEST_ASSERT(inSuite,
                   operationalErrorUnableToStartOrResume.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalErrorUnableToStartOrResume.errorStateLabel.HasValue() == false);
    NL_TEST_ASSERT(inSuite, operationalErrorUnableToStartOrResume.errorStateDetails.HasValue() == false);

    // General errors: UnableToCompleteOperation
    GenericOperationalError operationalErrorkUnableToCompleteOperation(to_underlying(ErrorStateEnum::kUnableToCompleteOperation));

    NL_TEST_ASSERT(inSuite,
                   operationalErrorkUnableToCompleteOperation.errorStateID ==
                       to_underlying(ErrorStateEnum::kUnableToCompleteOperation));
    NL_TEST_ASSERT(inSuite, operationalErrorkUnableToCompleteOperation.errorStateLabel.HasValue() == false);
    NL_TEST_ASSERT(inSuite, operationalErrorkUnableToCompleteOperation.errorStateDetails.HasValue() == false);

    // General errors: CommandInvalidInState
    GenericOperationalError operationalErrorCommandInvalidInState(to_underlying(ErrorStateEnum::kCommandInvalidInState));

    NL_TEST_ASSERT(inSuite,
                   operationalErrorCommandInvalidInState.errorStateID == to_underlying(ErrorStateEnum::kCommandInvalidInState));
    NL_TEST_ASSERT(inSuite, operationalErrorCommandInvalidInState.errorStateLabel.HasValue() == false);
    NL_TEST_ASSERT(inSuite, operationalErrorCommandInvalidInState.errorStateDetails.HasValue() == false);
}

void TestStructGenericOperationalErrorConstructorWithStateIDAndStateLabel(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ManufactureOperationalErrorEnum::kLowBattery));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == strlen(labelBuffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)) ==
                       0);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == false);
}

void TestStructGenericOperationalErrorConstructorWithFullParam(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ManufactureOperationalErrorEnum::kLowBattery));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == strlen(labelBuffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)) ==
                       0);

    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.Value().size() == strlen(detailBuffer));
    NL_TEST_ASSERT(
        inSuite,
        memcmp(const_cast<char *>(operationalError.errorStateDetails.Value().data()), detailBuffer, strlen(detailBuffer)) == 0);
}

void TestStructGenericOperationalErrorCopyConstructor(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, desOperationalError.errorStateID == srcOperationalError.errorStateID);
    NL_TEST_ASSERT(inSuite, desOperationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite,
                   desOperationalError.errorStateLabel.Value().size() == srcOperationalError.errorStateLabel.Value().size());
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(desOperationalError.errorStateLabel.Value().data()),
                          const_cast<char *>(srcOperationalError.errorStateLabel.Value().data()),
                          desOperationalError.errorStateLabel.Value().size()) == 0);

    NL_TEST_ASSERT(inSuite, desOperationalError.errorStateDetails.HasValue() == true);
    NL_TEST_ASSERT(inSuite,
                   desOperationalError.errorStateDetails.Value().size() == srcOperationalError.errorStateDetails.Value().size());
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(desOperationalError.errorStateDetails.Value().data()),
                          const_cast<char *>(srcOperationalError.errorStateDetails.Value().data()),
                          desOperationalError.errorStateDetails.Value().size()) == 0);
}

void TestStructGenericOperationalErrorCopyAssignment(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, desOperationalError.errorStateID == srcOperationalError.errorStateID);
    NL_TEST_ASSERT(inSuite, desOperationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite,
                   desOperationalError.errorStateLabel.Value().size() == srcOperationalError.errorStateLabel.Value().size());
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(desOperationalError.errorStateLabel.Value().data()),
                          const_cast<char *>(srcOperationalError.errorStateLabel.Value().data()),
                          desOperationalError.errorStateLabel.Value().size()) == 0);

    NL_TEST_ASSERT(inSuite, desOperationalError.errorStateDetails.HasValue() == true);
    NL_TEST_ASSERT(inSuite,
                   desOperationalError.errorStateDetails.Value().size() == srcOperationalError.errorStateDetails.Value().size());
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(desOperationalError.errorStateDetails.Value().data()),
                          const_cast<char *>(srcOperationalError.errorStateDetails.Value().data()),
                          desOperationalError.errorStateDetails.Value().size()) == 0);
}

void TestStructGenericOperationalErrorFuncSet(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kNoError));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == false);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == false);

    // call Set with stateId
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume));

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == false);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == false);

    // call Set with stateId and StateLabel
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)));

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == strlen(labelBuffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)) ==
                       0);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == false);

    // call Set with stateId, StateLabel and StateDetails
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)),
                         Optional<CharSpan>(CharSpan::fromCharString(detailBuffer)));

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == strlen(labelBuffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, strlen(labelBuffer)) ==
                       0);

    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.Value().size() == strlen(detailBuffer));
    NL_TEST_ASSERT(
        inSuite,
        memcmp(const_cast<char *>(operationalError.errorStateDetails.Value().data()), detailBuffer, strlen(detailBuffer)) == 0);

    // change state with label, label len = kOperationalStateLabelMaxSize
    for (size_t i = 0; i < sizeof(labelBuffer); i++)
    {
        labelBuffer[i] = 1;
    }
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan(labelBuffer, sizeof(labelBuffer))));

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == sizeof(labelBuffer));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer, sizeof(labelBuffer)) ==
                       0);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == false);

    // change state with label, label len = kOperationalStateLabelMaxSize + 1
    char labelBuffer2[kOperationalErrorLabelMaxSize + 1];
    for (size_t i = 0; i < sizeof(labelBuffer2); i++)
    {
        labelBuffer2[i] = 2;
    }
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan(labelBuffer2, sizeof(labelBuffer2))));

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == kOperationalErrorLabelMaxSize);
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer2,
                          kOperationalErrorLabelMaxSize) == 0);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == false);

    // change state with label and details, details len = kOperationalErrorDetailsMaxSize + 1
    char detailBuffer2[kOperationalErrorDetailsMaxSize + 1];
    for (size_t i = 0; i < sizeof(detailBuffer2); i++)
    {
        detailBuffer2[i] = 3;
    }
    operationalError.Set(to_underlying(ErrorStateEnum::kUnableToStartOrResume),
                         Optional<CharSpan>(CharSpan(labelBuffer2, sizeof(labelBuffer2))),
                         Optional<CharSpan>(CharSpan(detailBuffer2, sizeof(detailBuffer2))));

    NL_TEST_ASSERT(inSuite, operationalError.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, operationalError.errorStateLabel.Value().size() == kOperationalErrorLabelMaxSize);
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateLabel.Value().data()), labelBuffer2,
                          kOperationalErrorLabelMaxSize) == 0);

    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.HasValue() == true);

    NL_TEST_ASSERT(inSuite, operationalError.errorStateDetails.Value().size() == kOperationalErrorDetailsMaxSize);
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(operationalError.errorStateDetails.Value().data()), detailBuffer2,
                          kOperationalErrorDetailsMaxSize) == 0);
}

const nlTest sTests[] = {
    NL_TEST_DEF("Test struct GenericOperationalState: constructor with only StateID",
                TestStructGenericOperationalStateConstructorWithOnlyStateID),
    NL_TEST_DEF("Test struct GenericOperationalState: constructor with StateID and StateLabel",
                TestStructGenericOperationalStateConstructorWithStateIDAndStateLabel),
    NL_TEST_DEF("Test struct GenericOperationalState: copy constructor", TestStructGenericOperationalStateCopyConstructor),
    NL_TEST_DEF("Test struct GenericOperationalState: copy assignment", TestStructGenericOperationalStateCopyAssignment),
    NL_TEST_DEF("Test struct GenericOperationalState: member function 'Set'", TestStructGenericOperationalStateFuncSet),
    NL_TEST_DEF("Test struct GenericOperationalError: constructor with only StateID",
                TestStructGenericOperationalErrorConstructorWithOnlyStateID),
    NL_TEST_DEF("Test struct GenericOperationalError: constructor with StateID and StateLabel",
                TestStructGenericOperationalErrorConstructorWithStateIDAndStateLabel),
    NL_TEST_DEF("Test struct GenericOperationalError: constructor with StateID, StateLabel and StateDetail",
                TestStructGenericOperationalErrorConstructorWithFullParam),
    NL_TEST_DEF("Test struct GenericOperationalError: copy constructor", TestStructGenericOperationalErrorCopyConstructor),
    NL_TEST_DEF("Test struct GenericOperationalError: copy assignment", TestStructGenericOperationalErrorCopyAssignment),
    NL_TEST_DEF("Test struct GenericOperationalError: member function 'Set'", TestStructGenericOperationalErrorFuncSet),
    NL_TEST_SENTINEL()
};

int TestSetup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);
    return SUCCESS;
}

int TestTearDown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

int TestOperationalStateClusterObjects()
{
    nlTestSuite theSuite = { "Test Operational State Cluster Objects tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suite against one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestOperationalStateClusterObjects)
