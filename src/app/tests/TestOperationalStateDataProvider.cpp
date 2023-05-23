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

#include <app/clusters/operational-state-server/OperationalStateDataProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::OperationalState;

using OperationalStateStructType     = app::Clusters::OperationalState::Structs::OperationalStateStruct::Type;
using OperationalStateStructTypeList = app::DataModel::List<OperationalStateStructType>;

using PhaseListType = chip::CharSpan;
using PhaseList     = chip::app::DataModel::List<const chip::CharSpan>;

namespace {

void TestOperationalStateListStoreLoad(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    OperationalStateDataProvider OperationalStateDataProv;
    OperationalStateDataProv.Init(persistentStorage);

    const auto makeOperationalStateStructType = [](OperationalStateEnum stateId, char * stateLabel, size_t len = 0) {
        OperationalStateStructType state;
        state.operationalStateID = stateId;
        if (len)
            state.operationalStateLabel = chip::CharSpan(stateLabel, len);
        return state;
    };
    char opStopped[8]                 = "Stopped";
    char opRunning[32]                = "Running";
    char opPaused[64]                 = "Paused";
    EndpointId endpoint               = 1;
    ClusterId clusterId               = 0x60;
    OperationalStateStructType opS[3] = {
        makeOperationalStateStructType(OperationalStateEnum::kStopped, opStopped, sizeof(opStopped)),
        makeOperationalStateStructType(OperationalStateEnum::kRunning, opRunning, sizeof(opRunning)),
        makeOperationalStateStructType(OperationalStateEnum::kPaused, opPaused, sizeof(opPaused))
    };
    OperationalStateStructTypeList opL(opS);
    NL_TEST_ASSERT(inSuite, opL.size() == 3);
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       OperationalStateDataProv.StoreOperationalStateList<OperationalStateStructType>(endpoint, clusterId, opL));

    OperationalStateStructDynamicList * pOpList = nullptr;
    size_t size;
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR == OperationalStateDataProv.LoadOperationalStateList(endpoint, clusterId, &pOpList, size));
    NL_TEST_ASSERT(inSuite, size == 3);
    NL_TEST_ASSERT(inSuite, pOpList != nullptr);

    int i = 1;
    for (OperationalStateStructDynamicList * op = pOpList; op != nullptr; op = op->Next)
    {
        if (i == 1)
        {
            NL_TEST_ASSERT(inSuite, op->operationalStateID == OperationalStateEnum::kStopped);
            NL_TEST_ASSERT(inSuite, op->operationalStateLabel.size() == strlen(opStopped));
            NL_TEST_ASSERT(inSuite, 0 == memcmp(opStopped, op->operationalStateLabel.data(), strlen(opStopped)));
        }

        if (i == 2)
        {
            NL_TEST_ASSERT(inSuite, op->operationalStateID == OperationalStateEnum::kRunning);
            NL_TEST_ASSERT(inSuite, op->operationalStateLabel.size() == strlen(opRunning));
            NL_TEST_ASSERT(inSuite, 0 == memcmp(opRunning, op->operationalStateLabel.data(), strlen(opRunning)));
        }

        if (i == 3)
        {
            NL_TEST_ASSERT(inSuite, op->operationalStateID == OperationalStateEnum::kPaused);
            NL_TEST_ASSERT(inSuite, op->operationalStateLabel.size() == strlen(opPaused));
            NL_TEST_ASSERT(inSuite, 0 == memcmp(opPaused, op->operationalStateLabel.data(), strlen(opPaused)));
        }

        i++;
    }

    OperationalStateDataProv.ReleaseOperationalStateList(pOpList);
    pOpList = nullptr;
}

void TestOperationalStateListEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    OperationalStateDataProvider OperationalStateDataProv;
    OperationalStateDataProv.Init(persistentStorage);

    OperationalStateStructTypeList opList;
    EndpointId endpoint                         = 1;
    ClusterId clusterId                         = 0x60;
    size_t size                                 = 0;
    OperationalStateStructDynamicList * pOpList = nullptr;

    NL_TEST_ASSERT(inSuite,
                   CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND ==
                       OperationalStateDataProv.LoadOperationalStateList(endpoint, clusterId, &pOpList, size));
    NL_TEST_ASSERT(inSuite, size == 0);
}

void TestUseOpStateIDGetOpStateStruct(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    OperationalStateDataProvider OperationalStateDataProv;
    OperationalStateDataProv.Init(persistentStorage);

    const auto makeOperationalStateStructType = [](OperationalStateEnum stateId, char * stateLabel, size_t len = 0) {
        OperationalStateStructType state;
        state.operationalStateID = stateId;
        if (len)
            state.operationalStateLabel = chip::CharSpan(stateLabel, len);
        return state;
    };
    char opStopped[8]                 = "Stopped";
    char opRunning[32]                = "Running";
    char opPaused[64]                 = "Paused";
    EndpointId endpoint               = 1;
    ClusterId clusterId               = 0x60;
    OperationalStateStructType opS[3] = {
        makeOperationalStateStructType(OperationalStateEnum::kStopped, opStopped, sizeof(opStopped)),
        makeOperationalStateStructType(OperationalStateEnum::kRunning, opRunning, sizeof(opRunning)),
        makeOperationalStateStructType(OperationalStateEnum::kPaused, opPaused, sizeof(opPaused))
    };
    OperationalStateStructTypeList opL(opS);
    NL_TEST_ASSERT(inSuite, opL.size() == 3);
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       OperationalStateDataProv.StoreOperationalStateList<OperationalStateStructType>(endpoint, clusterId, opL));

    OperationalStateStructDynamicList pOp;
    pOp.operationalStateID = OperationalStateEnum::kPaused;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == OperationalStateDataProv.UseOpStateIDGetOpStateStruct(endpoint, clusterId, pOp));

    NL_TEST_ASSERT(inSuite, 0 == memcmp(opPaused, pOp.operationalStateLabel.data(), strlen(opPaused)));
}

void TestPhaseListStoreLoad(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    OperationalStateDataProvider OperationalStateDataProv;
    OperationalStateDataProv.Init(persistentStorage);

    EndpointId endpoint = 1;
    ClusterId clusterId = 0x60;

    char opPauseList[][32] = { "pre-soak", "rinse", "spin" };

    const auto makePhaseType = [](char * str, size_t len = 0) { return CharSpan::fromCharString(str); };

    PhaseListType phase[3] = {
        makePhaseType(opPauseList[0]),
        makePhaseType(opPauseList[1]),
        makePhaseType(opPauseList[2]),
    };

    PhaseList phaseList(phase);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == OperationalStateDataProv.StorePhaseList(endpoint, clusterId, phaseList));

    PhaseListCharSpan * readPhaseList = nullptr;
    size_t size;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == OperationalStateDataProv.LoadPhaseList(endpoint, clusterId, &readPhaseList, size));

    NL_TEST_ASSERT(inSuite, readPhaseList != nullptr);

    int i = 0;
    for (PhaseListCharSpan * op = readPhaseList; op != nullptr; op = op->Next)
    {
        if (i == 0)
        {
            NL_TEST_ASSERT(inSuite, memcmp(op->Phase, opPauseList[0], strlen(opPauseList[0]) + 1) == 0);
        }

        if (i == 1)
        {
            NL_TEST_ASSERT(inSuite, memcmp(op->Phase, opPauseList[1], strlen(opPauseList[0]) + 1) == 0);
        }

        if (i == 2)
        {
            NL_TEST_ASSERT(inSuite, memcmp(op->Phase, opPauseList[2], strlen(opPauseList[0]) + 1) == 0);
        }

        i++;
    }
    OperationalStateDataProv.ReleasePhaseList(readPhaseList);
    readPhaseList = nullptr;
}

void TestPhaseListEmpty(nlTestSuite * inSuite, void * inContext)
{
    TestPersistentStorageDelegate persistentStorage;
    OperationalStateDataProvider OperationalStateDataProv;
    OperationalStateDataProv.Init(persistentStorage);

    OperationalStateStructTypeList opList;
    EndpointId endpoint               = 1;
    ClusterId clusterId               = 0x60;
    size_t size                       = 0;
    PhaseListCharSpan * readPhaseList = nullptr;

    NL_TEST_ASSERT(inSuite,
                   CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND ==
                       OperationalStateDataProv.LoadPhaseList(endpoint, clusterId, &readPhaseList, size));
    NL_TEST_ASSERT(inSuite, size == 0);
}

const nlTest sTests[] = { NL_TEST_DEF("Test operational state list store load", TestOperationalStateListStoreLoad),
                          NL_TEST_DEF("Test operational state list (empty list)", TestOperationalStateListEmpty),
                          NL_TEST_DEF("Test use operational state id to get oprational state struct",
                                      TestUseOpStateIDGetOpStateStruct),
                          NL_TEST_DEF("Test phase list load", TestPhaseListStoreLoad),
                          NL_TEST_DEF("Test phase list (empty list)", TestPhaseListEmpty),
                          NL_TEST_SENTINEL() };

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

int TestOperationalStateDataProvider()
{
    nlTestSuite theSuite = { "Time Operational State data provider tests", &sTests[0], TestSetup, TestTearDown };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestOperationalStateDataProvider)
