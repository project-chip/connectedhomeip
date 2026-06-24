/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// FuzzTest harness for the WiFiPAFTP transport-protocol engine, seeded with real PAFTP fragment
// shapes so the mutator starts past the header-validation gate.

#include <cstdint>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>
#include <wifipaf/WiFiPAFTP.h>

namespace {

using chip::System::PacketBufferHandle;
using chip::WiFiPAF::SequenceNumber_t;
using chip::WiFiPAF::WiFiPAFTP;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Real PAFTP fragment shapes (capability + data-phase variants).
std::vector<std::vector<uint8_t>> PaftpFragmentSeeds()
{
    return {
        { 0x65, 0x6c, 0x04, 0x00, 0x00, 0x00, 0x5e, 0x01, 0x06 }, // capability request
        { 0x65, 0x6c, 0x04, 0x5b, 0x01, 0x06 },                   // capability response
        { 0x05, 0x01, 0x01, 0x00, 0x00, 0xAA },                   // start+end, seq 1
        { 0x01, 0x00, 0x01, 0x00, 0x00, 0xDE, 0xAD },             // start only
        { 0x02, 0x02, 0xBE, 0xEF },                               // continue
        { 0x04, 0x03, 0xCA, 0xFE },                               // end
        { 0x08, 0x02 },                                           // ack-only
        { 0x05, 0x01, 0x05, 0x00, 0x00, 0xDE, 0xAD, 0xBE, 0xEF }, // out-of-order seq 5
    };
}

void WiFiPAFTPDoesNotCrash(bool expectFirstAck, const std::vector<uint8_t> & frag0, const std::vector<uint8_t> & frag1,
                           const std::vector<uint8_t> & frag2, const std::vector<uint8_t> & frag3)
{
    EnsureInitialized();

    WiFiPAFTP engine;
    if (engine.Init(nullptr, expectFirstAck) != CHIP_NO_ERROR)
    {
        return;
    }

    for (const auto * frag : { &frag0, &frag1, &frag2, &frag3 })
    {
        auto buf = PacketBufferHandle::NewWithData(frag->data(), frag->size());
        if (buf.IsNull())
        {
            continue;
        }

        SequenceNumber_t receivedAck = 0;
        bool didReceiveAck           = false;
        // Fuzzer asserts only the no-crash property; the parse status is irrelevant here.
        RETURN_SAFELY_IGNORED engine.HandleCharacteristicReceived(std::move(buf), receivedAck, didReceiveAck);

        if (engine.RxState() == WiFiPAFTP::kState_Error)
        {
            break;
        }
    }
}

FUZZ_TEST(FuzzWiFiPAFTPPW, WiFiPAFTPDoesNotCrash)
    .WithDomains(Arbitrary<bool>(), Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpFragmentSeeds()),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpFragmentSeeds()),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpFragmentSeeds()),
                 Arbitrary<std::vector<uint8_t>>().WithSeeds(PaftpFragmentSeeds()));

} // namespace
