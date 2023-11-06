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

#define CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API // Up here in case some other header
                                              // includes SessionManager.h indirectly

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/tests/LoopbackTransportManager.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#undef CHIP_ENABLE_TEST_ENCRYPTED_BUFFER_API

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Test;
using namespace chip::Credentials;

using GroupInfo      = GroupDataProvider::GroupInfo;
using GroupKey       = GroupDataProvider::GroupKey;
using KeySet         = GroupDataProvider::KeySet;
using SecurityPolicy = GroupDataProvider::SecurityPolicy;

using TestContext = chip::Test::LoopbackTransportManager;

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
#if CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2
    // =======================================
    // Test early-SVE2 workaround
    // =======================================
    {
        .name     = "secure group message (no privacy, but invalid P=1 flag)",
        .peerAddr = "::1",

        .payload = "",

        // messageCounter = 0x12345691
        .plain     = "\06\x7d\xdb\x81\x91\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x01\x64\xee\x0e\x20\x7d",
        .encrypted = "\x06\x7d\xdb\x81\x91\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x91\xe0\x22\x85\xe0\x59\x07\xe0"
                     "\xd8\x68\x0c\x79\xac\x6d\x64\x46\x90\x65\xb2\x6f\x90\x26", // Includes MIC
        .privacy   = "\x06\x7d\xdb\x81\x91\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x91\xe0\x22\x85\xe0\x59\x07\xe0"
                     "\xd8\x68\x0c\x79\xac\x6d\x64\x46\x90\x65\xb2\x6f\x90\x26", // Includes MIC

        .payloadLength   = 0,
        .plainLength     = 24,
        .encryptedLength = 40,
        .privacyLength   = 40,

        .encryptKey = "\xca\x92\xd7\xa0\x94\x2d\x1a\x51\x1a\x0e\x26\xad\x07\x4f\x4c\x2f",
        .privacyKey = "\xbf\xe9\xda\x01\x6a\x76\x53\x65\xf2\xdd\x97\xa9\xf9\x39\xe4\x25",
        .epochKey   = "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf",

        .nonce        = "\x01\x91\x56\x34\x12\x01\x00\x00\x00\x00\x00\x00\x00",
        .privacyNonce = "\xdb\x7d\x79\xac\x6d\x64\x46\x90\x65\xb2\x6f\x90\x26",

        .sessionId    = 0xdb7d, // 56189
        .peerNodeId   = 0x0000000000000000ULL,
        .groupId      = 2,
        .sourceNodeId = 0x0000000000000002ULL,

        .expectedMessageCount = 1,
    },
#endif // CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2

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
        NL_TEST_ASSERT(mSuite, header.GetSessionId() == testEntry.sessionId);

        size_t dataLength   = msgBuf->DataLength();
        size_t expectLength = testEntry.payloadLength;

        NL_TEST_ASSERT(mSuite, dataLength == expectLength);
        NL_TEST_ASSERT(mSuite, memcmp(msgBuf->Start(), testEntry.payload, dataLength) == 0);

        ChipLogProgress(Test, "::: TestSessionManagerDispatch[%d] PASS", mTestVectorIndex);
    }

    void ResetTest(unsigned testVectorIndex)
    {
        mTestVectorIndex = testVectorIndex;
        mReceivedCount   = 0;
    }

    unsigned NumMessagesReceived() { return mReceivedCount; }

    nlTestSuite * mSuite      = nullptr;
    unsigned mTestVectorIndex = 0;
    unsigned mReceivedCount   = 0;
};

PeerAddress AddressFromString(const char * str)
{
    Inet::IPAddress addr;

    VerifyOrDie(Inet::IPAddress::FromString(str, addr));

    return PeerAddress::UDP(addr);
}

void TestSessionManagerInit(nlTestSuite * inSuite, TestContext & ctx, SessionManager & sessionManager)
{
    static FabricTableHolder fabricTableHolder;
    static secure_channel::MessageCounterManager gMessageCounterManager;
    static chip::TestPersistentStorageDelegate deviceStorage;
    static chip::Crypto::DefaultSessionKeystore sessionKeystore;

    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == fabricTableHolder.Init());
    NL_TEST_ASSERT(inSuite,
                   CHIP_NO_ERROR ==
                       sessionManager.Init(&ctx.GetSystemLayer(), &ctx.GetTransportMgr(), &gMessageCounterManager, &deviceStorage,
                                           &fabricTableHolder.GetFabricTable(), sessionKeystore));
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

void TestSessionManagerDispatch(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    SessionManager sessionManager;
    TestSessionManagerCallback callback;

    TestSessionManagerInit(inSuite, ctx, sessionManager);
    sessionManager.SetMessageDelegate(&callback);

    IPAddress addr;
    IPAddress::FromString("::1", addr);
    Transport::PeerAddress peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SessionHolder aliceToBobSession;
    SessionHolder testGroupSession;

    callback.mSuite = inSuite;
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = InjectGroupSessionWithTestKey(testGroupSession, testEntry);
        NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == err);

        const char * plain = testEntry.plain;
        const ByteSpan expectedPlain(reinterpret_cast<const uint8_t *>(plain), testEntry.plainLength);
        const char * privacy = testEntry.privacy;
        chip::System::PacketBufferHandle msg =
            chip::MessagePacketBuffer::NewWithData(reinterpret_cast<const uint8_t *>(privacy), testEntry.privacyLength);

        const PeerAddress peerAddress = AddressFromString(testEntry.peerAddr);
        sessionManager.OnMessageReceived(peerAddress, std::move(msg));
        NL_TEST_ASSERT(inSuite, callback.NumMessagesReceived() == testEntry.expectedMessageCount);

        if ((testEntry.expectedMessageCount == 0) && (callback.NumMessagesReceived() == 0))
        {
            ChipLogProgress(Test, "::: TestSessionManagerDispatch[%d] PASS (negative test case)", i);
        }
    }

    sessionManager.Shutdown();
}

// ============================================================================
//              Test Suite Instrumenation
// ============================================================================

/**
 *  Initialize the test suite.
 */
int Initialize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return SUCCESS;
}

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Session Manager Dispatch",  TestSessionManagerDispatch),

    NL_TEST_SENTINEL()
};

nlTestSuite sSuite =
{
    "TestSessionManagerDispatch",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

} // namespace

/**
 *  Main
 */
int TestSessionManagerDispatchSuite()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSessionManagerDispatchSuite);
