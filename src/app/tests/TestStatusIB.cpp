/*
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

#include <app/AppConfig.h>
#include <app/MessageDef/StatusIB.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

// Macro so failures will blame the right line.
#define VERIFY_ROUNDTRIP(err, status)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        StatusIB newStatus;                                                                                                        \
        newStatus.InitFromChipError(err);                                                                                          \
        NL_TEST_ASSERT(aSuite, newStatus.mStatus == status.mStatus);                                                               \
        NL_TEST_ASSERT(aSuite, newStatus.mClusterStatus == status.mClusterStatus);                                                 \
    } while (0);

void TestStatusIBToFromChipError(nlTestSuite * aSuite, void * aContext)
{
    StatusIB status;

    status.mStatus = Status::Success;
    CHIP_ERROR err = status.ToChipError();
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mStatus = Status::Failure;
    err            = status.ToChipError();
    NL_TEST_ASSERT(aSuite, err != CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mStatus = Status::InvalidAction;
    err            = status.ToChipError();
    NL_TEST_ASSERT(aSuite, err != CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mClusterStatus = MakeOptional(static_cast<ClusterStatus>(5));

    status.mStatus = Status::Success;
    err            = status.ToChipError();
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    status.mStatus = Status::Failure;
    err            = status.ToChipError();
    NL_TEST_ASSERT(aSuite, err != CHIP_NO_ERROR);
    VERIFY_ROUNDTRIP(err, status);

    status.mStatus = Status::InvalidAction;
    err            = status.ToChipError();
    NL_TEST_ASSERT(aSuite, err != CHIP_NO_ERROR);
    {
        StatusIB newStatus;
        newStatus.InitFromChipError(err);
        NL_TEST_ASSERT(aSuite, newStatus.mStatus == Status::Failure);
        NL_TEST_ASSERT(aSuite, newStatus.mClusterStatus == status.mClusterStatus);
    }

    err = CHIP_ERROR_NO_MEMORY;
    {
        StatusIB newStatus;
        newStatus.InitFromChipError(err);
        NL_TEST_ASSERT(aSuite, newStatus.mStatus == Status::Failure);
        NL_TEST_ASSERT(aSuite, !newStatus.mClusterStatus.HasValue());
    }
}

#if !CHIP_CONFIG_SHORT_ERROR_STR
void TestStatusIBErrorToString(nlTestSuite * aSuite, void * aContext)
{
    StatusIB status;
    status.mStatus   = Status::InvalidAction;
    CHIP_ERROR err   = status.ToChipError();
    const char * str = ErrorStr(err);

#if CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    NL_TEST_ASSERT(aSuite, strcmp(str, "IM Error 0x00000580: General error: 0x80 (INVALID_ACTION)") == 0);
#else  // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT
    NL_TEST_ASSERT(aSuite, strcmp(str, "IM Error 0x00000580: General error: 0x80") == 0);
#endif // CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT

    status.mStatus        = Status::Failure;
    status.mClusterStatus = MakeOptional(static_cast<ClusterStatus>(5));
    err                   = status.ToChipError();
    str                   = ErrorStr(err);
    NL_TEST_ASSERT(aSuite, strcmp(str, "IM Error 0x00000605: Cluster-specific error: 0x05") == 0);
}
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("StatusIBToFromChipError", TestStatusIBToFromChipError),
#if !CHIP_CONFIG_SHORT_ERROR_STR
    NL_TEST_DEF("StatusIBErrorToString", TestStatusIBErrorToString),
#endif // !CHIP_CONFIG_SHORT_ERROR_STR
    NL_TEST_SENTINEL()
};
// clang-format on
} // namespace

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    // Hand-register the error formatter.  Normally it's registered by
    // InteractionModelEngine::Init, but we don't want to mess with that here.
    StatusIB::RegisterErrorFormatter();
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestStatusIB()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "StatusIB",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestStatusIB)
