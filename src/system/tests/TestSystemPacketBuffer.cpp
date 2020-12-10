/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file implements a unit test suite for
 *      <tt>chip::System::PacketBuffer</tt>, a class that provides
 *      structure for network packet buffer management.
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <nlunit-test.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if (LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
#define PBUF_TYPE(pbuf) (pbuf)->type_internal
#else
#define PBUF_TYPE(pbuf) (pbuf)->type
#endif // (LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 1)
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

using ::chip::System::PacketBuffer;
using ::chip::System::PacketBufferHandle;

#if !CHIP_SYSTEM_CONFIG_USE_LWIP
using ::chip::System::pbuf;
#endif

// Utility functions.

#define TO_LWIP_PBUF(x) (reinterpret_cast<struct pbuf *>(reinterpret_cast<void *>(x)))
#define OF_LWIP_PBUF(x) (reinterpret_cast<PacketBuffer *>(reinterpret_cast<void *>(x)))

class PacketBufferTest
{
public:
    static void CheckNewWithAvailableSizeAndFree(nlTestSuite * inSuite, void * inContext);
    static void CheckStart(nlTestSuite * inSuite, void * inContext);
    static void CheckSetStart(nlTestSuite * inSuite, void * inContext);
    static void CheckDataLength(nlTestSuite * inSuite, void * inContext);
    static void CheckSetDataLength(nlTestSuite * inSuite, void * inContext);
    static void CheckTotalLength(nlTestSuite * inSuite, void * inContext);
    static void CheckMaxDataLength(nlTestSuite * inSuite, void * inContext);
    static void CheckAvailableDataLength(nlTestSuite * inSuite, void * inContext);
    static void CheckReservedSize(nlTestSuite * inSuite, void * inContext);
    static void CheckAddToEnd(nlTestSuite * inSuite, void * inContext);
    static void CheckDetachTail(nlTestSuite * inSuite, void * inContext);
    static void CheckCompactHead(nlTestSuite * inSuite, void * inContext);
    static void CheckConsumeHead(nlTestSuite * inSuite, void * inContext);
    static void CheckConsume(nlTestSuite * inSuite, void * inContext);
    static void CheckEnsureReservedSize(nlTestSuite * inSuite, void * inContext);
    static void CheckAlignPayload(nlTestSuite * inSuite, void * inContext);
    static void CheckNext(nlTestSuite * inSuite, void * inContext);
    static void CheckAddRef(nlTestSuite * inSuite, void * inContext);
    static void CheckFree(nlTestSuite * inSuite, void * inContext);
    static void CheckFreeHead(nlTestSuite * inSuite, void * inContext);
    static void CheckBuildFreeList(nlTestSuite * inSuite, void * inContext);

    static void BufferFree(struct TestContext * theContext);
};

namespace {

void CheckNewWithAvailableSizeAndFree(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckNewWithAvailableSizeAndFree(inSuite, inContext);
}
void CheckStart(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckStart(inSuite, inContext);
}
void CheckSetStart(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckSetStart(inSuite, inContext);
}
void CheckDataLength(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckDataLength(inSuite, inContext);
}
void CheckSetDataLength(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckSetDataLength(inSuite, inContext);
}
void CheckTotalLength(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckTotalLength(inSuite, inContext);
}
void CheckMaxDataLength(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckMaxDataLength(inSuite, inContext);
}
void CheckAvailableDataLength(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckAvailableDataLength(inSuite, inContext);
}
void CheckReservedSize(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckReservedSize(inSuite, inContext);
}
void CheckAddToEnd(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckAddToEnd(inSuite, inContext);
}
void CheckDetachTail(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckDetachTail(inSuite, inContext);
}
void CheckCompactHead(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckCompactHead(inSuite, inContext);
}
void CheckConsumeHead(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckConsumeHead(inSuite, inContext);
}
void CheckConsume(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckConsume(inSuite, inContext);
}
void CheckEnsureReservedSize(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckEnsureReservedSize(inSuite, inContext);
}
void CheckAlignPayload(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckAlignPayload(inSuite, inContext);
}
void CheckNext(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckNext(inSuite, inContext);
}
void CheckAddRef(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckAddRef(inSuite, inContext);
}
void CheckFree(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckFree(inSuite, inContext);
}
void CheckFreeHead(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckFreeHead(inSuite, inContext);
}
void CheckBuildFreeList(nlTestSuite * inSuite, void * inContext)
{
    PacketBufferTest::CheckBuildFreeList(inSuite, inContext);
}

} // namespace

// Test input vector format.

struct TestContext
{
    uint16_t init_len;
    uint16_t reserved_size;
    uint8_t * start_buffer;
    uint8_t * end_buffer;
    uint8_t * payload_ptr;
    struct pbuf * buf;
};

// Test input data.

// clang-format off
struct TestContext sContext[] =
{
      { 0,      0,                               nullptr, nullptr, nullptr, nullptr },
      { 0,      10,                              nullptr, nullptr, nullptr, nullptr },
      { 0,      128,                             nullptr, nullptr, nullptr, nullptr },
      { 0,      1536,                            nullptr, nullptr, nullptr, nullptr },
      { 0,      CHIP_SYSTEM_PACKETBUFFER_SIZE,   nullptr, nullptr, nullptr, nullptr }
};
// clang-format on

const uint16_t sLengths[] = { 0, 1, 10, 128, CHIP_SYSTEM_PACKETBUFFER_SIZE, UINT16_MAX };

// Number of test context examples.
const size_t kTestElements = sizeof(sContext) / sizeof(struct TestContext);
const size_t kTestLengths  = sizeof(sLengths) / sizeof(uint16_t);

/**
 *  Free allocated test buffer memory.
 */
void PacketBufferTest::BufferFree(struct TestContext * theContext)
{
    if (theContext->buf != nullptr)
    {
        PacketBuffer::Free(OF_LWIP_PBUF(theContext->buf));
        theContext->buf = nullptr;
    }
}

/**
 *  Allocate memory for a test buffer and configure according to test context.
 */
void BufferAlloc(struct TestContext * theContext)
{
    const size_t lInitialSize = CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE + theContext->reserved_size;
    const size_t lAllocSize   = CHIP_SYSTEM_PACKETBUFFER_SIZE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    u8_t lType, lFlags;
#if LWIP_PBUF_FROM_CUSTOM_POOLS
    u16_t lPool;
#endif // LWIP_PBUF_FROM_CUSTOM_POOLS
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    if (theContext->buf == nullptr)
    {
        theContext->buf = TO_LWIP_PBUF(PacketBuffer::New(0).Release_ForNow());
    }

    if (theContext->buf == nullptr)
    {
        fprintf(stderr, "Failed to allocate %zuB memory: %s\n", lAllocSize, strerror(errno));
        exit(EXIT_FAILURE);
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    lType  = PBUF_TYPE(theContext->buf);
    lFlags = theContext->buf->flags;
#if LWIP_PBUF_FROM_CUSTOM_POOLS
    lPool = theContext->buf->pool;
#endif // LWIP_PBUF_FROM_CUSTOM_POOLS
    memset(theContext->buf, 0, lAllocSize);
    PBUF_TYPE(theContext->buf) = lType;
    theContext->buf->flags     = lFlags;
#if LWIP_PBUF_FROM_CUSTOM_POOLS
    theContext->buf->pool = lPool;
#endif // LWIP_PBUF_FROM_CUSTOM_POOLS
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
    memset(theContext->buf, 0, lAllocSize);
#if CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
    theContext->buf->alloc_size = lAllocSize;
#endif // CHIP_SYSTEM_CONFIG_PACKETBUFFER_MAXALLOC == 0
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    theContext->start_buffer = reinterpret_cast<uint8_t *>(theContext->buf);
    theContext->end_buffer   = reinterpret_cast<uint8_t *>(theContext->buf) + lAllocSize;

    if (lInitialSize > lAllocSize)
    {
        theContext->payload_ptr = theContext->end_buffer;
    }
    else
    {
        theContext->payload_ptr = theContext->start_buffer + lInitialSize;
    }
}

/**
 *  Setup buffer layout as it is used by PacketBuffer class.
 */
PacketBuffer * PrepareTestBuffer(struct TestContext * theContext)
{
    BufferAlloc(theContext);

    theContext->buf->next    = nullptr;
    theContext->buf->payload = theContext->payload_ptr;
    theContext->buf->ref     = 1;
    theContext->buf->len     = theContext->init_len;
    theContext->buf->tot_len = theContext->init_len;

    return reinterpret_cast<PacketBuffer *>(theContext->buf);
}
// Test functions invoked from the suite.

/**
 *  Test PacketBuffer::Start() function.
 */
void PacketBufferTest::CheckStart(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->Start() == theContext->payload_ptr);

        theContext++;
    }
}

/**
 *  Test PacketBuffer::SetStart() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               for any offset value from start_offset[], pass it to the
 *               buffer's instance through SetStart method. Then, verify that
 *               the beginning of the buffer has been correctly internally
 *               adjusted according to the offset value passed into the
 *               SetStart() method.
 */
void PacketBufferTest::CheckSetStart(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext          = static_cast<struct TestContext *>(inContext);
    static const ptrdiff_t sSizePacketBuffer = CHIP_SYSTEM_PACKETBUFFER_SIZE;

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        // clang-format off
        static const ptrdiff_t start_offset[] =
        {
            -sSizePacketBuffer,
            -128,
            -1,
            0,
            1,
            128,
            sSizePacketBuffer
        };
        // clang-format on

        for (ptrdiff_t offset : start_offset)
        {
            PacketBuffer * buffer  = PrepareTestBuffer(theContext);
            uint8_t * test_start   = theContext->payload_ptr + offset;
            uint8_t * verify_start = test_start;

            buffer->SetStart(test_start);

            if (verify_start < theContext->start_buffer + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE)
            {
                // Set start before valid payload beginning.
                verify_start = theContext->start_buffer + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE;
            }

            if (verify_start > theContext->end_buffer)
            {
                // Set start after valid payload beginning.
                verify_start = theContext->end_buffer;
            }

            NL_TEST_ASSERT(inSuite, theContext->buf->payload == verify_start);

            if ((verify_start - theContext->payload_ptr) > theContext->init_len)
            {
                // Set start to the beginning of payload, right after buffer's header.
                NL_TEST_ASSERT(inSuite, theContext->buf->len == 0);
            }
            else
            {
                // Set start to somewhere between the end of the buffer's
                // header and the end of payload.
                NL_TEST_ASSERT(inSuite, theContext->buf->len == (theContext->init_len - (verify_start - theContext->payload_ptr)));
            }
        }
        theContext++;
    }
}

/**
 *  Test PacketBuffer::DataLength() function.
 */
void PacketBufferTest::CheckDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->DataLength() == theContext->buf->len);

        theContext++;
    }
}

/**
 *  Test PacketBuffer::SetDataLength() function.
 *
 *  Description: Take two initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. For any two buffers, call SetDataLength with
 *               different value from sLength[]. If two buffers are created with
 *               the same configuration, test SetDataLength on one buffer,
 *               without specifying the head of the buffer chain. Otherwise,
 *               test SetDataLength with one buffer being down the chain and the
 *               other one being passed as the head of the chain. After calling
 *               the method verify that data lenghts were correctly adjusted.
 */
void PacketBufferTest::CheckSetDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            for (uint16_t length : sLengths)
            {
                PacketBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
                PacketBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);

                if (theFirstContext == theSecondContext)
                {
                    // headOfChain (the second arg) is NULL
                    buffer_2->SetDataLength(length, nullptr);

                    if (length > (theSecondContext->end_buffer - theSecondContext->payload_ptr))
                    {
                        NL_TEST_ASSERT(
                            inSuite, theSecondContext->buf->len == (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite,
                                       theSecondContext->buf->tot_len ==
                                           (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == nullptr);
                    }
                    else
                    {
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->len == length);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->tot_len == length);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == nullptr);
                    }
                }
                else
                {
                    // headOfChain (the second arg) is buffer_1
                    buffer_2->SetDataLength(length, buffer_1);

                    if (length > (theSecondContext->end_buffer - theSecondContext->payload_ptr))
                    {
                        NL_TEST_ASSERT(
                            inSuite, theSecondContext->buf->len == (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite,
                                       theSecondContext->buf->tot_len ==
                                           (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == nullptr);

                        NL_TEST_ASSERT(inSuite,
                                       theFirstContext->buf->tot_len ==
                                           (theFirstContext->init_len +
                                            static_cast<int32_t>(theSecondContext->end_buffer - theSecondContext->payload_ptr) -
                                            static_cast<int32_t>(theSecondContext->init_len)));
                    }
                    else
                    {
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->len == length);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->tot_len == length);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == nullptr);

                        NL_TEST_ASSERT(inSuite,
                                       theFirstContext->buf->tot_len ==
                                           (theFirstContext->init_len + static_cast<int32_t>(length) -
                                            static_cast<int32_t>(theSecondContext->init_len)));
                    }
                }
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::TotalLength() function.
 */
void PacketBufferTest::CheckTotalLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->TotalLength() == theContext->init_len);

        theContext++;
    }
}

/**
 *  Test PacketBuffer::MaxDataLength() function.
 */
void PacketBufferTest::CheckMaxDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->MaxDataLength() == (theContext->end_buffer - theContext->payload_ptr));

        theContext++;
    }
}

/**
 *  Test PacketBuffer::AvailableDataLength() function.
 */
void PacketBufferTest::CheckAvailableDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(
            inSuite, buffer->AvailableDataLength() == ((theContext->end_buffer - theContext->payload_ptr) - theContext->init_len));

        theContext++;
    }
}

/**
 *  Test PacketBuffer::ReservedSize() function.
 */
void PacketBufferTest::CheckReservedSize(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer & lBuffer  = *PrepareTestBuffer(theContext);
        const size_t kAllocSize = lBuffer.AllocSize();

        if (theContext->reserved_size > kAllocSize)
        {
            NL_TEST_ASSERT(inSuite, lBuffer.ReservedSize() == kAllocSize);
        }
        else
        {
            NL_TEST_ASSERT(inSuite, lBuffer.ReservedSize() == theContext->reserved_size);
        }

        theContext++;
    }
}

/**
 *  Test PacketBuffer::AddToEnd() function.
 *
 *  Description: Take three initial configurations of PacketBuffer from
 *               inContext, create three PacketBuffers based on those
 *               configurations and then link those buffers together with
 *               PacketBuffer:AddToEnd(). Then, assert that after connecting
 *               buffers together, their internal states are correctly updated.
 *               This test function tests linking any combination of three
 *               buffer-configurations passed within inContext.
 */
void PacketBufferTest::CheckAddToEnd(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            struct TestContext * theThirdContext = static_cast<struct TestContext *>(inContext);

            for (size_t kth = 0; kth < kTestElements; kth++)
            {
                PacketBuffer * buffer_1 = nullptr;
                PacketBuffer * buffer_2 = nullptr;
                PacketBuffer * buffer_3 = nullptr;

                if (theFirstContext == theSecondContext || theFirstContext == theThirdContext ||
                    theSecondContext == theThirdContext)
                {
                    theThirdContext++;
                    continue;
                }

                buffer_1 = PrepareTestBuffer(theFirstContext);
                buffer_2 = PrepareTestBuffer(theSecondContext);
                buffer_3 = PrepareTestBuffer(theThirdContext);

                buffer_1->AddToEnd_ForNow(buffer_2);

                NL_TEST_ASSERT(inSuite, theFirstContext->buf->tot_len == (theFirstContext->init_len + theSecondContext->init_len));
                NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == theSecondContext->buf);
                NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == nullptr);

                NL_TEST_ASSERT(inSuite, theThirdContext->buf->next == nullptr);

                buffer_1->AddToEnd_ForNow(buffer_3);

                NL_TEST_ASSERT(inSuite,
                               theFirstContext->buf->tot_len ==
                                   (theFirstContext->init_len + theSecondContext->init_len + theThirdContext->init_len));
                NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == theSecondContext->buf);
                NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == theThirdContext->buf);
                NL_TEST_ASSERT(inSuite, theThirdContext->buf->next == nullptr);

                theThirdContext++;
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::DetachTail() function.
 *
 *  Description: Take two initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, link those buffers together, with the first
 *               buffer instance pointing to the second one. Then, call DetachTail()
 *               on the first buffer to unlink the second buffer. After the call,
 *               verify correct internal state of the first buffer.
 */
void PacketBufferTest::CheckDetachTail(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            PacketBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
            PacketBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);
            PacketBuffer * returned = nullptr;
            buffer_1->AddRef(); // The test still holds ownership via buffer_1.
            PacketBufferHandle buffer_handle = PacketBufferHandle::Adopt(buffer_1);

            if (theFirstContext != theSecondContext)
            {
                theFirstContext->buf->next    = theSecondContext->buf;
                theFirstContext->buf->tot_len = static_cast<uint16_t>(theFirstContext->buf->tot_len + theSecondContext->init_len);
            }

            PacketBufferHandle popped = buffer_handle.PopHead();
            returned                  = buffer_handle.Release_ForNow();

            NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == nullptr);
            NL_TEST_ASSERT(inSuite, theFirstContext->buf->tot_len == theFirstContext->init_len);

            if (theFirstContext != theSecondContext)
            {
                NL_TEST_ASSERT(inSuite, returned == buffer_2);
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::CompactHead() function.
 *
 *  Description: Take two initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, set both buffers' data length to any
 *               combination of values from sLengths[] and link those buffers
 *               into a chain. Then, call CompactHead() on the first buffer in
 *               the chain. After calling the method, verify correctly adjusted
 *               state of the first buffer.
 */
void PacketBufferTest::CheckCompactHead(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            // start with various initial length for the first buffer
            for (uint16_t firstLength : sLengths)
            {
                // start with various initial length for the second buffer
                for (uint16_t secondLength : sLengths)
                {
                    PacketBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
                    PacketBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);
                    uint16_t len1           = 0;
                    uint16_t len2           = 0;

                    buffer_1->SetDataLength(firstLength, buffer_1);
                    len1 = buffer_1->DataLength();

                    if (theFirstContext != theSecondContext)
                    {
                        theFirstContext->buf->next = theSecondContext->buf;

                        // Add various lengths to the second buffer
                        buffer_2->SetDataLength(secondLength, buffer_1);
                        len2 = buffer_2->DataLength();
                    }

                    buffer_1->CompactHead();

                    NL_TEST_ASSERT(inSuite,
                                   theFirstContext->buf->payload ==
                                       (theFirstContext->start_buffer + CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE));

                    /* verify length of the first buffer */
                    if (theFirstContext == theSecondContext)
                    {
                        NL_TEST_ASSERT(inSuite, theFirstContext->buf->tot_len == len1);
                    }
                    else if (theFirstContext->buf->tot_len > buffer_1->MaxDataLength())
                    {
                        NL_TEST_ASSERT(inSuite, theFirstContext->buf->len == buffer_1->MaxDataLength());
                        NL_TEST_ASSERT(inSuite,
                                       theSecondContext->buf->len == theFirstContext->buf->tot_len - buffer_1->MaxDataLength());
                    }
                    else
                    {
                        NL_TEST_ASSERT(inSuite, theFirstContext->buf->len == theFirstContext->buf->tot_len);
                        if (len1 >= buffer_1->MaxDataLength() && len2 == 0)
                        {
                            /* make sure the second buffer is not freed */
                            NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == theSecondContext->buf);
                        }
                        else
                        {
                            /* make sure the second buffer is freed */
                            NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == nullptr);
                            theSecondContext->buf = nullptr;
                        }
                    }
                }
            }
            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::ConsumeHead() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               for any value from sLengths[], pass it to the buffer's
 *               instance through ConsumeHead() method. Then, verify that
 *               the internal state of the buffer has been correctly
 *               adjusted according to the value passed into the method.
 */
void PacketBufferTest::CheckConsumeHead(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        for (uint16_t length : sLengths)
        {
            PacketBuffer * buffer = PrepareTestBuffer(theContext);

            buffer->ConsumeHead(length);

            if (length > theContext->init_len)
            {
                NL_TEST_ASSERT(inSuite, theContext->buf->payload == (theContext->payload_ptr + theContext->init_len));
                NL_TEST_ASSERT(inSuite, theContext->buf->len == 0);
                NL_TEST_ASSERT(inSuite, theContext->buf->tot_len == 0);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, theContext->buf->payload == (theContext->payload_ptr + length));
                NL_TEST_ASSERT(inSuite, theContext->buf->len == (theContext->buf->len - length));
                NL_TEST_ASSERT(inSuite, theContext->buf->tot_len == (theContext->buf->tot_len - length));
            }

            if (theContext->buf->ref == 0)
            {
                theContext->buf = nullptr;
            }
        }

        theContext++;
    }
}

/**
 *  Test PacketBuffer::Consume() function.
 *
 *  Description: Take two different initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, set both buffers' data length to any
 *               combination of values from sLengths[]  and link those buffers
 *               into a chain. Then, call Consume() on the first buffer in
 *               the chain with all values from sLengths[]. After calling the
 *               method, verify correctly adjusted the state of the first
 *               buffer and appropriate return pointer from the method's call.
 */
void PacketBufferTest::CheckConsume(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            // consume various amounts of memory
            for (uint16_t consumeLength : sLengths)
            {
                // start with various initial length for the first buffer
                for (uint16_t firstLength : sLengths)
                {
                    // start with various initial length for the second buffer
                    for (uint16_t secondLength : sLengths)
                    {
                        PacketBuffer * buffer_1;
                        PacketBuffer * buffer_2;
                        PacketBuffer * returned;
                        uint16_t buf_1_len = 0;
                        uint16_t buf_2_len = 0;

                        if (theFirstContext == theSecondContext)
                        {
                            continue;
                        }

                        buffer_1 = PrepareTestBuffer(theFirstContext);
                        buffer_2 = PrepareTestBuffer(theSecondContext);

                        theFirstContext->buf->next = theSecondContext->buf;

                        // Add various lengths to buffers
                        buffer_1->SetDataLength(firstLength, buffer_1);
                        buffer_2->SetDataLength(secondLength, buffer_1);

                        buf_1_len = theFirstContext->buf->len;
                        buf_2_len = theSecondContext->buf->len;

                        returned = buffer_1->Consume(consumeLength);

                        if (consumeLength == 0)
                        {
                            NL_TEST_ASSERT(inSuite, returned == buffer_1);
                            continue;
                        }

                        if (consumeLength < buf_1_len)
                        {
                            NL_TEST_ASSERT(inSuite, returned == buffer_1);
                        }
                        else if ((consumeLength >= buf_1_len) &&
                                 (consumeLength < buf_1_len + buf_2_len ||
                                  (consumeLength == buf_1_len + buf_2_len && buf_2_len == 0)))
                        {
                            NL_TEST_ASSERT(inSuite, returned == buffer_2);
                            theFirstContext->buf = nullptr;
                        }
                        else if (consumeLength >= (buf_1_len + buf_2_len))
                        {
                            NL_TEST_ASSERT(inSuite, returned == nullptr);
                            theFirstContext->buf  = nullptr;
                            theSecondContext->buf = nullptr;
                        }
                    }
                }
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::EnsureReservedSize() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               manually specify how much space is reserved in the buffer.
 *               Then, verify that EnsureReservedSize() method correctly
 *               retrieves the amount of the reserved space.
 */
void PacketBufferTest::CheckEnsureReservedSize(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        for (uint16_t length : sLengths)
        {
            PacketBuffer & lBuffer    = *PrepareTestBuffer(theContext);
            const uint16_t kAllocSize = lBuffer.AllocSize();
            uint16_t reserved_size    = theContext->reserved_size;

            if (CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE + theContext->reserved_size > kAllocSize)
            {
                reserved_size = static_cast<uint16_t>(kAllocSize - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE);
            }

            if (length <= reserved_size)
            {
                NL_TEST_ASSERT(inSuite, lBuffer.EnsureReservedSize(length) == true);
                continue;
            }

            if ((length + theContext->init_len) > (kAllocSize - CHIP_SYSTEM_PACKETBUFFER_HEADER_SIZE))
            {
                NL_TEST_ASSERT(inSuite, lBuffer.EnsureReservedSize(length) == false);
                continue;
            }

            NL_TEST_ASSERT(inSuite, lBuffer.EnsureReservedSize(length) == true);
            NL_TEST_ASSERT(inSuite, theContext->buf->payload == (theContext->payload_ptr + length - reserved_size));
        }

        theContext++;
    }
}

/**
 *  Test PacketBuffer::AlignPayload() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               manually specify how much space is reserved and the
 *               required payload shift. Then, verify that AlignPayload()
 *               method correctly aligns the payload start pointer.
 */
void PacketBufferTest::CheckAlignPayload(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        for (size_t n = 0; n < kTestLengths - 1; n++)
        {
            PacketBuffer & lBuffer    = *PrepareTestBuffer(theContext);
            const uint16_t kAllocSize = lBuffer.AllocSize();

            if (sLengths[n] == 0)
            {
                NL_TEST_ASSERT(inSuite, lBuffer.AlignPayload(sLengths[n]) == false);
                continue;
            }

            uint16_t reserved_size = theContext->reserved_size;
            if (theContext->reserved_size > kAllocSize)
            {
                reserved_size = kAllocSize;
            }

            uint16_t payload_offset = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(lBuffer.Start()) % sLengths[n]);
            uint16_t payload_shift  = 0;
            if (payload_offset > 0)
                payload_shift = static_cast<uint16_t>(sLengths[n] - payload_offset);

            if (payload_shift <= kAllocSize - reserved_size)
            {
                NL_TEST_ASSERT(inSuite, lBuffer.AlignPayload(sLengths[n]) == true);
                NL_TEST_ASSERT(inSuite, ((unsigned long) lBuffer.Start() % sLengths[n]) == 0);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, lBuffer.AlignPayload(sLengths[n]) == false);
            }
        }

        theContext++;
    }
}

/**
 *  Test PacketBuffer::Next() function.
 */
void PacketBufferTest::CheckNext(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            PacketBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
            PacketBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);

            if (theFirstContext != theSecondContext)
            {
                theFirstContext->buf->next = theSecondContext->buf;

                NL_TEST_ASSERT(inSuite, buffer_1->ChainedBuffer() == buffer_2);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, !buffer_1->HasChainedBuffer());
            }

            NL_TEST_ASSERT(inSuite, !buffer_2->HasChainedBuffer());
            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::AddRef() function.
 */
void PacketBufferTest::CheckAddRef(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBuffer * buffer = PrepareTestBuffer(theContext);
        buffer->AddRef();

        NL_TEST_ASSERT(inSuite, theContext->buf->ref == 2);

        theContext++;
    }
}

/**
 *  Test PacketBuffer::NewWithAvailableSize() and PacketBuffer::Free() functions.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance using NewWithAvailableSize() method. Then, verify that
 *               when the size of the reserved space passed to NewWithAvailableSize() is
 *               greater than #CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX, the method
 *               returns NULL. Otherwise, check for correctness of initializing
 *               the new buffer's internal state. Finally, free the buffer.
 */
void PacketBufferTest::CheckNewWithAvailableSizeAndFree(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = static_cast<struct TestContext *>(inContext);
    PacketBufferHandle buffer;

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct pbuf * pb = nullptr;

        buffer = PacketBuffer::NewWithAvailableSize(theContext->reserved_size, 0);

        if (theContext->reserved_size > CHIP_SYSTEM_CONFIG_PACKETBUFFER_CAPACITY_MAX)
        {
            NL_TEST_ASSERT(inSuite, buffer.IsNull());
            theContext++;
            continue;
        }

        NL_TEST_ASSERT(inSuite, theContext->reserved_size <= buffer->AllocSize());
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());

        if (!buffer.IsNull())
        {
            pb = TO_LWIP_PBUF(buffer.Get());

            NL_TEST_ASSERT(inSuite, pb->len == 0);
            NL_TEST_ASSERT(inSuite, pb->tot_len == 0);
            NL_TEST_ASSERT(inSuite, pb->next == nullptr);
            NL_TEST_ASSERT(inSuite, pb->ref == 1);
        }

        theContext++;
    }

    // Use the rest of the buffer space
    for (;;)
    {
        buffer = PacketBuffer::NewWithAvailableSize(0, 0);
        if (buffer.IsNull())
        {
            break;
        }
        // Leak the buffer on purpose, to use up all the buffer space.
        // (Compilers other than GCC recognize the `(void)` idiom, which is mandated in C++20.)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        (void) buffer.Release_ForNow();
#pragma GCC diagnostic pop
    }
}

/**
 *  Test PacketBuffer::Free() function.
 *
 *  Description: Take two different initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, chain two buffers together and set each
 *               buffer's reference count to one of the values from
 *               init_ret_count[]. Then, call Free() on the first buffer in
 *               the chain and verify correctly adjusted states of the two
 *               buffers.
 */
void PacketBufferTest::CheckFree(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            const uint8_t init_ref_count[] = { 1, 2, 3 };
            const int refs                 = sizeof(init_ref_count) / sizeof(uint16_t);

            // start with various buffer ref counts
            for (size_t r = 0; r < refs; r++)
            {
                PacketBuffer * buffer_1;

                if (theFirstContext == theSecondContext)
                {
                    continue;
                }

                buffer_1 = PrepareTestBuffer(theFirstContext);
                (void) PrepareTestBuffer(theSecondContext);

                theFirstContext->buf->next = theSecondContext->buf;

                // Add various buffer ref counts
                theFirstContext->buf->ref  = init_ref_count[r];
                theSecondContext->buf->ref = init_ref_count[(r + 1) % refs];

                PacketBuffer::Free(buffer_1);

                NL_TEST_ASSERT(inSuite, theFirstContext->buf->ref == (init_ref_count[r] - 1));

                if (init_ref_count[r] == 1)
                {
                    NL_TEST_ASSERT(inSuite, theSecondContext->buf->ref == (init_ref_count[(r + 1) % refs] - 1));
                }
                else
                {
                    NL_TEST_ASSERT(inSuite, theSecondContext->buf->ref == (init_ref_count[(r + 1) % refs]));
                }

                if (init_ref_count[r] > 1)
                {
                    NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == theSecondContext->buf);
                }

                if (theFirstContext->buf->ref == 0)
                {
                    theFirstContext->buf = nullptr;
                }

                if (theSecondContext->buf->ref == 0)
                {
                    theSecondContext->buf = nullptr;
                }
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::FreeHead() function.
 *
 *  Description: Take two different initial configurations of PacketBuffer from
 *               inContext and create two PacketBuffer instances based on those
 *               configurations. Next, chain two buffers together. Then, call
 *               FreeHead() on the first buffer in the chain and verify that
 *               the method returned pointer to the second buffer.
 */
void PacketBufferTest::CheckFreeHead(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            PacketBuffer * buffer_1;
            PacketBuffer * buffer_2;
            PacketBuffer * returned = nullptr;

            if (theFirstContext == theSecondContext)
            {
                continue;
            }

            buffer_1 = PrepareTestBuffer(theFirstContext);
            buffer_2 = PrepareTestBuffer(theSecondContext);

            theFirstContext->buf->next = theSecondContext->buf;

            returned = PacketBuffer::FreeHead(buffer_1);

            NL_TEST_ASSERT(inSuite, returned == buffer_2);

            theFirstContext->buf = nullptr;
            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test PacketBuffer::BuildFreeList() function.
 */
void PacketBufferTest::CheckBuildFreeList(nlTestSuite * inSuite, void * inContext)
{
    // BuildFreeList() is a private method called automatically.
    (void) inSuite;
    (void) inContext;
}

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("PacketBuffer::NewWithAvailableSize&PacketBuffer::Free", CheckNewWithAvailableSizeAndFree),
    NL_TEST_DEF("PacketBuffer::Start",                          CheckStart),
    NL_TEST_DEF("PacketBuffer::SetStart",                       CheckSetStart),
    NL_TEST_DEF("PacketBuffer::DataLength",                     CheckDataLength),
    NL_TEST_DEF("PacketBuffer::SetDataLength",                  CheckSetDataLength),
    NL_TEST_DEF("PacketBuffer::TotalLength",                    CheckTotalLength),
    NL_TEST_DEF("PacketBuffer::MaxDataLength",                  CheckMaxDataLength),
    NL_TEST_DEF("PacketBuffer::AvailableDataLength",            CheckAvailableDataLength),
    NL_TEST_DEF("PacketBuffer::ReservedSize",                   CheckReservedSize),
    NL_TEST_DEF("PacketBuffer::AddToEnd",                       CheckAddToEnd),
    NL_TEST_DEF("PacketBuffer::DetachTail",                     CheckDetachTail),
    NL_TEST_DEF("PacketBuffer::CompactHead",                    CheckCompactHead),
    NL_TEST_DEF("PacketBuffer::ConsumeHead",                    CheckConsumeHead),
    NL_TEST_DEF("PacketBuffer::Consume",                        CheckConsume),
    NL_TEST_DEF("PacketBuffer::EnsureReservedSize",             CheckEnsureReservedSize),
    NL_TEST_DEF("PacketBuffer::AlignPayload",                   CheckAlignPayload),
    NL_TEST_DEF("PacketBuffer::Next",                           CheckNext),
    NL_TEST_DEF("PacketBuffer::AddRef",                         CheckAddRef),
    NL_TEST_DEF("PacketBuffer::Free",                           CheckFree),
    NL_TEST_DEF("PacketBuffer::FreeHead",                       CheckFreeHead),
    NL_TEST_DEF("PacketBuffer::BuildFreeList",                  CheckBuildFreeList),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 * Set up the test suite.
 *
 *  This is a work-around to initiate PacketBuffer protected class instance's data and set it to a known state, before an instance
 *  is created.
 */
int TestSetup(void * inContext)
{
    struct TestContext * theContext = reinterpret_cast<TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        BufferAlloc(theContext);
        theContext++;
    }

    return (SUCCESS);
}

/**
 * Tear down the test suite.
 *
 *  Free memory reserved at TestSetup.
 */
int TestTeardown(void * inContext)
{
    struct TestContext * theContext = reinterpret_cast<TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        PacketBufferTest::BufferFree(theContext);
        theContext++;
    }

    return (SUCCESS);
}

int TestSystemPacketBuffer(void)
{ /*
 #if CHIP_SYSTEM_CONFIG_USE_LWIP
     tcpip_init(NULL, NULL);
 #endif // CHIP_SYSTEM_CONFIG_USE_LWIP
 */

    // clang-format off
    nlTestSuite theSuite =
	{
        "chip-system-packetbuffer",
        &sTests[0],
        TestSetup,
        TestTeardown
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, &sContext);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSystemPacketBuffer)
