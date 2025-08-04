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
#include <cstddef>
#include <cstdint>
#include <lib/format/protocol_decoder.h>
#include <lib/support/StringBuilder.h>

#include <tlv/meta/clusters_meta.h>
#include <tlv/meta/protocols_meta.h>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip::Decoders;
using namespace chip::FlatTree;
using namespace chip::TLV;
using namespace chip::TLVMeta;
using namespace fuzztest;

// The Property Function; The FUZZ_TEST macro will call this function, with the fuzzed input domains
void RunDecodeFuzz(const std::vector<std::uint8_t> & bytes, chip::Protocols::Id mProtocol, uint8_t mMessageType)
{

    PayloadDecoderInitParams params;
    params.SetProtocolDecodeTree(chip::TLVMeta::protocols_meta).SetClusterDecodeTree(chip::TLVMeta::clusters_meta);

    // Fuzzing with different Protocols
    params.SetProtocol(mProtocol);

    // Fuzzing with different MessageTypes
    params.SetMessageType(mMessageType);
    chip::Decoders::PayloadDecoder<64, 128> decoder(params);

    chip::ByteSpan payload(bytes.data(), bytes.size());

    decoder.StartDecoding(payload);

    PayloadEntry entry;
    while (decoder.Next(entry))
    {
        // Nothing to do ...
    }
}

// This function allows us to fuzz using all existing protocols; by using FuzzTests's `ElementOf` API, we define an
// input domain by explicitly enumerating the set of values in it More Info:
// https://github.com/google/fuzztest/blob/main/doc/domains-reference.md#elementof-domains-element-of
auto AnyProtocolID()
{
    // Adding an Invalid Protocol
    static constexpr chip::Protocols::Id InvalidProtocolID(chip::VendorId::Common, 2121);

    return ElementOf({ chip::Protocols::SecureChannel::Id, chip::Protocols::InteractionModel::Id, chip::Protocols::BDX::Id,
                       chip::Protocols::UserDirectedCommissioning::Id, chip::Protocols::Echo::Id, chip::Protocols::NotSpecified,
                       InvalidProtocolID });
}

FUZZ_TEST(PayloadDecoder, RunDecodeFuzz).WithDomains(Arbitrary<std::vector<std::uint8_t>>(), AnyProtocolID(), Arbitrary<uint8_t>());

} // namespace
