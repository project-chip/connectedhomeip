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

#include <support/ErrorStr.h>
#include <transport/MessageHeader.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestHeaderInitialState(nlTestSuite * inSuite, void * inContext)
{
    MessageHeader header;

    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 0);
    NL_TEST_ASSERT(inSuite, !header.GetDestinationNodeId().HasValue());
    NL_TEST_ASSERT(inSuite, !header.GetSourceNodeId().HasValue());
}

void TestHeaderEncodeDecode(nlTestSuite * inSuite, void * inContext)
{
    MessageHeader header;
    uint8_t buffer[64];
    size_t encodeLen;
    size_t decodeLen;

    header.SetMessageId(123);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);

    NL_TEST_ASSERT(inSuite, header.Decode(buffer, encodeLen, &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 123);
    NL_TEST_ASSERT(inSuite, !header.GetDestinationNodeId().HasValue());
    NL_TEST_ASSERT(inSuite, !header.GetSourceNodeId().HasValue());

    header.SetSourceNodeId(55);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);

    NL_TEST_ASSERT(inSuite, header.Decode(buffer, encodeLen, &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 123);
    NL_TEST_ASSERT(inSuite, !header.GetDestinationNodeId().HasValue());
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(55));

    header.ClearSourceNodeId().SetDestinationNodeId(11);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, encodeLen, &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 123);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(11));
    NL_TEST_ASSERT(inSuite, !header.GetSourceNodeId().HasValue());

    header.SetMessageId(234).SetSourceNodeId(77).SetDestinationNodeId(88);
    NL_TEST_ASSERT(inSuite, header.Encode(buffer, sizeof(buffer), &encodeLen) == CHIP_NO_ERROR);

    // change it to verify decoding
    header.SetMessageId(222).SetSourceNodeId(1).SetDestinationNodeId(2);
    NL_TEST_ASSERT(inSuite, header.Decode(buffer, encodeLen, &decodeLen) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, encodeLen == decodeLen);
    NL_TEST_ASSERT(inSuite, header.GetMessageId() == 234);
    NL_TEST_ASSERT(inSuite, header.GetDestinationNodeId() == Optional<uint64_t>::Value(88));
    NL_TEST_ASSERT(inSuite, header.GetSourceNodeId() == Optional<uint64_t>::Value(77));
}

void TestHeaderEncodeDecodeBounds(nlTestSuite * inSuite, void * inContext)
{
    MessageHeader header;
    uint8_t buffer[64];
    size_t unusedLen;

    for (size_t shortLen = 0; shortLen < 6; shortLen++)
    {
        NL_TEST_ASSERT(inSuite, header.Encode(buffer, shortLen, &unusedLen) != CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, header.Decode(buffer, shortLen, &unusedLen) != CHIP_NO_ERROR);
    }
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("InitialState", TestHeaderInitialState),
    NL_TEST_DEF("EncodeDecode", TestHeaderEncodeDecode),
    NL_TEST_DEF("EncodeDecodeBounds", TestHeaderEncodeDecodeBounds),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestMessageHeader(void)
{
    nlTestSuite theSuite = { "Transport-MessageHeader", &sTests[0], NULL, NULL };
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
