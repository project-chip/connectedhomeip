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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>

constexpr chip::FabricIndex fabric1 = 10;
constexpr chip::NodeId node1        = 12344321;
constexpr chip::FabricIndex fabric2 = 14;
constexpr chip::NodeId node2        = 11223344;

TEST(TestSimpleSessionResumptionStorage, TestLink)
{
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    sessionStorage.Init(&storage);

    chip::SimpleSessionResumptionStorage::ResumptionIdStorage resumptionId;
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(resumptionId.data(), resumptionId.size()), CHIP_NO_ERROR);

    EXPECT_EQ(sessionStorage.SaveLink(resumptionId, chip::ScopedNodeId(node1, fabric1)), CHIP_NO_ERROR);

    chip::ScopedNodeId node;
    EXPECT_EQ(sessionStorage.LoadLink(resumptionId, node), CHIP_NO_ERROR);
    EXPECT_EQ(node, chip::ScopedNodeId(node1, fabric1));

    EXPECT_EQ(sessionStorage.DeleteLink(resumptionId), CHIP_NO_ERROR);

    EXPECT_EQ(sessionStorage.LoadLink(resumptionId, node), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

TEST(TestSimpleSessionResumptionStorage, TestState)
{
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    sessionStorage.Init(&storage);

    chip::ScopedNodeId node(node1, fabric1);

    chip::SimpleSessionResumptionStorage::ResumptionIdStorage resumptionId;
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(resumptionId.data(), resumptionId.size()), CHIP_NO_ERROR);

    chip::Crypto::P256ECDHDerivedSecret sharedSecret;
    sharedSecret.SetLength(sharedSecret.Capacity());
    EXPECT_EQ(chip::Crypto::DRBG_get_bytes(sharedSecret.Bytes(), sharedSecret.Length()), CHIP_NO_ERROR);

    chip::CATValues peerCATs;

    EXPECT_EQ(sessionStorage.SaveState(node, resumptionId, sharedSecret, peerCATs), CHIP_NO_ERROR);

    chip::SimpleSessionResumptionStorage::ResumptionIdStorage resumptionId2;
    chip::Crypto::P256ECDHDerivedSecret sharedSecret2;
    chip::CATValues peerCATs2;
    EXPECT_EQ(sessionStorage.LoadState(node, resumptionId2, sharedSecret2, peerCATs2), CHIP_NO_ERROR);
    EXPECT_EQ(resumptionId, resumptionId2);
    EXPECT_EQ(memcmp(sharedSecret.Bytes(), sharedSecret2.Bytes(), sharedSecret.Length()), 0);

    EXPECT_EQ(sessionStorage.DeleteState(node), CHIP_NO_ERROR);

    EXPECT_EQ(sessionStorage.LoadState(node, resumptionId2, sharedSecret2, peerCATs2),
              CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

TEST(TestSimpleSessionResumptionStorage, TestIndex)
{
    chip::TestPersistentStorageDelegate storage;
    chip::SimpleSessionResumptionStorage sessionStorage;
    sessionStorage.Init(&storage);

    chip::ScopedNodeId node(node1, fabric1);

    chip::DefaultSessionResumptionStorage::SessionIndex index0o;
    EXPECT_EQ(sessionStorage.LoadIndex(index0o), CHIP_NO_ERROR);
    EXPECT_EQ(index0o.mSize, 0u);

    chip::DefaultSessionResumptionStorage::SessionIndex index1;
    index1.mSize = 0;
    EXPECT_EQ(sessionStorage.SaveIndex(index1), CHIP_NO_ERROR);
    chip::DefaultSessionResumptionStorage::SessionIndex index1o;
    EXPECT_EQ(sessionStorage.LoadIndex(index1o), CHIP_NO_ERROR);
    EXPECT_EQ(index1o.mSize, 0u);

    chip::DefaultSessionResumptionStorage::SessionIndex index2;
    index2.mSize     = 2;
    index2.mNodes[0] = chip::ScopedNodeId(node1, fabric1);
    index2.mNodes[1] = chip::ScopedNodeId(node2, fabric2);
    EXPECT_EQ(CHIP_NO_ERROR, sessionStorage.SaveIndex(index2));
    chip::DefaultSessionResumptionStorage::SessionIndex index2o;
    EXPECT_EQ(CHIP_NO_ERROR, sessionStorage.LoadIndex(index2o));
    EXPECT_EQ(index2o.mSize, 2u);
    EXPECT_EQ(index2o.mNodes[0], chip::ScopedNodeId(node1, fabric1));
    EXPECT_EQ(index2o.mNodes[1], chip::ScopedNodeId(node2, fabric2));
}

// Test Suite
