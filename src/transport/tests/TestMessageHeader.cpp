/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a process to effect a functional test for
 *      the Message Header class within the transport layer
 *
 */
#include "TestTransportLayer.h"

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/TestUtils.h>
#include <transport/MessageHeader.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestPacketHeaderInitialState(nlTestSuite * inSuite, void * inContext)
{
    PacketHeader header;

    NL_TEST_ASSERT(inSuite, !header.IsSecureSessionControlMsg());
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 0);
    NL_TEST_ASSERT(inSuite, header.GetEncryptionKeyID() == 0);
    NL_TEST_ASSERT(inSuite, !header.GetDestinationNodeId().HasValue());
    NL_TEST_ASSERT(inSuite, !header.GetSourceNodeId().HasValue());
}

void TestPayloadHeaderInitialState(nlTestSuite * inSuite, void * inContext)
{
    PayloadHeader header;

    NL_TEST_ASSERT(inSuite, header.GetMessageType() == 0);
    NL_TEST_ASSERT(inSuite, header.GetExchangeID() == 0);
    NL_TEST_ASSERT(inSuite, header.GetProtocolID() == 0);
    NL_TEST_ASSERT(inSuite, !header.GetVendorId().HasValue());
}

void TestPacketHeaderEncodeDecode(nlTestSuite * inSuite, void * inContext)
{
    PacketHeader header;
    uint8_t buffer[64];
    uint16_t encodeLen;
    uint16_t decodeLen;

    header.SetMessageId(123).SetPayloadLength(16);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);

    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 123);
    NL_TEST_ASSERT(inSuite, !header.GetDestinationNodeId().HasValue());

    header.SetSourceNodeId(55);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);

    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 123);
    NL_TEST_ASSERT(inSuite, !header.GetDestinationNodeId().HasValue());
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(55));

    header.ClearSourceNodeId().SetDestinationNodeId(11);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 123);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(11));
    NL_TEST_ASSERT(inSuite, !header.GetSourceNodeId().HasValue());

    header.SetMessageId(234).SetSourceNodeId(77).SetDestinationNodeId(88);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 234);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(88));
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(77));

    header.SetMessageId(234).SetSourceNodeId(77).SetDestinationNodeId(88).SetSecureSessionControlMsg(true);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 234);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(88));
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(77));
    NL_TEST_ASSERT(inSuite, header.IsSecureSessionControlMsg());

    header.SetMessageId(234).SetSourceNodeId(77).SetDestinationNodeId(88).SetEncryptionKeyID(2);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 234);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(88));
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(77));
    NL_TEST_ASSERT(inSuite, header.GetEncryptionKeyID() == 2);

    header.SetMessageId(234).SetSourceNodeId(77).SetDestinationNodeId(88);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen, Header::Flags::None()) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 234);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(88));
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(77));
}

void TestPayloadHeaderEncodeDecode(nlTestSuite * inSuite, void * inContext)
{
    PayloadHeader header;
    uint8_t buffer[64];
    uint16_t encodeLen;
    uint16_t decodeLen;

    header.SetMessageType(112).SetExchangeID(2233);
    NL_TEST_ASSERT(inSuite, !header.GetVendorId().HasValue());

    header.SetMessageType(112).SetExchangeID(2233).SetProtocolID(1221);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen) == CHIP_NO_ERROR);

    header.SetMessageType(221).SetExchangeID(3322).SetProtocolID(4567);
    NL_TEST_ASSERT(inSuite, header.Decode(Header::Flags::None(), buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageType() == 112);
    NL_TEST_ASSERT(inSuite, header.GetExchangeID() == 2233);
    NL_TEST_ASSERT(inSuite, header.GetProtocolID() == 1221);
    NL_TEST_ASSERT(inSuite, !header.GetVendorId().HasValue());

    header.SetMessageType(112).SetExchangeID(2233).SetProtocolID(1221);
    header.SetVendorId(6789);

    Header::Flags encodeFlags = header.GetEncodePacketFlags();
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen) == CHIP_NO_ERROR);

    header.SetMessageType(111).SetExchangeID(222);

    NL_TEST_ASSERT(inSuite, header.Decode(encodeFlags, buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetExchangeID() == 2233);
    NL_TEST_ASSERT(inSuite, header.GetVendorId() == Optional<uint16_t>::Value(6789));

    header.SetMessageType(221).SetExchangeID(3322).SetProtocolID(4567);
    header.SetVendorId(8976);

    NL_TEST_ASSERT(inSuite, header.Decode(encodeFlags, buffer, sizeof(buffer), &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetExchangeID() == 2233);
    NL_TEST_ASSERT(inSuite, header.GetProtocolID() == 1221);
    NL_TEST_ASSERT(inSuite, header.GetVendorId() == Optional<uint16_t>::Value(6789));
}

void TestPacketHeaderEncodeDecodeBounds(nlTestSuite * inSuite, void * inContext)
{
    PacketHeader header;
    uint8_t buffer[64];
    uint16_t unusedLen;

    for (size_t shortLen = 0; shortLen < 6; shortLen++)
    {
        NL_TEST_ASSERT(inSuite, header.Encode(buffer, shortLen, &unusedLen, Header::Flags::None()) != CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, header.Decode(buffer, shortLen, &unusedLen) != CHIP_NO_ERROR);
    }
}

void TestPayloadHeaderEncodeDecodeBounds(nlTestSuite * inSuite, void * inContext)
{
    PayloadHeader header;
    uint8_t buffer[64];
    uint16_t unusedLen;

    for (size_t shortLen = 0; shortLen < 6; shortLen++)
    {
        NL_TEST_ASSERT(inSuite, header.Encode(buffer, shortLen, &unusedLen) != CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, header.Decode(Header::Flags::None(), buffer, shortLen, &unusedLen) != CHIP_NO_ERROR);
    }
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("PacketInitialState", TestPacketHeaderInitialState),
    NL_TEST_DEF("PayloadInitialState", TestPayloadHeaderInitialState),
    NL_TEST_DEF("PacketEncodeDecode", TestPacketHeaderEncodeDecode),
    NL_TEST_DEF("PayloadEncodeDecode", TestPayloadHeaderEncodeDecode),
    NL_TEST_DEF("PacketEncodeDecodeBounds", TestPacketHeaderEncodeDecodeBounds),
    NL_TEST_DEF("PayloadEncodeDecodeBounds", TestPayloadHeaderEncodeDecodeBounds),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestMessageHeader(void)
{
    nlTestSuite theSuite = { "Transport-MessageHeader", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMessageHeader)
