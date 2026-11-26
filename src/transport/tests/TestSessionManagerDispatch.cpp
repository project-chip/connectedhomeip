/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements unit tests for the SessionManager implementation.
 */

#include <errno.h>
#include <memory>

#include <pw_unit_test/framework.h>

#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API // Up here in case some other header
                                              // includes SessionManager.h indirectly

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/AutoRelease.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SecureMessageCodec.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/tests/LoopbackTransportManager.h>

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Testing;
using namespace chip::Credentials;

using GroupInfo      = GroupDataProvider::GroupInfo;
using GroupKey       = GroupDataProvider::GroupKey;
using KeySet         = GroupDataProvider::KeySet;
using SecurityPolicy = GroupDataProvider::SecurityPolicy;

using TestContext = LoopbackTransportManager;

struct MessageTestEntry
{
    const char * name;

    const char * peerAddr;

    const char * payload;
    const char * plain;
    const char * encrypted;
    const char * privacy;

    size_t payloadLength;
    size_t plainLength;
    size_t encryptedLength;
    size_t privacyLength;

    const char * encryptKey;
    const char * privacyKey;
    const char * epochKey;

    const char * nonce;
    const char * privacyNonce;
    const char * compressedFabricId;

    const char * mic;

    uint16_t sessionId;
    NodeId peerNodeId;
    GroupId groupId;
    NodeId sourceNodeId;

    uint8_t expectedMessageCount;
};

struct MessageTestEntry theMessageTestVector[] = {
    // =======================================
    // PASE positive test cases
    // =======================================
    {
        .name     = "secure pase message (no payload)",
        .peerAddr = "::1",

        .payload   = "",
        .plain     = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x05\x64\xee\x0e\x20\x7d",
        .encrypted = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d"
                     "\x84\x7f\x53\x5c\x30\x07\xe6\x15\x0c\xd6\x58\x67\xf2\xb8\x17\xdb", // Includes MIC
        .privacy   = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d"
                     "\x84\x7f\x53\x5c\x30\x07\xe6\x15\x0c\xd6\x58\x67\xf2\xb8\x17\xdb", // Includes MIC

        .payloadLength   = 0,
        .plainLength     = 14,
        .encryptedLength = 30,
        .privacyLength   = 30,

        // TODO(#22830): unicast message tests must use test key currently
        .encryptKey = "\x5e\xde\xd2\x44\xe5\x53\x2b\x3c\xdc\x23\x40\x9d\xba\xd0\x52\xd2",

        .nonce = "\x00\x39\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",

        .sessionId  = 0x0bb8, // 3000
        .peerNodeId = 0x0000000000000000ULL,

        .expectedMessageCount = 1,
    },
    {
        .name     = "secure pase message (short payload)",
        .peerAddr = "::1",

        .payload   = "\x11\x22\x33\x44\x55",
        .plain     = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x05\x64\xee\x0e\x20\x7d\x11\x22\x33\x44\x55",
        .encrypted = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d\x0f\x7f\x88\x5d\xfb"
                     "\x2f\xaa\x89\x49\xcf\x73\x0a\x57\x28\xe0\x35\x46\x10\xa0\xc4\xa7", // Includes MIC
        .privacy   = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d\x0f\x7f\x88\x5d\xfb"
                     "\x2f\xaa\x89\x49\xcf\x73\x0a\x57\x28\xe0\x35\x46\x10\xa0\xc4\xa7", // Includes MIC

        .payloadLength   = 5,
        .plainLength     = 19,
        .encryptedLength = 35,
        .privacyLength   = 35,

        // TODO(#22830): unicast message tests must use test key currently
        .encryptKey = "\x5e\xde\xd2\x44\xe5\x53\x2b\x3c\xdc\x23\x40\x9d\xba\xd0\x52\xd2",

        .nonce = "\x00\x39\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",

        .sessionId  = 0x0bb8, // 3000
        .peerNodeId = 0x0000000000000000ULL,

        .expectedMessageCount = 1,
    },
    // =======================================
    // PASE negative test cases
    // =======================================
    {
        .name     = "secure pase message (no payload / wrong MIC)",
        .peerAddr = "::1",

        .payload   = "",
        .plain     = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x05\x64\xee\x0e\x20\x7d",
        .encrypted = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d"
                     "\x84\x7f\x53\x5c\x30\x07\xe6\x15\x0c\xd6\x58\x67\xf2\xb8\x17\xdd", // Includes wrong MIC
        .privacy   = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d"
                     "\x84\x7f\x53\x5c\x30\x07\xe6\x15\x0c\xd6\x58\x67\xf2\xb8\x17\xdd", // Includes wrong MIC

        .payloadLength   = 0,
        .plainLength     = 14,
        .encryptedLength = 30,
        .privacyLength   = 30,

        // TODO(#22830): unicast message tests must use test key currently
        .encryptKey = "\x5e\xde\xd2\x44\xe5\x53\x2b\x3c\xdc\x23\x40\x9d\xba\xd0\x52\xd2",

        .nonce = "\x00\x39\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",

        .sessionId  = 0x0bb8, // 3000
        .peerNodeId = 0x0000000000000000ULL,

        .expectedMessageCount = 0,
    },
    {
        .name     = "secure pase message (short payload / wrong MIC)",
        .peerAddr = "::1",

        .payload   = "\x11\x22\x33\x44\x55",
        .plain     = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x05\x64\xee\x0e\x20\x7d\x11\x22\x33\x44\x55",
        .encrypted = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d\x0f\x7f\x88\x5d\xfb"
                     "\x2f\xaa\x89\x49\xcf\x73\x0a\x57\x28\xe0\x35\x46\x10\xa0\xc4\xaa", // Includes wrong MIC
        .privacy   = "\x00\xb8\x0b\x00\x39\x30\x00\x00\x5a\x98\x9a\xe4\x2e\x8d\x0f\x7f\x88\x5d\xfb"
                     "\x2f\xaa\x89\x49\xcf\x73\x0a\x57\x28\xe0\x35\x46\x10\xa0\xc4\xaa", // Includes wrong MIC

        .payloadLength   = 5,
        .plainLength     = 19,
        .encryptedLength = 35,
        .privacyLength   = 35,

        // TODO(#22830): unicast message tests must use test key currently
        .encryptKey = "\x5e\xde\xd2\x44\xe5\x53\x2b\x3c\xdc\x23\x40\x9d\xba\xd0\x52\xd2",

        .nonce = "\x00\x39\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",

        .sessionId  = 0x0bb8, // 3000
        .peerNodeId = 0x0000000000000000ULL,

        .expectedMessageCount = 0,
    },
    {
        .name     = "secure pase message (short payload / drop when privacy enabled)",
        .peerAddr = "::1",

        .payload   = "\x11\x22\x33\x44\x55",
        .plain     = "\x00\xb8\x0b\x80\x39\x30\x00\x00\x05\x64\xee\x0e\x20\x7d\x11\x22\x33\x44\x55",
        .encrypted = "\x00\xb8\x0b\x80\x39\x30\x00\x00\xaa\x26\xa0\xf9\x01\xef\xce\x9f\x9a\x67\xc8"
                     "\x13\x79\x17\xd1\x5b\x81\xd1\x5d\x31\x33\x08\x31\x97\x58\xea\x3f", // Includes MIC
        .privacy   = "\x00\xb8\x0b\x80\x87\xbe\xef\x06\xaa\x26\xa0\xf9\x01\xef\xce\x9f\x9a\x67\xc8"
                     "\x13\x79\x17\xd1\x5b\x81\xd1\x5d\x31\x33\x08\x31\x97\x58\xea\x3f", // Includes MIC

        .payloadLength   = 5,
        .plainLength     = 19,
        .encryptedLength = 35,
        .privacyLength   = 35,

        // TODO(#22830): unicast message tests must use test key currently
        .encryptKey = "\x5e\xde\xd2\x44\xe5\x53\x2b\x3c\xdc\x23\x40\x9d\xba\xd0\x52\xd2",

        .nonce        = "\x80\x39\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        .privacyNonce = "\x0b\xb8\x81\xd1\x5d\x31\x33\x08\x31\x97\x58\xea\x3f",

        .sessionId  = 0x0bb8, // 3000
        .peerNodeId = 0x0000000000000000ULL,

        .expectedMessageCount = 0,
    },
#if !CHIP_CONFIG_SECURITY_TEST_MODE
    // =======================================
    // GROUP positive test cases
    // =======================================
    {
        .name     = "secure group message (no privacy)",
        .peerAddr = "::1",

        .payload = "",

        // messageCounter = 0x12345678 // each group use case must increment this to pass replay.
        .plain     = "\06\x7d\xdb\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x64\xee\x0e\x20\x7d",
        .encrypted = "\x06\x7d\xdb\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x65\xc7\x67\xbc\x6c\xda"
                     "\x01\x06\xc9\x80\x13\x23\x90\x0e\x9b\x3c\xe6\xd4\xbb\x03\x27\xd6", // Includes MIC
        .privacy   = "\x06\x7d\xdb\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x65\xc7\x67\xbc\x6c\xda"
                     "\x01\x06\xc9\x80\x13\x23\x90\x0e\x9b\x3c\xe6\xd4\xbb\x03\x27\xd6", // Includes MIC

        .payloadLength   = 0,
        .plainLength     = 24,
        .encryptedLength = 40,
        .privacyLength   = 40,

        .encryptKey = "\xca\x92\xd7\xa0\x94\x2d\x1a\x51\x1a\x0e\x26\xad\x07\x4f\x4c\x2f",
        .privacyKey = "\xbf\xe9\xda\x01\x6a\x76\x53\x65\xf2\xdd\x97\xa9\xf9\x39\xe4\x25",
        .epochKey   = "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf",

        .nonce        = "\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00",
        .privacyNonce = "\xdb\x7d\x23\x90\x0e\x9b\x3c\xe6\xd4\xbb\x03\x27\xd6",

        .sessionId    = 0xdb7d, // 56189
        .peerNodeId   = 0x0000000000000000ULL,
        .groupId      = 2,
        .sourceNodeId = 0x0000000000000002ULL,

        .expectedMessageCount = 1,
    },
    {
        .name     = "secure group message (no privacy, drop replay)",
        .peerAddr = "::1",

        .payload = "",

        // messageCounter = 0x12345678 // each group use case must increment this to pass replay.
        .plain     = "\06\x7d\xdb\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x64\xee\x0e\x20\x7d",
        .encrypted = "\x06\x7d\xdb\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x65\xc7\x67\xbc\x6c\xda"
                     "\x01\x06\xc9\x80\x13\x23\x90\x0e\x9b\x3c\xe6\xd4\xbb\x03\x27\xd6", // Includes MIC
        .privacy   = "\x06\x7d\xdb\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x65\xc7\x67\xbc\x6c\xda"
                     "\x01\x06\xc9\x80\x13\x23\x90\x0e\x9b\x3c\xe6\xd4\xbb\x03\x27\xd6", // Includes MIC

        .payloadLength   = 0,
        .plainLength     = 24,
        .encryptedLength = 40,
        .privacyLength   = 40,

        .encryptKey = "\xca\x92\xd7\xa0\x94\x2d\x1a\x51\x1a\x0e\x26\xad\x07\x4f\x4c\x2f",
        .privacyKey = "\xbf\xe9\xda\x01\x6a\x76\x53\x65\xf2\xdd\x97\xa9\xf9\x39\xe4\x25",
        .epochKey   = "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf",

        .nonce        = "\x01\x78\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00",
        .privacyNonce = "\xdb\x7d\x23\x90\x0e\x9b\x3c\xe6\xd4\xbb\x03\x27\xd6",

        .sessionId    = 0xdb7d, // 56189
        .peerNodeId   = 0x0000000000000000ULL,
        .groupId      = 2,
        .sourceNodeId = 0x0000000000000002ULL,

        .expectedMessageCount = 0, ///< same test vector as above, but drops due to replay protection
    },
    {
        .name     = "private group message",
        .peerAddr = "::1",

        .payload = "",

        // messageCounter = 0x12345679
        .plain     = "\x06\x7d\xdb\x81\x79\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x64\xee\x0e\x20\x7d",
        .encrypted = "\x06\x7d\xdb\x81\x79\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x2b\x2f\x91\x5a\x66\xc9"
                     "\x59\x62\x90\xeb\xe4\x40\x82\x17\xb3\xc0\xc9\x21\xa2\xfc\xa4\xe1",
        .privacy   = "\x06\x7d\xdb\x81\xd9\x26\xaf\xce\x24\xc8\xa0\x98\x1b\xdd\x44\xf4\xe7\x30\x2b\x2f\x91\x5a\x66\xc9"
                     "\x59\x62\x90\xeb\xe4\x40\x82\x17\xb3\xc0\xc9\x21\xa2\xfc\xa4\xe1",

        .payloadLength   = 0,
        .plainLength     = 24,
        .encryptedLength = 40,
        .privacyLength   = 40,

        .encryptKey = "\xca\x92\xd7\xa0\x94\x2d\x1a\x51\x1a\x0e\x26\xad\x07\x4f\x4c\x2f",
        .privacyKey = "\xbf\xe9\xda\x01\x6a\x76\x53\x65\xf2\xdd\x97\xa9\xf9\x39\xe4\x25",
        .epochKey   = "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf",

        .nonce        = "\x01\x79\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00",
        .privacyNonce = "\xdb\x7d\x40\x82\x17\xb3\xc0\xc9\x21\xa2\xfc\xa4\xe1",

        .sessionId    = 0xdb7d, // 56189
        .peerNodeId   = 0x0000000000000000ULL,
        .groupId      = 2,
        .sourceNodeId = 0x0000000000000002ULL,

        .expectedMessageCount = 1,
    },
    // =======================================
    // GROUP negative test cases
    // =======================================
    {
        .name     = "private group message (wrong MIC)",
        .peerAddr = "::1",

        .payload = "",

        // messageCounter = 0x12345679
        .plain     = "\x06\x7d\xdb\x81\x79\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x64\xee\x0e\x20\x7d",
        .encrypted = "\x06\x7d\xdb\x81\x79\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x2b\x2f\x91\x5a\x66\xc9"
                     "\x59\x62\x90\xeb\xe4\x40\x82\x17\xb3\xc0\xc9\x21\xa2\xfc\xa4\xee",
        .privacy   = "\x06\x7d\xdb\x81\xd9\x26\xaf\xce\x24\xc8\xa0\x98\x1b\xdd\x44\xf4\xe7\x30\x2b\x2f\x91\x5a\x66\xc9"
                     "\x59\x62\x90\xeb\xe4\x40\x82\x17\xb3\xc0\xc9\x21\xa2\xfc\xa4\xee",

        .payloadLength   = 0,
        .plainLength     = 24,
        .encryptedLength = 40,
        .privacyLength   = 40,

        .encryptKey = "\xca\x92\xd7\xa0\x94\x2d\x1a\x51\x1a\x0e\x26\xad\x07\x4f\x4c\x2f",
        .privacyKey = "\xbf\xe9\xda\x01\x6a\x76\x53\x65\xf2\xdd\x97\xa9\xf9\x39\xe4\x25",
        .epochKey   = "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf",

        .nonce        = "\x01\x79\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00",
        .privacyNonce = "\xdb\x7d\x40\x82\x17\xb3\xc0\xc9\x21\xa2\xfc\xa4\xe1",

        .sessionId    = 0xdb7d, // 56189
        .peerNodeId   = 0x0000000000000000ULL,
        .groupId      = 2,
        .sourceNodeId = 0x0000000000000002ULL,

        .expectedMessageCount = 0,
    },
    {
        // Buffer is truncated to exactly 16 bytes (the MIC size), so it passes
        // the MIC extraction check but is far too short to hold the privacy header.
        //
        // msgFlags  = 0x06: version=0, kSourceNodeIdPresent (0x04), kDestinationGroupIdPresent (0x02)
        // sessionId = 0xdb7d (matches the group key set up by the epoch key below)
        // secFlags  = 0x81: kPrivacyFlag (0x80) | kGroupSession (0x01)
        //
        // PrivacyHeaderLength() = 4 (min) + 8 (source NodeId) + 2 (dest GroupId) = 14
        // Required buffer for privacy region: offset 4 + length 14 = 18 bytes > 16 bytes → OOB
        .name     = "private group message (privacy header exceeds buffer length)",
        .peerAddr = "::1",

        .payload = "",
        .plain   = "",
        .privacy = "\x06\x7d\xdb\x81\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",

        .payloadLength = 0,
        .plainLength   = 0,
        .privacyLength = 16,

        .epochKey = "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf",

        .sessionId    = 0xdb7d, // 56189
        .peerNodeId   = 0x0000000000000000ULL,
        .groupId      = 2,
        .sourceNodeId = 0x0000000000000002ULL,

        .expectedMessageCount = 0,
    },

#endif // !CHIP_CONFIG_SECURITY_TEST_MODE
};

const uint16_t theMessageTestVectorLength = sizeof(theMessageTestVector) / sizeof(theMessageTestVector[0]);

// Just enough init to replace a ton of boilerplate
constexpr FabricIndex kFabricIndex = kMinValidFabricIndex;
constexpr size_t kGroupIndex       = 0;

constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 8;

static chip::TestPersistentStorageDelegate sStorageDelegate;
static chip::Crypto::DefaultSessionKeystore sSessionKeystore;
static GroupDataProviderImpl sProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);
class FabricTableHolder
{
public:
    FabricTableHolder() {}
    ~FabricTableHolder()
    {
        mFabricTable.Shutdown();
        mOpKeyStore.Finish();
        mOpCertStore.Finish();
    }

    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mOpKeyStore.Init(&mStorage));
        ReturnErrorOnFailure(mOpCertStore.Init(&mStorage));

        // Initialize Group Data Provider
        sProvider.SetStorageDelegate(&sStorageDelegate);
        sProvider.SetSessionKeystore(&sSessionKeystore);
        // sProvider.SetListener(&chip::app::TestGroups::sListener);
        ReturnErrorOnFailure(sProvider.Init());
        Credentials::SetGroupDataProvider(&sProvider);

        // Initialize Fabric Table
        chip::FabricTable::InitParams initParams;
        initParams.storage             = &mStorage;
        initParams.operationalKeystore = &mOpKeyStore;
        initParams.opCertStore         = &mOpCertStore;

        return mFabricTable.Init(initParams);
    }

    FabricTable & GetFabricTable() { return mFabricTable; }

private:
    chip::FabricTable mFabricTable;
    chip::TestPersistentStorageDelegate mStorage;
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
};

class TestSessionManagerCallback : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf) override
    {
        mReceivedCount++;

        MessageTestEntry & testEntry = theMessageTestVector[mTestVectorIndex];

        ChipLogProgress(Test, "OnMessageReceived: sessionId=0x%04x", testEntry.sessionId);
        EXPECT_EQ(header.GetSessionId(), testEntry.sessionId);

        size_t dataLength   = msgBuf->DataLength();
        size_t expectLength = testEntry.payloadLength;

        EXPECT_EQ(dataLength, expectLength);
        EXPECT_EQ(memcmp(msgBuf->Start(), testEntry.payload, dataLength), 0);

        ChipLogProgress(Test, "::: TestSessionManagerDispatch[%d] PASS", mTestVectorIndex);
    }

    void ResetTest(unsigned testVectorIndex)
    {
        mTestVectorIndex = testVectorIndex;
        mReceivedCount   = 0;
    }

    unsigned NumMessagesReceived() { return mReceivedCount; }

    unsigned mTestVectorIndex = 0;
    unsigned mReceivedCount   = 0;
};

PeerAddress AddressFromString(const char * str)
{
    Inet::IPAddress addr;

    VerifyOrDie(Inet::IPAddress::FromString(str, addr));

    return PeerAddress::UDP(addr);
}

/**
 * Holds resources needed by SessionManager tests.
 * Managed via std::unique_ptr in the test fixture to ensure they are destroyed
 * before Platform::MemoryShutdown() is called in TearDown.
 */
struct SessionManagerTestResources
{
    FabricTableHolder fabricTableHolder;
    secure_channel::MessageCounterManager messageCounterManager;
    chip::TestPersistentStorageDelegate deviceStorage;
    chip::Crypto::DefaultSessionKeystore sessionKeystore;

    CHIP_ERROR Init() { return fabricTableHolder.Init(); }
};

void TestSessionManagerInit(TestContext & ctx, SessionManager & sessionManager, SessionManagerTestResources & resources)
{
    EXPECT_EQ(CHIP_NO_ERROR,
              sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &resources.messageCounterManager,
                                  &resources.deviceStorage, &resources.fabricTableHolder.GetFabricTable(),
                                  resources.sessionKeystore));
}

// constexpr chip::FabricId kFabricId1               = 0x2906C908D115D362;
static const uint8_t kCompressedFabricIdBuffer1[] = { 0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30 };
constexpr ByteSpan kCompressedFabricId1(kCompressedFabricIdBuffer1);

CHIP_ERROR InjectGroupSessionWithTestKey(SessionHolder & sessionHolder, MessageTestEntry & testEntry)
{
    constexpr uint16_t kKeySetIndex = 0x0;

    GroupId groupId              = testEntry.groupId;
    GroupDataProvider * provider = GetGroupDataProvider();

    static KeySet sKeySet(kKeySetIndex, SecurityPolicy::kTrustFirst, 1);
    static GroupKey sGroupKeySet(groupId, kKeySetIndex);
    static GroupInfo sGroupInfo(groupId, "Name Matter Not");
    static Transport::IncomingGroupSession sSessionBobToFriends(groupId, kFabricIndex, testEntry.sourceNodeId);

    if (testEntry.epochKey)
    {
        memcpy(sKeySet.epoch_keys[0].key, testEntry.epochKey, 16);
        sKeySet.epoch_keys[0].start_time = 0;
        sGroupInfo.group_id              = groupId;
        sGroupKeySet.group_id            = groupId;

        ReturnErrorOnFailure(provider->SetKeySet(kFabricIndex, kCompressedFabricId1, sKeySet));
        ReturnErrorOnFailure(provider->SetGroupKeyAt(kFabricIndex, kGroupIndex, sGroupKeySet));
        ReturnErrorOnFailure(provider->SetGroupInfoAt(kFabricIndex, kGroupIndex, sGroupInfo));
    }

    sessionHolder = SessionHandle(sSessionBobToFriends);

    return CHIP_NO_ERROR;
}

class TestSessionManagerDispatch : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_EQ(mContext.Init(), CHIP_NO_ERROR);
        mResources = std::make_unique<SessionManagerTestResources>();
        ASSERT_EQ(mResources->Init(), CHIP_NO_ERROR);
    }
    void TearDown() override
    {
        // Force cleanup of resources (which may release platform memory)
        // before mContext.Shutdown() calls Platform::MemoryShutdown().
        mResources.reset();
        mContext.Shutdown();
    }

    TestContext mContext;
    std::unique_ptr<SessionManagerTestResources> mResources;
};

TEST_F(TestSessionManagerDispatch, TestSessionManagerDispatch)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SessionManager sessionManager;
    TestSessionManagerCallback callback;

    TestSessionManagerInit(mContext, sessionManager, *mResources);
    sessionManager.SetMessageDelegate(&callback);

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;
    SessionHolder testGroupSession;

    for (unsigned i = 0; i < theMessageTestVectorLength; i++)
    {
        MessageTestEntry & testEntry = theMessageTestVector[i];
        callback.ResetTest(i);

        ChipLogProgress(Test, "===> TestSessionManagerDispatch[%d] '%s': sessionId=0x%04x", i, testEntry.name, testEntry.sessionId);

        // TODO(#22830): inject raw keys rather than always defaulting to test key
        // TODO: switch on session type

        // Inject Sessions
        err = sessionManager.InjectPaseSessionWithTestKey(aliceToBobSession, testEntry.sessionId, testEntry.peerNodeId,
                                                          testEntry.sessionId, kFabricIndex, peer,
                                                          CryptoContext::SessionRole::kResponder);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = InjectGroupSessionWithTestKey(testGroupSession, testEntry);
        EXPECT_EQ(CHIP_NO_ERROR, err);

        const char * plain = testEntry.plain;
        const ByteSpan expectedPlain(reinterpret_cast<const uint8_t *>(plain), testEntry.plainLength);
        const char * privacy = testEntry.privacy;
        chip::System::PacketBufferHandle msg =
            chip::MessagePacketBuffer::NewWithData(reinterpret_cast<const uint8_t *>(privacy), testEntry.privacyLength);

        const PeerAddress peerAddress = AddressFromString(testEntry.peerAddr);
        sessionManager.OnMessageReceived(peerAddress, std::move(msg));
        EXPECT_EQ(callback.NumMessagesReceived(), testEntry.expectedMessageCount);

        if ((testEntry.expectedMessageCount == 0) && (callback.NumMessagesReceived() == 0))
        {
            ChipLogProgress(Test, "::: TestSessionManagerDispatch[%d] PASS (negative test case)", i);
        }
    }

    sessionManager.Shutdown();
}

#if !CHIP_CONFIG_SECURITY_TEST_MODE
class TestGroupPrivacyMessageDelegate : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf) override
    {
        mMessageReceived = true;
        mHeader          = header;
    }

    bool mMessageReceived = false;
    PacketHeader mHeader;
};

static void SetupGroupKeys(SessionManager & sessionManager, FabricIndex & fabricIndex, GroupId groupId, const char * epochKey)
{
    using namespace chip::TestCerts;

    // Injects a test fabric
    FabricTable * fabricTable = sessionManager.GetFabricTable();
    ASSERT_NE(nullptr, fabricTable);
    CHIP_ERROR err = fabricTable->AddNewFabricForTestIgnoringCollisions(
        GetRootACertAsset().mCert, GetIAA1CertAsset().mCert, GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey, &fabricIndex);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Extracts assigned 64-bit compressed fabric ID span.
    uint8_t compressedFabricBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricSpan(compressedFabricBuf);
    EXPECT_EQ(CHIP_NO_ERROR, fabricTable->FindFabricWithIndex(fabricIndex)->GetCompressedFabricIdBytes(compressedFabricSpan));

    // Get pointer to active group data provider
    GroupDataProvider * provider = GetGroupDataProvider();
    ASSERT_NE(nullptr, provider);

    // registers symmetric keys under non-zero KeySetID 0x0123.
    constexpr uint16_t kTestKeysetId = 0x0123;
    KeySet keySet(kTestKeysetId, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
    memcpy(keySet.epoch_keys[0].key, epochKey, 16);
    keySet.epoch_keys[0].start_time = 0;
    GroupKey groupKey(groupId, kTestKeysetId);
    GroupInfo groupInfo(groupId, "Privacy Group");

    // Setup Group key sets, group key maps, and group info
    EXPECT_EQ(CHIP_NO_ERROR, provider->SetKeySet(fabricIndex, compressedFabricSpan, keySet));
    EXPECT_EQ(CHIP_NO_ERROR, provider->SetGroupKeyAt(fabricIndex, 0, groupKey));
    EXPECT_EQ(CHIP_NO_ERROR, provider->SetGroupInfoAt(fabricIndex, 0, groupInfo));
}

TEST_F(TestSessionManagerDispatch, TestGroupPrepareMessagePrivacy)
{
    using namespace chip::TestCerts;

    SessionManager sessionManager;
    TestGroupPrivacyMessageDelegate delegate;
    TestSessionManagerInit(mContext, sessionManager, *mResources);
    sessionManager.SetMessageDelegate(&delegate);

    // Loads test parameters for GroupId 2
    const MessageTestEntry & testEntry = theMessageTestVector[7];

    FabricIndex fabricIndex = kUndefinedFabricIndex;
    SetupGroupKeys(sessionManager, fabricIndex, testEntry.groupId, testEntry.epochKey);

    // Instantiates outgoing (for PrepareMessage) session.
    Transport::OutgoingGroupSession outgoingSession(testEntry.groupId, fabricIndex);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    // Create the test payload header, data, and buffer
    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
    const char testPayload[] = "PrivacyTest";
    System::PacketBufferHandle payloadBuf =
        MessagePacketBuffer::NewWithData(reinterpret_cast<const uint8_t *>(testPayload), sizeof(testPayload));
    ASSERT_FALSE(payloadBuf.IsNull());

    // Prepare the group message
    EncryptedPacketBufferHandle preparedMessage;
    CHIP_ERROR err =
        sessionManager.PrepareMessage(outgoingHolder.Get().Value(), payloadHeader, std::move(payloadBuf), preparedMessage);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    // Unwraps the buffer and verifies PrepareMessage set the privacy flag bit high and set the group session type.
    PacketHeader decodedHeader;
    uint16_t headerSize                    = 0;
    System::PacketBufferHandle writableMsg = preparedMessage.CastToWritable();
    ASSERT_FALSE(writableMsg.IsNull());
    EXPECT_EQ(CHIP_NO_ERROR, decodedHeader.Decode(writableMsg->Start(), writableMsg->DataLength(), &headerSize));
    EXPECT_TRUE(decodedHeader.IsGroupSession());
    EXPECT_TRUE(decodedHeader.HasPrivacyFlag());

    // Feeds ciphertext back into transport dispatch to verify PrivacyDecrypt and MIC authentication.
    IPAddress loopbackAddress;
    IPAddress::FromString("::1", loopbackAddress);
    const PeerAddress peerAddress = PeerAddress::UDP(loopbackAddress, CHIP_PORT);
    sessionManager.OnMessageReceived(peerAddress, std::move(writableMsg));
    EXPECT_TRUE(delegate.mMessageReceived);

    // Verify decrypted fields match expectations
    EXPECT_EQ(delegate.mHeader.GetSessionId(), decodedHeader.GetSessionId());
    EXPECT_EQ(delegate.mHeader.GetDestinationGroupId().Value(), testEntry.groupId);

    FabricTable * fabricTable = sessionManager.GetFabricTable();
    ASSERT_NE(nullptr, fabricTable);
    NodeId expectedSourceNodeId = fabricTable->FindFabricWithIndex(fabricIndex)->GetNodeId();
    EXPECT_TRUE(delegate.mHeader.GetSourceNodeId().HasValue());
    EXPECT_EQ(delegate.mHeader.GetSourceNodeId().Value(), expectedSourceNodeId);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManagerDispatch, TestGroupIncomingPrivacyBoundsCheck)
{
    using namespace chip::TestCerts;

    SessionManager sessionManager;
    TestGroupPrivacyMessageDelegate delegate;
    TestSessionManagerInit(mContext, sessionManager, *mResources);
    sessionManager.SetMessageDelegate(&delegate);

    // Loads test parameters for GroupId 2
    const MessageTestEntry & testEntry = theMessageTestVector[7];

    FabricIndex fabricIndex = kUndefinedFabricIndex;
    SetupGroupKeys(sessionManager, fabricIndex, testEntry.groupId, testEntry.epochKey);

    // Instantiates outgoing (for PrepareMessage) session.
    Transport::OutgoingGroupSession outgoingSession(testEntry.groupId, fabricIndex);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    // Create the test payload header, data, and buffer
    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
    const char testPayload[] = "PrivacyTest";
    System::PacketBufferHandle payloadBuf =
        MessagePacketBuffer::NewWithData(reinterpret_cast<const uint8_t *>(testPayload), sizeof(testPayload));
    ASSERT_FALSE(payloadBuf.IsNull());

    // Prepare the group message
    EncryptedPacketBufferHandle preparedMessage;
    CHIP_ERROR err =
        sessionManager.PrepareMessage(outgoingHolder.Get().Value(), payloadHeader, std::move(payloadBuf), preparedMessage);
    EXPECT_EQ(CHIP_NO_ERROR, err);

    System::PacketBufferHandle writableMsg = preparedMessage.CastToWritable();
    ASSERT_FALSE(writableMsg.IsNull());

    // Shrink the buffer to trigger the bounds check failure in GroupKeyDecryptAttempt.
    writableMsg->SetDataLength(1);

    IPAddress loopbackAddress;
    IPAddress::FromString("::1", loopbackAddress);
    const PeerAddress peerAddress = PeerAddress::UDP(loopbackAddress, CHIP_PORT);
    sessionManager.OnMessageReceived(peerAddress, std::move(writableMsg));

    // The message should be discarded and NOT received by the delegate.
    EXPECT_FALSE(delegate.mMessageReceived);

    sessionManager.Shutdown();
}

TEST_F(TestSessionManagerDispatch, TestGroupPrepareMessageChainedBufferFailure)
{
    using namespace chip::TestCerts;

    SessionManager sessionManager;
    TestSessionManagerInit(mContext, sessionManager, *mResources);

    // Loads test parameters for GroupId 2
    const MessageTestEntry & testEntry = theMessageTestVector[7];

    FabricIndex fabricIndex = kUndefinedFabricIndex;
    SetupGroupKeys(sessionManager, fabricIndex, testEntry.groupId, testEntry.epochKey);

    // Instantiates outgoing (for PrepareMessage) session.
    Transport::OutgoingGroupSession outgoingSession(testEntry.groupId, fabricIndex);
    SessionHandle outgoingHandle(outgoingSession);
    SessionHolder outgoingHolder(outgoingHandle);

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);

    // Create a chained buffer
    System::PacketBufferHandle buf1 = MessagePacketBuffer::New(0);
    System::PacketBufferHandle buf2 = MessagePacketBuffer::New(0);
    ASSERT_FALSE(buf1.IsNull());
    ASSERT_FALSE(buf2.IsNull());
    buf1.AddToEnd(std::move(buf2));

    EXPECT_TRUE(buf1->HasChainedBuffer());

    EncryptedPacketBufferHandle preparedMessage;
    CHIP_ERROR err = sessionManager.PrepareMessage(outgoingHolder.Get().Value(), payloadHeader, std::move(buf1), preparedMessage);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    sessionManager.Shutdown();
}
#endif // !CHIP_CONFIG_SECURITY_TEST_MODE

// A message that fails to decrypt must not update the cached peer address of the
// session it names.
TEST_F(TestSessionManagerDispatch, TestUndecryptableMessageDoesNotRebindPeerAddress)
{
    SessionManager sessionManager;
    TestSessionManagerInit(mContext, sessionManager, *mResources);

    constexpr uint16_t kLocalSessionId   = 0x1234;
    constexpr NodeId kSessionPeerNodeId  = 0x0000000000000002ULL;
    const PeerAddress establishedAddress = AddressFromString("fe80::1");
    const PeerAddress spoofedAddress     = AddressFromString("fe80::2");

    SessionHolder sessionHolder;
    ASSERT_SUCCESS(sessionManager.InjectPaseSessionWithTestKey(sessionHolder, kLocalSessionId, kSessionPeerNodeId, kLocalSessionId,
                                                               kFabricIndex, establishedAddress,
                                                               CryptoContext::SessionRole::kResponder));

    SecureSession * secureSession = sessionHolder.Get().Value()->AsSecureSession();
    ASSERT_EQ(secureSession->GetPeerAddress(), establishedAddress);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);

    const uint8_t kPayload[]           = { 0x11, 0x22, 0x33, 0x44 };
    System::PacketBufferHandle payload = MessagePacketBuffer::NewWithData(kPayload, sizeof(kPayload));
    ASSERT_FALSE(payload.IsNull());

    EncryptedPacketBufferHandle preparedMessage;
    ASSERT_SUCCESS(sessionManager.PrepareMessage(sessionHolder.Get().Value(), payloadHeader, std::move(payload), preparedMessage));

    // Corrupt the trailing integrity check so the message parses but cannot be
    // authenticated.
    System::PacketBufferHandle msg = preparedMessage.CastToWritable();
    ASSERT_FALSE(msg.IsNull());
    ASSERT_GT(msg->DataLength(), 0u);
    msg->Start()[msg->DataLength() - 1] = static_cast<uint8_t>(msg->Start()[msg->DataLength() - 1] ^ 0xff);

    sessionManager.OnMessageReceived(spoofedAddress, std::move(msg));

    EXPECT_EQ(secureSession->GetPeerAddress(), establishedAddress);

    sessionManager.Shutdown();
}

// Injects the two halves of a PASE session pair so that a message prepared on
// `initiator` decrypts on `responder`.
CHIP_ERROR InjectSessionPair(SessionManager & sessionManager, SessionHolder & initiator, SessionHolder & responder,
                             const PeerAddress & peerAddress)
{
    ReturnErrorOnFailure(sessionManager.InjectPaseSessionWithTestKey(initiator, 2, 0x0000000000000002ULL, 1, kFabricIndex,
                                                                     peerAddress, CryptoContext::SessionRole::kInitiator));
    return sessionManager.InjectPaseSessionWithTestKey(responder, 1, 0x0000000000000001ULL, 2, kFabricIndex, peerAddress,
                                                       CryptoContext::SessionRole::kResponder);
}

// Prepares an encrypted message on `session` carrying a fixed payload.
CHIP_ERROR PrepareTestMessage(SessionManager & sessionManager, const SessionHandle & session,
                              EncryptedPacketBufferHandle & prepared)
{
    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(0);
    payloadHeader.SetMessageType(chip::Protocols::InteractionModel::MsgType::InvokeCommandRequest);
    payloadHeader.SetInitiator(true);
    // Reliable, so a duplicate is not dropped at the no-ack fast path and reaches
    // the peer address update.
    payloadHeader.SetNeedsAck(true);

    const uint8_t kPayload[]           = { 0x11, 0x22, 0x33, 0x44 };
    System::PacketBufferHandle payload = MessagePacketBuffer::NewWithData(kPayload, sizeof(kPayload));
    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_NO_MEMORY);
    return sessionManager.PrepareMessage(session, payloadHeader, std::move(payload), prepared);
}

// A replay of an already accepted message is authentic, so it must not be able to
// update the session's cached peer address.
TEST_F(TestSessionManagerDispatch, TestReplayedMessageDoesNotRebindPeerAddress)
{
    SessionManager sessionManager;
    TestSessionManagerInit(mContext, sessionManager, *mResources);

    const PeerAddress establishedAddress = AddressFromString("fe80::1");
    const PeerAddress replayAddress      = AddressFromString("fe80::2");

    SessionHolder initiator;
    SessionHolder responder;
    ASSERT_SUCCESS(InjectSessionPair(sessionManager, initiator, responder, establishedAddress));
    SecureSession * receiver = responder.Get().Value()->AsSecureSession();

    EncryptedPacketBufferHandle prepared;
    ASSERT_SUCCESS(PrepareTestMessage(sessionManager, initiator.Get().Value(), prepared));
    ASSERT_FALSE(prepared.IsNull());

    EncryptedPacketBufferHandle firstDelivery = prepared.CloneData();
    ASSERT_FALSE(firstDelivery.IsNull());
    sessionManager.OnMessageReceived(establishedAddress, firstDelivery.CastToWritable());
    ASSERT_EQ(receiver->GetPeerAddress(), establishedAddress);

    EncryptedPacketBufferHandle replay = prepared.CloneData();
    ASSERT_FALSE(replay.IsNull());
    sessionManager.OnMessageReceived(replayAddress, replay.CastToWritable());

    EXPECT_EQ(receiver->GetPeerAddress(), establishedAddress);

    sessionManager.Shutdown();
}

// A peer that moves to a new address is still tracked: the first message that is
// authentic and not a replay updates the cached address.
TEST_F(TestSessionManagerDispatch, TestAcceptedMessageFromNewAddressRebindsPeerAddress)
{
    SessionManager sessionManager;
    TestSessionManagerInit(mContext, sessionManager, *mResources);

    const PeerAddress establishedAddress = AddressFromString("fe80::1");
    const PeerAddress newAddress         = AddressFromString("fe80::2");

    SessionHolder initiator;
    SessionHolder responder;
    ASSERT_SUCCESS(InjectSessionPair(sessionManager, initiator, responder, establishedAddress));
    SecureSession * receiver = responder.Get().Value()->AsSecureSession();

    EncryptedPacketBufferHandle prepared;
    ASSERT_SUCCESS(PrepareTestMessage(sessionManager, initiator.Get().Value(), prepared));
    ASSERT_FALSE(prepared.IsNull());

    sessionManager.OnMessageReceived(newAddress, prepared.CastToWritable());

    EXPECT_EQ(receiver->GetPeerAddress(), newAddress);

    sessionManager.Shutdown();
}

class CountingGroupDelegate final : public SessionMessageDelegate
{
public:
    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader &, const SessionHandle & session,
                           DuplicateMessage, System::PacketBufferHandle &&) override
    {
        mReceivedCount++;
        mLastCounter = packetHeader.GetMessageCounter();
        mLastGroupId = session->AsIncomingGroupSession()->GetGroupId();
    }

    size_t mReceivedCount = 0;
    uint32_t mLastCounter = 0;
    GroupId mLastGroupId  = kUndefinedGroupId;
};

struct GroupKeySetDispatchContext
{
    static constexpr FabricIndex kFabric = kFabricIndex;

    Credentials::GroupDataProvider * mPreviousProvider = nullptr;
    TestPersistentStorageDelegate mProviderStorage;
    Crypto::DefaultSessionKeystore mProviderKeystore;
    GroupDataProviderImpl mProvider;
    SessionManager mSessionManager;
    CountingGroupDelegate mDelegate;
    PeerAddress mPeerAddress;

    GroupKeySetDispatchContext() : mPreviousProvider(GetGroupDataProvider()) {}

    ~GroupKeySetDispatchContext()
    {
        mSessionManager.FabricRemoved(kFabric);
        mSessionManager.Shutdown();
        mProvider.Finish();
        SetGroupDataProvider(mPreviousProvider);
    }

    CHIP_ERROR Init(TestContext & transportCtx, SessionManagerTestResources & resources)
    {
        mProvider.SetStorageDelegate(&mProviderStorage);
        mProvider.SetSessionKeystore(&mProviderKeystore);
        ReturnErrorOnFailure(mProvider.Init());
        SetGroupDataProvider(&mProvider);

        ReturnErrorOnFailure(mSessionManager.Init(&transportCtx.GetSystemLayer(), &transportCtx.GetTransportMgr(),
                                                  &resources.messageCounterManager, &resources.deviceStorage,
                                                  &resources.fabricTableHolder.GetFabricTable(), resources.sessionKeystore));
        mSessionManager.SetMessageDelegate(&mDelegate);
        mSessionManager.FabricRemoved(kFabric);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR InstallGroupKeySet(KeysetId keysetId, const uint8_t (&epochKey)[16])
    {
        KeySet keySet(keysetId, SecurityPolicy::kTrustFirst, 1);
        keySet.epoch_keys[0].start_time = 0;
        memcpy(keySet.epoch_keys[0].key, epochKey, sizeof(epochKey));
        return mProvider.SetKeySet(kFabric, kCompressedFabricId1, keySet);
    }

    CHIP_ERROR MapGroupToKeySet(size_t mapIndex, GroupId groupId, KeysetId keysetId)
    {
        ReturnErrorOnFailure(mProvider.SetGroupKeyAt(kFabric, mapIndex, GroupKey(groupId, keysetId)));
        return mProvider.SetGroupInfoAt(kFabric, mapIndex, GroupInfo(groupId, "TestGroup"));
    }

    CHIP_ERROR BuildEncryptedGroupPacket(GroupId groupId, NodeId sourceNodeId, uint32_t counter,
                                         System::PacketBufferHandle & outPacket)
    {
        Crypto::SymmetricKeyContext * keyContext = mProvider.GetKeyContext(kFabric, groupId);
        VerifyOrReturnError(keyContext != nullptr, CHIP_ERROR_INTERNAL);
        AutoRelease<Crypto::SymmetricKeyContext> keyContextOwner(keyContext);

        PayloadHeader payloadHeader;
        payloadHeader.SetMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest);

        const uint8_t kPayload[]       = { 0x15, 0x18 };
        System::PacketBufferHandle msg = MessagePacketBuffer::NewWithData(kPayload, sizeof(kPayload));
        VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

        PacketHeader packetHeader;
        packetHeader.SetDestinationGroupId(groupId);
        packetHeader.SetMessageCounter(counter);
        packetHeader.SetSessionType(Header::SessionType::kGroupSession);
        packetHeader.SetFlags(Header::SecFlagValues::kPrivacyFlag);
        packetHeader.SetSourceNodeId(sourceNodeId);
        packetHeader.SetSessionId(keyContext->GetKeyHash());

        CryptoContext cryptoContext(keyContext);
        CryptoContext::NonceStorage nonce;
        ReturnErrorOnFailure(CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), counter, sourceNodeId));
        ReturnErrorOnFailure(SecureMessageCodec::Encrypt(cryptoContext, nonce, payloadHeader, packetHeader, msg));
        ReturnErrorOnFailure(packetHeader.EncodeBeforeData(msg));

        uint8_t * data     = msg->Start();
        size_t len         = msg->TotalLength();
        uint16_t footerLen = packetHeader.MICTagLength();
        VerifyOrReturnError(footerLen <= len, CHIP_ERROR_INTERNAL);

        uint16_t taglen = 0;
        MessageAuthenticationCode mac;
        ReturnErrorOnFailure(mac.Decode(packetHeader, &data[len - footerLen], footerLen, &taglen));

        uint8_t * privacyHeader = packetHeader.PrivacyHeader(msg->Start());
        size_t privacyLength    = packetHeader.PrivacyHeaderLength();
        ReturnErrorOnFailure(cryptoContext.PrivacyEncrypt(privacyHeader, privacyLength, privacyHeader, packetHeader, mac));

        outPacket = std::move(msg);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReceivePacket(const System::PacketBufferHandle & wirePacket)
    {
        System::PacketBufferHandle copy = wirePacket.CloneData();
        VerifyOrReturnError(!copy.IsNull(), CHIP_ERROR_NO_MEMORY);
        mSessionManager.OnMessageReceived(mPeerAddress, std::move(copy));
        return CHIP_NO_ERROR;
    }
};

TEST_F(TestSessionManagerDispatch, TestGroupKeySetCounterIsolation)
{
    auto ctx = std::make_unique<GroupKeySetDispatchContext>();
    ASSERT_SUCCESS(ctx->Init(mContext, *mResources));

    constexpr GroupId kGroupA      = 519;
    constexpr KeysetId kKeySetA    = 535;
    constexpr GroupId kGroupB      = 520;
    constexpr KeysetId kKeySetB    = 540;
    constexpr NodeId kSenderNodeId = 0x00A00001;

    const uint8_t kKeyA[16] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };
    const uint8_t kKeyB[16] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f };

    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kKeySetA, kKeyA));
    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kKeySetB, kKeyB));
    ASSERT_SUCCESS(ctx->MapGroupToKeySet(0, kGroupA, kKeySetA));
    ASSERT_SUCCESS(ctx->MapGroupToKeySet(1, kGroupB, kKeySetB));

    // Sender sends an initial message on Group A (KeySet A).
    System::PacketBufferHandle groupAMsg1;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroupA, kSenderNodeId, 0x0201B0BFu, groupAMsg1));
    ASSERT_SUCCESS(ctx->ReceivePacket(groupAMsg1));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 1u);
    EXPECT_EQ(ctx->mDelegate.mLastGroupId, kGroupA);

    // Same SourceNodeId under KeySet B jumps forward by ~2^30.
    System::PacketBufferHandle groupBMsg;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroupB, kSenderNodeId, 0x4201BC77u, groupBMsg));
    ASSERT_SUCCESS(ctx->ReceivePacket(groupBMsg));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 2u);
    EXPECT_EQ(ctx->mDelegate.mLastGroupId, kGroupB);

    // Subsequent message on Group A (KeySet A) is unaffected by KeySet B's counter.
    System::PacketBufferHandle groupAMsg2;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroupA, kSenderNodeId, 0x0201C82Fu, groupAMsg2));
    ASSERT_SUCCESS(ctx->ReceivePacket(groupAMsg2));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 3u);
    EXPECT_EQ(ctx->mDelegate.mLastGroupId, kGroupA);
    EXPECT_EQ(ctx->mDelegate.mLastCounter, 0x0201C82Fu);
}

TEST_F(TestSessionManagerDispatch, TestGroupKeySetEvictionIsolation)
{
    auto ctx = std::make_unique<GroupKeySetDispatchContext>();
    ASSERT_SUCCESS(ctx->Init(mContext, *mResources));

    constexpr GroupId kGroupA      = 519;
    constexpr KeysetId kKeySetA    = 535;
    constexpr GroupId kGroupB      = 520;
    constexpr KeysetId kKeySetB    = 540;
    constexpr NodeId kSenderNodeId = 0x00A00001;

    const uint8_t kKeyA[16] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };
    const uint8_t kKeyB[16] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f };

    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kKeySetA, kKeyA));
    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kKeySetB, kKeyB));
    ASSERT_SUCCESS(ctx->MapGroupToKeySet(0, kGroupA, kKeySetA));
    ASSERT_SUCCESS(ctx->MapGroupToKeySet(1, kGroupB, kKeySetB));

    // Initial message on Group A (KeySet A) at counter 100.
    System::PacketBufferHandle initialGroupAFrame;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroupA, kSenderNodeId, 100u, initialGroupAFrame));
    ASSERT_SUCCESS(ctx->ReceivePacket(initialGroupAFrame));
    ASSERT_EQ(ctx->mDelegate.mReceivedCount, 1u);

    // Fill KeySet B past CHIP_CONFIG_MAX_GROUP_DATA_PEERS distinct sender NodeIds.
    constexpr uint32_t kChurnPeers = CHIP_CONFIG_MAX_GROUP_DATA_PEERS + 2;
    for (uint32_t i = 0; i < kChurnPeers; i++)
    {
        System::PacketBufferHandle churnFrame;
        ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroupB, 0x00B00000 + i, 1u, churnFrame)) << "peer " << i;
        ASSERT_SUCCESS(ctx->ReceivePacket(churnFrame));
    }
    ASSERT_EQ(ctx->mDelegate.mReceivedCount, 1u + kChurnPeers);

    // Replaying the earlier Group A frame is still rejected as a duplicate.
    ASSERT_SUCCESS(ctx->ReceivePacket(initialGroupAFrame));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 1u + kChurnPeers);

    // Fresh counter 101 from the same sender on Group A succeeds.
    System::PacketBufferHandle freshGroupAFrame;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroupA, kSenderNodeId, 101u, freshGroupAFrame));
    ASSERT_SUCCESS(ctx->ReceivePacket(freshGroupAFrame));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 2u + kChurnPeers);
}

TEST_F(TestSessionManagerDispatch, TestGroupSharedKeySetCounterAndRotation)
{
    auto ctx = std::make_unique<GroupKeySetDispatchContext>();
    ASSERT_SUCCESS(ctx->Init(mContext, *mResources));

    constexpr GroupId kGroup1        = 519;
    constexpr GroupId kGroup2        = 521;
    constexpr KeysetId kSharedKeySet = 535;
    constexpr NodeId kSenderNodeId   = 0x00A00001;

    const uint8_t kEpoch0[16] = { 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f };
    const uint8_t kEpoch1[16] = { 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f };
    const uint8_t kEpoch2[16] = { 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f };
    const uint8_t kDisjoint[16] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f
    };

    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kSharedKeySet, kEpoch0));
    ASSERT_SUCCESS(ctx->MapGroupToKeySet(0, kGroup1, kSharedKeySet));
    ASSERT_SUCCESS(ctx->MapGroupToKeySet(1, kGroup2, kSharedKeySet));

    // Groups sharing kSharedKeySet share per-keyset counter state.
    System::PacketBufferHandle group1Counter200;
    System::PacketBufferHandle group2Counter200;
    System::PacketBufferHandle group1Epoch0Counter202;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup1, kSenderNodeId, 200u, group1Counter200));
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup2, kSenderNodeId, 200u, group2Counter200));
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup1, kSenderNodeId, 202u, group1Epoch0Counter202));

    ASSERT_SUCCESS(ctx->ReceivePacket(group1Counter200));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 1u);

    // Same counter 200 on kGroup2 under the same keyset is rejected, while a fresh counter on kGroup2 is delivered.
    ASSERT_SUCCESS(ctx->ReceivePacket(group2Counter200));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 1u);

    System::PacketBufferHandle group2Counter201;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup2, kSenderNodeId, 201u, group2Counter201));
    ASSERT_SUCCESS(ctx->ReceivePacket(group2Counter201));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 2u);
    EXPECT_EQ(ctx->mDelegate.mLastGroupId, kGroup2);

    // Overlapping epoch-key rotation ([E0] -> [E0, E1, E2]) preserves replay state across old and new epoch keys.
    KeySet rotated(kSharedKeySet, SecurityPolicy::kTrustFirst, 3);
    rotated.epoch_keys[0].start_time = 0;
    memcpy(rotated.epoch_keys[0].key, kEpoch0, sizeof(kEpoch0));
    rotated.epoch_keys[1].start_time = 1000;
    memcpy(rotated.epoch_keys[1].key, kEpoch1, sizeof(kEpoch1));
    rotated.epoch_keys[2].start_time = 2000;
    memcpy(rotated.epoch_keys[2].key, kEpoch2, sizeof(kEpoch2));
    ASSERT_SUCCESS(ctx->mProvider.SetKeySet(GroupKeySetDispatchContext::kFabric, kCompressedFabricId1, rotated));

    // Replay of counter 200 under E0 or E1 is still rejected, while counter 202 under E0 succeeds.
    ASSERT_SUCCESS(ctx->ReceivePacket(group1Counter200));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 2u);

    System::PacketBufferHandle group1Epoch1Counter200;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup1, kSenderNodeId, 200u, group1Epoch1Counter200));
    ASSERT_SUCCESS(ctx->ReceivePacket(group1Epoch1Counter200));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 2u);

    ASSERT_SUCCESS(ctx->ReceivePacket(group1Epoch0Counter202));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 3u);

    // Advance kSharedKeySet by +2^30 under E1, then overwrite kSharedKeySet with disjoint keys;
    // OnKeySetRemoved clears the previous keyset's counter state.
    System::PacketBufferHandle highCounterUnderEpoch1;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup1, kSenderNodeId, 202u + 0x40000000u, highCounterUnderEpoch1));
    ASSERT_SUCCESS(ctx->ReceivePacket(highCounterUnderEpoch1));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 4u);

    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kSharedKeySet, kDisjoint));
    System::PacketBufferHandle freshAfterDisjointReplacement;
    ASSERT_SUCCESS(ctx->BuildEncryptedGroupPacket(kGroup1, kSenderNodeId, 203u, freshAfterDisjointReplacement));
    ASSERT_SUCCESS(ctx->ReceivePacket(freshAfterDisjointReplacement));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 5u);

    // Before RemoveKeySet, replaying counter 203u is rejected as a duplicate.
    ASSERT_SUCCESS(ctx->ReceivePacket(freshAfterDisjointReplacement));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 5u);

    // Explicit RemoveKeySet clears GroupPeerTable counter state via OnKeySetRemoved;
    // re-installing the same keyset ID accepts counter 203u again.
    ASSERT_SUCCESS(ctx->mProvider.RemoveKeySet(GroupKeySetDispatchContext::kFabric, kSharedKeySet));
    ASSERT_SUCCESS(ctx->InstallGroupKeySet(kSharedKeySet, kDisjoint));
    ASSERT_SUCCESS(ctx->mProvider.SetGroupKey(GroupKeySetDispatchContext::kFabric, kGroup1, kSharedKeySet));
    ASSERT_SUCCESS(ctx->ReceivePacket(freshAfterDisjointReplacement));
    EXPECT_EQ(ctx->mDelegate.mReceivedCount, 6u);
}

} // namespace
