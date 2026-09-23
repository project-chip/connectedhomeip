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

/**
 *    @file
 *      Seeded FuzzTest harness for the BtpEngine transport-level reassembler.
 *      Uses real BTP fragment shapes as seeds so the mutator starts from valid
 *      header-flag combinations and explores the reorder-queue / ack-handling
 *      branches instead of bouncing off the header-validation gate.
 */

#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <ble/Ble.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>

namespace {

using chip::Ble::BtpEngine;
using chip::Ble::SequenceNumber_t;
using chip::System::PacketBufferHandle;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Structure-aware BTP fragment generation.
//
// HandleCharacteristicReceived gates every fragment on `seq == mRxNextSeqNum`
// (a running counter starting at 0, or 1 when expect_first_ack). A raw-byte
// mutator essentially never reproduces the correct sequence number, so it
// bounces off the seq gate at the second fragment and the reassembly state
// machine is never reached. Instead the fuzzer picks per-fragment header flags
// / declared length / payload, and the harness stamps the CORRECT running
// sequence number, so the mutator explores the reassembler (Idle->InProgress->
// End, AddToEnd/CompactHead, length trim) rather than the seq-reject path.
//
// The ack bit (kFragmentAck=0x08) is intentionally excluded: the harness never
// transmits, so the TX window is empty and any ack-bearing fragment is rejected
// by HandleAckReceived before the sequence number is consumed (which would also
// desync this harness's seq mirror). Ack/TX-path fuzzing needs a separate
// harness that drives sends first.

// Per-fragment fuzzed fields: (headerFlags, declaredMsgLen, payload).
using FragSpec = std::tuple<uint8_t, uint16_t, std::vector<uint8_t>>;

// Seeds: valid multi-fragment messages to bootstrap the corpus.
std::vector<std::vector<FragSpec>> BtpSeedSequences()
{
    return {
        { FragSpec{ 0x05, 2, { 0xDE, 0xAD } } },
        { FragSpec{ 0x01, 6, { 0x01, 0x02 } }, FragSpec{ 0x02, 0, { 0x03, 0x04 } }, FragSpec{ 0x04, 0, { 0x05, 0x06 } } },
        { FragSpec{ 0x01, 4, { 0xAA, 0xBB } }, FragSpec{ 0x04, 0, { 0xCC, 0xDD } } },
    };
}

// Property: feeding a sequence of correctly-sequenced BTP data fragments must
// never crash the reassembler.
void BtpEngineDoesNotCrash(bool expectFirstAck, const std::vector<FragSpec> & frags)
{
    EnsureInitialized();

    BtpEngine engine;
    if (engine.Init(nullptr, expectFirstAck) != CHIP_NO_ERROR)
    {
        return;
    }

    // Mirror the engine's expected-rx counter (uint8 wrap); matches BtpEngine::Init,
    // which sets mRxNextSeqNum = 0 when expect_first_ack, else 1. Every fragment we
    // build carries the header+seq bytes, so the engine consumes the seq and
    // increments in lockstep.
    SequenceNumber_t seq = expectFirstAck ? 0 : 1;

    for (const auto & f : frags)
    {
        const uint8_t flags                  = std::get<0>(f);
        const uint16_t declaredLen           = std::get<1>(f);
        const std::vector<uint8_t> & payload = std::get<2>(f);

        std::vector<uint8_t> frame;
        frame.push_back(flags);
        frame.push_back(seq); // correct running sequence number
        if (flags & 0x01)     // kStartMessage carries a 16-bit total length
        {
            frame.push_back(static_cast<uint8_t>(declaredLen));
            frame.push_back(static_cast<uint8_t>(declaredLen >> 8));
        }
        frame.insert(frame.end(), payload.begin(), payload.end());

        auto buf = PacketBufferHandle::NewWithData(frame.data(), frame.size());
        if (buf.IsNull())
            continue;

        SequenceNumber_t receivedAck = 0;
        bool didReceiveAck           = false;
        RETURN_SAFELY_IGNORED engine.HandleCharacteristicReceived(std::move(buf), receivedAck, didReceiveAck);

        seq = static_cast<SequenceNumber_t>(seq + 1);

        if (engine.RxState() == BtpEngine::kState_Error)
            break;
    }
}

FUZZ_TEST(FuzzBtpEnginePW, BtpEngineDoesNotCrash)
    .WithDomains(Arbitrary<bool>(),
                 // Per-fragment: (headerFlags from the data-fragment set, declaredLen, payload).
                 VectorOf(TupleOf(ElementOf<uint8_t>({ 0x01, 0x02, 0x04, 0x05, 0x06, 0x03, 0x07, 0x00 }), Arbitrary<uint16_t>(),
                                  Arbitrary<std::vector<uint8_t>>()))
                     .WithMaxSize(10)
                     .WithSeeds(BtpSeedSequences()));

} // namespace
