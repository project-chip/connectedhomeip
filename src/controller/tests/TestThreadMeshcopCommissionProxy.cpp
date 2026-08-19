/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <controller/ThreadMeshcopCommissionProxy.h>

#include <json/json.h>
#include <pw_unit_test/framework.h>

#include <cstdint>
#include <sstream>
#include <vector>

namespace chip {
namespace Controller {
namespace {

constexpr uint8_t kJoinerId[] = { 0x1a, 0x6f, 0x7e, 0xf3, 0x12, 0x66, 0xa6, 0x30 };

constexpr uint8_t kMattercSrvQuery[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // DNS header
    0x08, '_',  'm',  'a',  't',  't',  'e',  'r',  'c',                    // _matterc
    0x04, '_',  'u',  'd',  'p',                                            // _udp
    0x05, 'l',  'o',  'c',  'a',  'l',                                      // local
    0x00, 0x00, 0x21, 0x00, 0x01,                                           // SRV, IN
};

constexpr uint8_t kMalformedMattercSrvResponse[] = {
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, // DNS response header
    0x08, '_',  'm',  'a',  't',  't',  'e',  'r',  'c',                    // _matterc
    0x04, '_',  'u',  'd',  'p',                                            // _udp
    0x05, 'l',  'o',  'c',  'a',  'l',                                      // local
    0x00, 0x00, 0x21, 0x00, 0x01,                                           // SRV, IN
    0x00, 0x00, 0x00, 0x78, 0x00, 0x01, 0x00,                               // TTL, invalid one-byte SRV data
};

constexpr uint8_t kCompleteMattercResponse[] = {
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, // DNS response header
    0x08, '_',  'm',  'a',  't',  't',  'e',  'r',  'c',                    // _matterc
    0x04, '_',  'u',  'd',  'p',                                            // _udp
    0x05, 'l',  'o',  'c',  'a',  'l',                                      // local
    0x00, 0x00, 0x21, 0x00, 0x01,                                           // SRV, IN
    0x00, 0x00, 0x00, 0x78, 0x00, 0x07,                                     // TTL, data length
    0x00, 0x00, 0x00, 0x00, 0x15, 0xa4, 0x00,                               // priority, weight, port, root target
    0x08, '_',  'm',  'a',  't',  't',  'e',  'r',  'c',                    // _matterc
    0x04, '_',  'u',  'd',  'p',                                            // _udp
    0x05, 'l',  'o',  'c',  'a',  'l',                                      // local
    0x00, 0x00, 0x10, 0x00, 0x01,                                           // TXT, IN
    0x00, 0x00, 0x00, 0x78, 0x00, 0x04, 0x03, 'D',  '=',  '0',              // TTL and D=0 TXT entry
};

template <size_t N>
void SendJoinerPacket(ThreadMeshcopCommissionProxy & proxy, const uint8_t (&packet)[N])
{
    proxy.OnJoinerMessage(std::vector<uint8_t>(std::begin(kJoinerId), std::end(kJoinerId)), 49154,
                          std::vector<uint8_t>(std::begin(packet), std::end(packet)));
}

bool LastDiscoveryIsValid(ThreadMeshcopCommissionProxy & proxy)
{
    Json::Value root;
    std::string errors;
    std::istringstream diagnostic(proxy.GetLastDiscoveryDiagnosticJson());
    bool parsed = Json::parseFromStream(Json::CharReaderBuilder(), diagnostic, &root, &errors);

    EXPECT_TRUE(parsed);
    return parsed && root["valid"].asBool();
}

TEST(TestThreadMeshcopCommissionProxy, WaitsForCompleteResponseAfterSrvQuery)
{
    ThreadMeshcopCommissionProxy proxy;

    SendJoinerPacket(proxy, kMattercSrvQuery);
    EXPECT_FALSE(LastDiscoveryIsValid(proxy));

    SendJoinerPacket(proxy, kCompleteMattercResponse);
    EXPECT_TRUE(LastDiscoveryIsValid(proxy));
}

TEST(TestThreadMeshcopCommissionProxy, WaitsForCompleteResponseAfterMalformedSrv)
{
    ThreadMeshcopCommissionProxy proxy;

    SendJoinerPacket(proxy, kMalformedMattercSrvResponse);
    EXPECT_FALSE(LastDiscoveryIsValid(proxy));

    SendJoinerPacket(proxy, kCompleteMattercResponse);
    EXPECT_TRUE(LastDiscoveryIsValid(proxy));
}

TEST(TestThreadMeshcopCommissionProxy, AcceptsCompleteResponse)
{
    ThreadMeshcopCommissionProxy proxy;

    SendJoinerPacket(proxy, kCompleteMattercResponse);

    EXPECT_TRUE(LastDiscoveryIsValid(proxy));
}

} // namespace
} // namespace Controller
} // namespace chip
