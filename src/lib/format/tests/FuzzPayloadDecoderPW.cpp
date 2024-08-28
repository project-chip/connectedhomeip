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

void RunDecodePW(const std::vector<std::uint8_t> & bytes, chip::Protocols::Id mProtocol, uint8_t mMessageType)
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

    // TODO: remove
    // PRINT BYTES: To check the combination of bytes being printed
    //  std::cout << "bytes: ";
    //  for (const auto& byte : bytes) {
    //      std::cout << static_cast<int>(byte) << " ";
    //  }
    //  std::cout <<std::endl << std::endl;

    // printing Protocol IDs
    // std::cout << "protocol ID: " << mProtocol.GetProtocolId()<<std::endl;

    // printing mMessageType
    // std::cout << "mMessageType: " << mMessageType << std::endl;
}

// This allows us to fuzz test with all the combinations of protocols
auto ProtocolIDs()
{
    return ElementOf({ chip::Protocols::SecureChannel::Id, chip::Protocols::InteractionModel::Id, chip::Protocols::BDX::Id,
                       chip::Protocols::UserDirectedCommissioning::Id });
}

FUZZ_TEST(PayloadDecoder, RunDecodePW).WithDomains(Arbitrary<std::vector<std::uint8_t>>(), ProtocolIDs(), Arbitrary<uint8_t>());

} // namespace
