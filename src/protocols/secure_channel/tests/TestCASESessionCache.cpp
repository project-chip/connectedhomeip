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

/**
 *    @file
 *      This file implements unit tests for the CASESession implementation.
 */

#include <errno.h>
#include <nlunit-test.h>

#include <lib/core/CHIPConfig.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/CASESessionCache.h>
#include <stdarg.h>

using namespace chip;

namespace {
NodeId sTest_PeerId = 0xEDEDEDED00010001;

uint8_t sTest_SharedSecret[] = {
    0x7d, 0x73, 0x5b, 0xef, 0xe9, 0x16, 0xa1, 0xc0, 0xca, 0x02, 0xf8, 0xca, 0x98, 0x81, 0xe4, 0x26,
    0x63, 0xaa, 0xaf, 0x9a, 0xb9, 0xc4, 0x33, 0xb2, 0x89, 0xbe, 0x26, 0x70, 0x10, 0x75, 0x74, 0x10,
};

uint8_t sTest_ResumptionId[kCASEResumptionIDSize] = { 0 };

} // namespace

class CASESessionTest : public CASESession
{
public:
    void createCASESessionTestCachable(uint8_t i)
    {
        uint16_t sharedSecretLen                      = sizeof(sTest_SharedSecret);
        sTest_SharedSecret[sharedSecretLen - 1]       = static_cast<uint8_t>(sTest_SharedSecret[sharedSecretLen - 1] + i);
        uint64_t timestamp                            = static_cast<uint64_t>(4000 + i * 1000);
        sTest_ResumptionId[kCASEResumptionIDSize - 1] = static_cast<uint8_t>(sTest_ResumptionId[kCASEResumptionIDSize - 1] + i);

        mCASESessionCachableArray[i].mSharedSecretLen = sharedSecretLen;
        memcpy(mCASESessionCachableArray[i].mSharedSecret, sTest_SharedSecret, sharedSecretLen);
        mCASESessionCachableArray[i].mPeerNodeId         = static_cast<uint16_t>(sTest_PeerId + i);
        mCASESessionCachableArray[i].mPeerCATs.values[0] = (uint32_t) i;
        memcpy(mCASESessionCachableArray[i].mResumptionId, sTest_ResumptionId, kCASEResumptionIDSize);
        mCASESessionCachableArray[i].mLocalFabricIndex      = 0;
        mCASESessionCachableArray[i].mSessionSetupTimeStamp = timestamp;
    }

    bool isEqual(int index, CASESessionCachable cachableSession)
    {
        return (cachableSession.mSharedSecretLen == mCASESessionCachableArray[index].mSharedSecretLen) &&
            ((ByteSpan(cachableSession.mSharedSecret)).data_equal(ByteSpan(mCASESessionCachableArray[index].mSharedSecret))) &&
            (cachableSession.mPeerNodeId == mCASESessionCachableArray[index].mPeerNodeId) &&
            cachableSession.mPeerCATs.values[0] == mCASESessionCachableArray[index].mPeerCATs.values[0] &&
            ((ResumptionID(cachableSession.mResumptionId))
                 .data_equal(ResumptionID(mCASESessionCachableArray[index].mResumptionId))) &&
            (cachableSession.mLocalFabricIndex == mCASESessionCachableArray[index].mLocalFabricIndex) &&
            (cachableSession.mSessionSetupTimeStamp == mCASESessionCachableArray[index].mSessionSetupTimeStamp);
    }

    void InitializeCASESessionCachableArray()
    {
        for (size_t j = 0; j < kCASEResumptionIDSize; j++)
        {
            sTest_ResumptionId[j] = 0x01;
        }
        for (uint8_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; i++)
        {
            createCASESessionTestCachable(i);
        }
    }

    CASESessionCachable mCASESessionCachableArray[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE + 1] = { { 0 } };
    CASESessionCache mCASESessionCache;
};

CASESessionTest mCASESessionTest;

static void CASESessionCache_Create_Test(nlTestSuite * inSuite, void * inContext)
{
    mCASESessionTest.InitializeCASESessionCachableArray();
}

static void CASESessionCache_Add_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint8_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; i++)
    {
        CASESession session;
        err = mCASESessionTest.mCASESessionCache.Add(mCASESessionTest.mCASESessionCachableArray[i]);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }
}

static void CASESessionCache_Get_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint8_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; i++)
    {
        CASESessionCachable outCachableSession;
        err = mCASESessionTest.mCASESessionCache.Get(ResumptionID(mCASESessionTest.mCASESessionCachableArray[i].mResumptionId),
                                                     outCachableSession);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, true == mCASESessionTest.isEqual(i, outCachableSession));
    }
}

static void CASESessionCache_Add_When_Full_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mCASESessionTest.createCASESessionTestCachable(CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE);
    err = mCASESessionTest.mCASESessionCache.Add(
        mCASESessionTest.mCASESessionCachableArray[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE]);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Check if the entry with lowest timestamp has been removed
    CASESessionCachable outCachableSession;
    err = mCASESessionTest.mCASESessionCache.Get(ResumptionID(mCASESessionTest.mCASESessionCachableArray[0].mResumptionId),
                                                 outCachableSession);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Check if the new entry has been added.
    err = mCASESessionTest.mCASESessionCache.Get(
        ResumptionID(mCASESessionTest.mCASESessionCachableArray[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE].mResumptionId),
        outCachableSession);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, true == mCASESessionTest.isEqual(CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE, outCachableSession));
}

static void CASESessionCache_Remove_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint8_t i = 1; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE + 1; i++)
    {
        err = mCASESessionTest.mCASESessionCache.Remove(ResumptionID(mCASESessionTest.mCASESessionCachableArray[i].mResumptionId));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        CASESessionCachable outCachableSession;
        err = mCASESessionTest.mCASESessionCache.Get(ResumptionID(mCASESessionTest.mCASESessionCachableArray[i].mResumptionId),
                                                     outCachableSession);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Create",    CASESessionCache_Create_Test),
    NL_TEST_DEF("Add",    CASESessionCache_Add_Test),
    NL_TEST_DEF("Get",   CASESessionCache_Get_Test),
    NL_TEST_DEF("AddWhenFull", CASESessionCache_Add_When_Full_Test),
    NL_TEST_DEF("Remove", CASESessionCache_Remove_Test),

    NL_TEST_SENTINEL()
};
// clang-format on

int CASESessionCache_Test_Setup(void * inContext);
int CASESessionCache_Test_Teardown(void * inContext);

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SecurePairing-CASECache",
    &sTests[0],
    CASESessionCache_Test_Setup,
    CASESessionCache_Test_Teardown,
};
// clang-format on

namespace {
/*
 *  Set up the test suite.
 */
CHIP_ERROR CASETestCacheSetup(void * inContext)
{
    ReturnErrorOnFailure(chip::Platform::MemoryInit());
    return CHIP_NO_ERROR;
}
} // anonymous namespace

/**
 *  Set up the test suite.
 */
int CASESessionCache_Test_Setup(void * inContext)
{
    return CASETestCacheSetup(inContext) == CHIP_NO_ERROR ? SUCCESS : FAILURE;
}

/**
 *  Tear down the test suite.
 */
int CASESessionCache_Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *  Main
 */
int TestCASESessionCache()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCASESessionCache)
