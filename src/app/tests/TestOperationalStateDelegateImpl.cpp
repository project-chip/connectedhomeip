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

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <operational-state-delegate-impl.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

void TestOperationalStateDelegatelGetAndSetOperationalState(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app;
    using namespace chip::app::Clusters::OperationalState;
    /*
     * An example to present device's endpointId
     */
    EndpointId kEndpointId = 1;
    /**
     * Enquriy Table of Operational State List
     * Note: User Define
     */
    const GenericOperationalState opStateList[] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };

    /**
     * Enquriy Table of Operational Phase List
     * Note: User Define
     */
    const GenericOperationalPhase opPhaseList[] = {
        /**
         * Phase List is null
         */
        GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    };

    // create a delegate
    OperationalStateDelegate opStateDelegate(
        kEndpointId, Clusters::OperationalState::Id, GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)), Span<const GenericOperationalState>(opStateList),
        Span<const GenericOperationalPhase>(opPhaseList));

    GenericOperationalState opState(to_underlying(OperationalStateEnum::kRunning));

    // test 1: Get OperationalState
    opStateDelegate.GetOperationalState(opState);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(OperationalStateEnum::kStopped));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == false);

    // test 2: Set OperationalState without label
    opStateDelegate.SetOperationalState(GenericOperationalState(to_underlying(OperationalStateEnum::kError)));

    // Get OperationalState
    opStateDelegate.GetOperationalState(opState);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(OperationalStateEnum::kError));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == false);

    // test 3: Set OperationalState with label
    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };
    char buffer[kOperationalStateLabelMaxSize] = "rebooting";
    opStateDelegate.SetOperationalState(GenericOperationalState(to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                                                Optional<CharSpan>(CharSpan::fromCharString(buffer))));

    // Get OperationalState
    opStateDelegate.GetOperationalState(opState);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kRebooting));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, memcmp(const_cast<char *>(opState.operationalStateLabel.Value().data()), buffer, strlen(buffer)) == 0);
}

void TestOperationalStateDelegatelGetOperationalStateList(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app;
    using namespace chip::app::Clusters::OperationalState;
    /*
     * An example to present device's endpointId
     */
    EndpointId kEndpointId = 1;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };
    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    /**
     * Enquriy Table of Operational State List
     * Note: User Define
     */
    const GenericOperationalState opStateList[] = { GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
                                                    GenericOperationalState(
                                                        to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                                        Optional<CharSpan>(CharSpan::fromCharString(buffer))) };

    const GenericOperationalPhase opPhaseList[] = {
        /**
         * Phase List is null
         */
        GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    };

    // create a delegate
    OperationalStateDelegate opStateDelegate(
        kEndpointId, Clusters::OperationalState::Id, GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)), Span<const GenericOperationalState>(opStateList),
        Span<const GenericOperationalPhase>(opPhaseList));

    GenericOperationalState opState(to_underlying(OperationalStateEnum::kRunning));
    // Get operational state list
    size_t index = 0;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalStateAtIndex(index, opState) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(OperationalStateEnum::kStopped));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == false);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalStateAtIndex(index, opState) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(OperationalStateEnum::kRunning));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == false);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalStateAtIndex(index, opState) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(OperationalStateEnum::kPaused));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == false);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalStateAtIndex(index, opState) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(OperationalStateEnum::kError));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == false);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalStateAtIndex(index, opState) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opState.operationalStateID == to_underlying(ManufactureOperationalStateEnum::kRebooting));
    NL_TEST_ASSERT(inSuite, opState.operationalStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, memcmp(const_cast<char *>(opState.operationalStateLabel.Value().data()), buffer, strlen(buffer)) == 0);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalStateAtIndex(index, opState) == CHIP_ERROR_NOT_FOUND);
}

void TestOperationalStateDelegatelGetAndSetOperationalError(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app;
    using namespace chip::app::Clusters::OperationalState;
    /*
     * An example to present device's endpointId
     */
    EndpointId kEndpointId = 1;
    /**
     * Enquriy Table of Operational State List
     * Note: User Define
     */
    const GenericOperationalState opStateList[] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };

    const GenericOperationalPhase opPhaseList[] = {
        /**
         * Phase List is null
         */
        GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    };

    // create a delegate
    OperationalStateDelegate opStateDelegate(
        kEndpointId, Clusters::OperationalState::Id, GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)), Span<const GenericOperationalState>(opStateList),
        Span<const GenericOperationalPhase>(opPhaseList));

    GenericOperationalError err(to_underlying(ErrorStateEnum::kUnableToStartOrResume));

    // test 1: Get OperationalError
    opStateDelegate.GetOperationalError(err);
    NL_TEST_ASSERT(inSuite, err.errorStateID == to_underlying(ErrorStateEnum::kNoError));
    NL_TEST_ASSERT(inSuite, err.errorStateDetails.HasValue() == false);

    // test 2: Set OperationalError without label
    opStateDelegate.SetOperationalError(GenericOperationalError(to_underlying(ErrorStateEnum::kUnableToStartOrResume)));
    // Get OperationalError
    opStateDelegate.GetOperationalError(err);
    NL_TEST_ASSERT(inSuite, err.errorStateID == to_underlying(ErrorStateEnum::kUnableToStartOrResume));
    NL_TEST_ASSERT(inSuite, err.errorStateDetails.HasValue() == false);

    enum class ManufactureOperationalErrorEnum : uint8_t
    {
        kLowBattery = 0x81,
    };

    // ManufacturerStates error with label(label len = 11) and detail (len = 25):
    char labelBuffer[kOperationalErrorLabelMaxSize]    = "low battery";
    char detailBuffer[kOperationalErrorDetailsMaxSize] = "Please plug in for charge";

    // test 3: Set OperationalError with label and detail
    opStateDelegate.SetOperationalError(GenericOperationalError(to_underlying(ManufactureOperationalErrorEnum::kLowBattery),
                                                                Optional<CharSpan>(CharSpan::fromCharString(labelBuffer)),
                                                                Optional<CharSpan>(CharSpan::fromCharString(detailBuffer))));

    // Get OperationalError
    opStateDelegate.GetOperationalError(err);

    NL_TEST_ASSERT(inSuite, err.errorStateID == to_underlying(ManufactureOperationalErrorEnum::kLowBattery));
    NL_TEST_ASSERT(inSuite, err.errorStateLabel.HasValue() == true);
    NL_TEST_ASSERT(inSuite, err.errorStateLabel.Value().size() == strlen(labelBuffer));
    NL_TEST_ASSERT(
        inSuite,
        memcmp(const_cast<char *>(err.errorStateLabel.Value().data()), labelBuffer, err.errorStateLabel.Value().size()) == 0);

    NL_TEST_ASSERT(inSuite, err.errorStateDetails.HasValue() == true);
    NL_TEST_ASSERT(inSuite, err.errorStateDetails.Value().size() == strlen(detailBuffer));
    NL_TEST_ASSERT(
        inSuite,
        memcmp(const_cast<char *>(err.errorStateDetails.Value().data()), detailBuffer, err.errorStateDetails.Value().size()) == 0);
}

void TestOperationalStateDelegatelGetOperationalPhaseListNull(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app;
    using namespace chip::app::Clusters::OperationalState;
    /*
     * An example to present device's endpointId
     */
    EndpointId kEndpointId = 1;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };
    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    /**
     * Enquriy Table of Operational State List
     * Note: User Define
     */
    const GenericOperationalState opStateList[] = { GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
                                                    GenericOperationalState(
                                                        to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                                        Optional<CharSpan>(CharSpan::fromCharString(buffer))) };

    const GenericOperationalPhase opPhaseList[] = {
        /**
         * Phase List is null
         */
        GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    };

    // create a delegate
    OperationalStateDelegate opStateDelegate(
        kEndpointId, Clusters::OperationalState::Id, GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)), Span<const GenericOperationalState>(opStateList),
        Span<const GenericOperationalPhase>(opPhaseList));

    GenericOperationalPhase opPhase = GenericOperationalPhase(DataModel::Nullable<CharSpan>());

    // Get operational phase list
    size_t index = 0;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalPhaseAtIndex(index, opPhase) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, opPhase.isNullable() == true);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalPhaseAtIndex(index, opPhase) == CHIP_ERROR_NOT_FOUND);
}

void TestOperationalStateDelegatelGetOperationalPhaseList(nlTestSuite * inSuite, void * inContext)
{
    using namespace chip::app;
    using namespace chip::app::Clusters::OperationalState;
    /*
     * An example to present device's endpointId
     */
    EndpointId kEndpointId = 1;

    enum class ManufactureOperationalStateEnum : uint8_t
    {
        kRebooting = 0x81,
    };
    char buffer[kOperationalStateLabelMaxSize] = "rebooting";

    /**
     * Enquriy Table of Operational State List
     * Note: User Define
     */
    const GenericOperationalState opStateList[] = { GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
                                                    GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
                                                    GenericOperationalState(
                                                        to_underlying(ManufactureOperationalStateEnum::kRebooting),
                                                        Optional<CharSpan>(CharSpan::fromCharString(buffer))) };

    /**
     * Enquriy Table of Operational Phase List
     * Note: User Define
     */

    const char * kWasherPreSoak = "pre-soak";
    const char * kWasherRinse   = "rinse";
    const char * kWasherSpin    = "spin";

    const GenericOperationalPhase opPhaseList[] = {
        /**
         * Phase List isn't null
         */
        GenericOperationalPhase(DataModel::Nullable<CharSpan>(CharSpan::fromCharString(kWasherPreSoak))),
        GenericOperationalPhase(DataModel::Nullable<CharSpan>(CharSpan::fromCharString(kWasherRinse))),
        GenericOperationalPhase(DataModel::Nullable<CharSpan>(CharSpan::fromCharString(kWasherSpin))),
    };

    // create a delegate
    OperationalStateDelegate opStateDelegate(
        kEndpointId, Clusters::OperationalState::Id, GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)), Span<const GenericOperationalState>(opStateList),
        Span<const GenericOperationalPhase>(opPhaseList));

    GenericOperationalPhase opPhase = GenericOperationalPhase(DataModel::Nullable<CharSpan>());

    // Get operational phase list
    size_t index = 0;

    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalPhaseAtIndex(index, opPhase) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opPhase.isNullable() == false);
    NL_TEST_ASSERT(inSuite, opPhase.mPhaseName.Value().size() == strlen(kWasherPreSoak));
    NL_TEST_ASSERT(inSuite,
                   memcmp(const_cast<char *>(opPhase.mPhaseName.Value().data()), kWasherPreSoak, strlen(kWasherPreSoak)) == 0);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalPhaseAtIndex(index, opPhase) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opPhase.isNullable() == false);
    NL_TEST_ASSERT(inSuite, opPhase.mPhaseName.Value().size() == strlen(kWasherRinse));
    NL_TEST_ASSERT(inSuite, memcmp(const_cast<char *>(opPhase.mPhaseName.Value().data()), kWasherRinse, strlen(kWasherRinse)) == 0);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalPhaseAtIndex(index, opPhase) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, opPhase.isNullable() == false);
    NL_TEST_ASSERT(inSuite, opPhase.mPhaseName.Value().size() == strlen(kWasherSpin));
    NL_TEST_ASSERT(inSuite, memcmp(const_cast<char *>(opPhase.mPhaseName.Value().data()), kWasherSpin, strlen(kWasherSpin)) == 0);

    index++;
    NL_TEST_ASSERT(inSuite, opStateDelegate.GetOperationalPhaseAtIndex(index, opPhase) == CHIP_ERROR_NOT_FOUND);
}

const nlTest sTests[] = {
    NL_TEST_DEF("Test OperationalStateDelegate: Set and Get Operational State",
                TestOperationalStateDelegatelGetAndSetOperationalState),
    NL_TEST_DEF("Test OperationalStateDelegate: Get Operational State List", TestOperationalStateDelegatelGetOperationalStateList),
    NL_TEST_DEF("Test OperationalStateDelegate: Set and Get Operational Error",
                TestOperationalStateDelegatelGetAndSetOperationalError),
    NL_TEST_DEF("Test OperationalStateDelegate: Get Operational Phase List (null)",
                TestOperationalStateDelegatelGetOperationalPhaseListNull),
    NL_TEST_DEF("Test OperationalStateDelegate: Get Operational Phase List", TestOperationalStateDelegatelGetOperationalPhaseList),
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

int TestOperationalStateDelegateImpl()
{
    nlTestSuite theSuite = { "Test Operational State delegate impl tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestOperationalStateDelegateImpl)
