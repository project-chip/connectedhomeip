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

using TestContext = chip::Test::MessagingContext;

namespace {
TransportMgrBase gTransportMgr;
Test::LoopbackTransport gLoopback;
chip::Test::IOContext gIOContext;

NodeId sTest_PeerId = 0xEDEDEDED00010001;

uint8_t sTest_Version = 1;

uint8_t sTest_SharedSecret[] = {
    0x7d, 0x73, 0x5b, 0xef, 0xe9, 0x16, 0xa1, 0xc0, 0xca, 0x02, 0xf8, 0xca, 0x98, 0x81, 0xe4, 0x26,
    0x63, 0xaa, 0xaf, 0x9a, 0xb9, 0xc4, 0x33, 0xb2, 0x89, 0xbe, 0x26, 0x70, 0x10, 0x75, 0x74, 0x10,
};

uint8_t sTest_MessageDigest[] = {
    0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint16_t sTest_LocalSessionId = 400;
uint16_t sTest_PeerSessionId  = 900;

uint8_t sTest_ResumptionId[kCASEResumptionIDSize] = { 0 };

} // namespace

class CASESessionTest : public CASESession
{
public:
    void createCASESessionTestSerializable(uint8_t i)
    {
        uint16_t sharedSecretLen = sizeof(sTest_SharedSecret);
        sTest_SharedSecret[sharedSecretLen - 1] += i;
        uint16_t messageDigestLen = sizeof(sTest_MessageDigest);
        sTest_MessageDigest[messageDigestLen - 1] += i;
        uint64_t timestamp = static_cast<uint64_t>(4000 + i * 1000);
        sTest_ResumptionId[kCASEResumptionIDSize - 1] += i;

        mCASESessionSerializableArray[i].mVersion         = sTest_Version;
        mCASESessionSerializableArray[i].mSharedSecretLen = sharedSecretLen;
        memcpy(mCASESessionSerializableArray[i].mSharedSecret, sTest_SharedSecret, sharedSecretLen);
        mCASESessionSerializableArray[i].mMessageDigestLen = messageDigestLen;
        memcpy(mCASESessionSerializableArray[i].mMessageDigest, sTest_MessageDigest, messageDigestLen);
        mCASESessionSerializableArray[i].mPeerNodeId     = sTest_PeerId + static_cast<uint16_t>(i);
        mCASESessionSerializableArray[i].mLocalSessionId = sTest_LocalSessionId + static_cast<uint16_t>(i);
        mCASESessionSerializableArray[i].mPeerSessionId  = sTest_PeerSessionId + static_cast<uint16_t>(i);
        memcpy(mCASESessionSerializableArray[i].mResumptionId, sTest_ResumptionId, kCASEResumptionIDSize);
        mCASESessionSerializableArray[i].mLocalFabricIndex      = 0;
        mCASESessionSerializableArray[i].mSessionSetupTimeStamp = timestamp;
    }

    bool isEqual(int index, CASESessionSerializable serializable)
    {
        return (serializable.mVersion == mCASESessionSerializableArray[index].mVersion) &&
            (serializable.mSharedSecretLen == mCASESessionSerializableArray[index].mSharedSecretLen) &&
            ((ResumptionID(serializable.mSharedSecret)).data_equal(ResumptionID(mCASESessionSerializableArray[index].mSharedSecret))) &&
            (serializable.mMessageDigestLen == mCASESessionSerializableArray[index].mMessageDigestLen) &&
            ((ResumptionID(serializable.mMessageDigest)).data_equal(ResumptionID(mCASESessionSerializableArray[index].mMessageDigest))) &&
            (serializable.mPeerNodeId == mCASESessionSerializableArray[index].mPeerNodeId) &&
            (serializable.mLocalSessionId == mCASESessionSerializableArray[index].mLocalSessionId) &&
            (serializable.mPeerSessionId == mCASESessionSerializableArray[index].mPeerSessionId) &&
            ((ResumptionID(serializable.mResumptionId)).data_equal(ResumptionID(mCASESessionSerializableArray[index].mResumptionId))) &&
            (serializable.mLocalFabricIndex == mCASESessionSerializableArray[index].mLocalFabricIndex) &&
            (serializable.mSessionSetupTimeStamp == mCASESessionSerializableArray[index].mSessionSetupTimeStamp);
    }

    void InitializeCASESessionSerializableArray()
    {
        for (size_t j = 0; j < kCASEResumptionIDSize; j++)
        {
            sTest_ResumptionId[j] = 0x01;
        }
        for (uint8_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; i++)
        {
            createCASESessionTestSerializable(i);
        }
    }

    CASESessionSerializable mCASESessionSerializableArray[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE + 1] = { { 0 } };
    CASESessionCache mCASESessionCache;
};

CASESessionTest mCASESessionTest;

static void CASESessionCache_Create_Test(nlTestSuite * inSuite, void * inContext)
{
    mCASESessionTest.InitializeCASESessionSerializableArray();
}

static void CASESessionCache_Add_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint8_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; i++)
    {
        CASESession session;
        err = session.FromSerializable(mCASESessionTest.mCASESessionSerializableArray[i]);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        err = mCASESessionTest.mCASESessionCache.Add(session);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }
}

static void CASESessionCache_Get_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint8_t i = 0; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE; i++)
    {
        CASESession outSession;
        err = mCASESessionTest.mCASESessionCache.Get(ResumptionID(mCASESessionTest.mCASESessionSerializableArray[i].mResumptionId),
                                                     outSession);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        CASESessionSerializable serializable;
        err = outSession.ToSerializable(serializable);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, true == mCASESessionTest.isEqual(i, serializable));
    }
}

static void CASESessionCache_Add_When_Full_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mCASESessionTest.createCASESessionTestSerializable(CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE);
    CASESession session;
    err = session.FromSerializable(mCASESessionTest.mCASESessionSerializableArray[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE]);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = mCASESessionTest.mCASESessionCache.Add(session);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Check if the entry with lowest timestamp has been removed
    CASESession outSession;
    err = mCASESessionTest.mCASESessionCache.Get(ResumptionID(mCASESessionTest.mCASESessionSerializableArray[0].mResumptionId),
                                                 outSession);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    // Check if the new entry has been added.
    err = mCASESessionTest.mCASESessionCache.Get(
        ResumptionID(mCASESessionTest.mCASESessionSerializableArray[CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE].mResumptionId),
        outSession);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    CASESessionSerializable serializable;
    err = outSession.ToSerializable(serializable);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, true == mCASESessionTest.isEqual(CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE, serializable));
}

static void CASESessionCache_Remove_Test(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (uint8_t i = 1; i < CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE + 1; i++)
    {
        err = mCASESessionTest.mCASESessionCache.Remove(ResumptionID(mCASESessionTest.mCASESessionSerializableArray[i].mResumptionId));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        CASESession outSession;
        err = mCASESessionTest.mCASESessionCache.Get(ResumptionID(mCASESessionTest.mCASESessionSerializableArray[i].mResumptionId),
                                                     outSession);
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

static TestContext sContext;

namespace {
/*
 *  Set up the test suite.
 */
CHIP_ERROR CASETestCacheSetup(void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ReturnErrorOnFailure(chip::Platform::MemoryInit());

    gTransportMgr.Init(&gLoopback);
    ReturnErrorOnFailure(gIOContext.Init());

    ReturnErrorOnFailure(ctx.Init(&gTransportMgr, &gIOContext));

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
    reinterpret_cast<TestContext *>(inContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *  Main
 */
int TestCASESessionCache()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCASESessionCache)
