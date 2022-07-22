/*
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

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>

constexpr chip::FabricIndex fabric1 = 10;
constexpr chip::NodeId node1        = 12344321;
constexpr chip::FabricIndex fabric2 = 14;
constexpr chip::NodeId node2        = 11223344;

void TestLink(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    sessionStorage.Init(&storage);

    chip::SimpleSessionResumptionStorage::ResumptionIdStorage resumptionId;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(resumptionId.data(), resumptionId.size()));

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.SaveLink(resumptionId, chip::ScopedNodeId(node1, fabric1)));

    chip::ScopedNodeId node;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.LoadLink(resumptionId, node));
    NL_TEST_ASSERT(inSuite, node == chip::ScopedNodeId(node1, fabric1));

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.DeleteLink(resumptionId));

    NL_TEST_ASSERT(inSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == sessionStorage.LoadLink(resumptionId, node));
}

void TestState(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    sessionStorage.Init(&storage);

    chip::ScopedNodeId node(node1, fabric1);

    chip::SimpleSessionResumptionStorage::ResumptionIdStorage resumptionId;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(resumptionId.data(), resumptionId.size()));

    chip::Crypto::P256ECDHDerivedSecret sharedSecret;
    sharedSecret.SetLength(sharedSecret.Capacity());
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == chip::Crypto::DRBG_get_bytes(sharedSecret.Bytes(), sharedSecret.Length()));

    chip::CATValues peerCATs;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.SaveState(node, resumptionId, sharedSecret, peerCATs));

    chip::SimpleSessionResumptionStorage::ResumptionIdStorage resumptionId2;
    chip::Crypto::P256ECDHDerivedSecret sharedSecret2;
    chip::CATValues peerCATs2;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.LoadState(node, resumptionId2, sharedSecret2, peerCATs2));
    NL_TEST_ASSERT(inSuite, resumptionId == resumptionId2);
    NL_TEST_ASSERT(inSuite, memcmp(sharedSecret.Bytes(), sharedSecret2.Bytes(), sharedSecret.Length()) == 0);

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.DeleteState(node));

    NL_TEST_ASSERT(inSuite,
                   CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND ==
                       sessionStorage.LoadState(node, resumptionId2, sharedSecret2, peerCATs2));
}

void TestIndex(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    sessionStorage.Init(&storage);

    chip::ScopedNodeId node(node1, fabric1);

    chip::DefaultSessionResumptionStorage::SessionIndex index0o;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.LoadIndex(index0o));
    NL_TEST_ASSERT(inSuite, index0o.mSize == 0);

    chip::DefaultSessionResumptionStorage::SessionIndex index1;
    index1.mSize = 0;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.SaveIndex(index1));
    chip::DefaultSessionResumptionStorage::SessionIndex index1o;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.LoadIndex(index1o));
    NL_TEST_ASSERT(inSuite, index1o.mSize == 0);

    chip::DefaultSessionResumptionStorage::SessionIndex index2;
    index2.mSize     = 2;
    index2.mNodes[0] = chip::ScopedNodeId(node1, fabric1);
    index2.mNodes[1] = chip::ScopedNodeId(node2, fabric2);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.SaveIndex(index2));
    chip::DefaultSessionResumptionStorage::SessionIndex index2o;
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == sessionStorage.LoadIndex(index2o));
    NL_TEST_ASSERT(inSuite, index2o.mSize == 2);
    NL_TEST_ASSERT(inSuite, index2o.mNodes[0] == chip::ScopedNodeId(node1, fabric1));
    NL_TEST_ASSERT(inSuite, index2o.mNodes[1] == chip::ScopedNodeId(node2, fabric2));
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestLink", TestLink),
    NL_TEST_DEF("TestState", TestState),
    NL_TEST_DEF("TestIndex", TestState),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-SimpleSessionResumptionStorage",
    &sTests[0],
    nullptr,
    nullptr,
};
// clang-format on

/**
 *  Main
 */
int TestSimpleSessionResumptionStorage()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSimpleSessionResumptionStorage)
