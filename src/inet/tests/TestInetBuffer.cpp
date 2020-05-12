/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file implements a unit test suite for
 *      <tt>chip::Inet::InetBuffer</tt>, a class that provides
 *      structure for network buffer management.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "TestInetLayer.h"

#include <stdint.h>

#include <inet/InetConfig.h>

#if INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

#include <string.h>

#include <inet/InetBuffer.h>

#include <nlunit-test.h>

//#include "ToolCommon.h"

#define MEM_ALIGN_SIZE(SZ, ALIGN) (((SZ) + (ALIGN) -1) & ~((ALIGN) -1))
#if INET_LWIP
#define INET_BUF_SIZE LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE)
#define INET_BUF_HEADER_SIZE LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf))
#else
#define INET_BUF_SIZE CHIP_SYSTEM_PACKETBUFFER_SIZE
#define INET_BUF_HEADER_SIZE MEM_ALIGN_SIZE(sizeof(InetBuffer), 4)
#endif

#define INET_TO_PBUF(x) (static_cast<struct pbuf *>(static_cast<void *>(x)))
#define PBUF_TO_INET(x) (static_cast<InetBuffer *>(static_cast<void *>(x)))

using namespace chip::Inet;

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

static struct TestContext sContext[] = { { 0, 0, NULL, NULL, NULL, NULL },
                                         { 0, 10, NULL, NULL, NULL, NULL },
                                         { 0, 128, NULL, NULL, NULL, NULL },
                                         { 0, CHIP_SYSTEM_PACKETBUFFER_SIZE, NULL, NULL, NULL, NULL },
                                         { 0, INET_BUF_SIZE, NULL, NULL, NULL, NULL } };

static const uint16_t sLengths[] = { 0, 1, 10, 128, INET_BUF_SIZE, UINT16_MAX };

// Number of test context examples.
static const size_t kTestElements = sizeof(sContext) / sizeof(struct TestContext);
static const size_t kTestLengths  = sizeof(sLengths) / sizeof(uint16_t);

// Utility functions.

/**
 *  Free allocated test buffer memory.
 */
static void BufferFree(struct TestContext * theContext)
{
    if (theContext->buf != NULL)
    {
        InetBuffer::Free(PBUF_TO_INET(theContext->buf));
    }
}

/**
 *  Allocate memory for a test buffer and configure according to test context.
 */
static void BufferAlloc(struct TestContext * theContext)
{
    if (theContext->buf == NULL)
    {
        theContext->buf = INET_TO_PBUF(InetBuffer::New(0));
    }

    if (theContext->buf == NULL)
    {
        fprintf(stderr, "Failed to allocate %zuB memory: %s\n", ((size_t) INET_BUF_SIZE), strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(theContext->buf, 0, INET_BUF_SIZE);

    theContext->start_buffer = reinterpret_cast<uint8_t *>(theContext->buf);
    theContext->end_buffer   = reinterpret_cast<uint8_t *>(theContext->buf) + INET_BUF_SIZE;

    if (INET_BUF_HEADER_SIZE + theContext->reserved_size > INET_BUF_SIZE)
    {
        theContext->payload_ptr = (theContext->start_buffer + INET_BUF_SIZE);
    }
    else
    {
        theContext->payload_ptr = (theContext->start_buffer + INET_BUF_HEADER_SIZE + theContext->reserved_size);
    }
}

/**
 *  Setup buffer layout as it is used by InetBuffer class.
 */
static InetBuffer * PrepareTestBuffer(struct TestContext * theContext)
{
    BufferAlloc(theContext);

    theContext->buf->next    = NULL;
    theContext->buf->payload = theContext->payload_ptr;
    theContext->buf->ref     = 1;
    theContext->buf->len     = theContext->init_len;
    theContext->buf->tot_len = theContext->init_len;
#if INET_LWIP
    theContext->buf->type = PBUF_POOL;
#endif

    return reinterpret_cast<InetBuffer *>(theContext->buf);
}

// Test functions invoked from the suite.

/**
 *  Test InetBuffer::Start() function.
 */
static void CheckStart(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->Start() == theContext->payload_ptr);

        theContext++;
    }
}

/**
 *  Test InetBuffer::SetStart() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               for any offset value from start_offset[], pass it to the
 *               buffer's instance through SetStart method. Then, verify that
 *               the beginning of the buffer has been correctly internally
 *               adjusted according to the offset value passed into the
 *               SetStart() method.
 */
static void CheckSetStart(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        const int start_offset[] = { -static_cast<int>(INET_BUF_SIZE), -128, -1, 0, 1, 128, static_cast<int>(INET_BUF_SIZE) };

        for (size_t s = 0; s < sizeof(start_offset) / sizeof(int); s++)
        {
            uint8_t * test_start   = theContext->payload_ptr + start_offset[s];
            uint8_t * verify_start = test_start;
            InetBuffer * buffer    = PrepareTestBuffer(theContext);

            buffer->SetStart(test_start);

            if (verify_start <= theContext->start_buffer + INET_BUF_HEADER_SIZE)
            {
                // Set start before valid payload beginning.
                verify_start = theContext->start_buffer + INET_BUF_HEADER_SIZE;
            }

            if (verify_start >= theContext->end_buffer)
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
 *  Test InetBuffer::DataLength() function.
 */
static void CheckDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->DataLength() == theContext->buf->len);

        theContext++;
    }
}

/**
 *  Test InetBuffer::SetDataLength() function.
 *
 *  Description: Take two initial configurations of InetBuffer from
 *               inContext and create two InetBuffer instances based on those
 *               configurations. For any two buffers, call SetDataLength with
 *               different value from sLength[]. If two buffers are created with
 *               the same configuration, test SetDataLength on one buffer,
 *               without specifying the head of the buffer chain. Otherwise,
 *               test SetDataLength with one buffer being down the chain and the
 *               other one being passed as the head of the chain. After calling
 *               the method verify that data lenghts were correctly adjusted.
 */
static void CheckSetDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            for (size_t n = 0; n < kTestLengths; n++)
            {
                InetBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
                InetBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);

                if (theFirstContext == theSecondContext)
                {
                    // headOfChain (the second arg) is NULL
                    buffer_2->SetDataLength(sLengths[n], NULL);

                    if (sLengths[n] > (theSecondContext->end_buffer - theSecondContext->payload_ptr))
                    {
                        NL_TEST_ASSERT(
                            inSuite, theSecondContext->buf->len == (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite,
                                       theSecondContext->buf->tot_len ==
                                           (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == NULL);
                    }
                    else
                    {
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->len == sLengths[n]);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->tot_len == sLengths[n]);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == NULL);
                    }
                }
                else
                {
                    // headOfChain (the second arg) is buffer_1
                    buffer_2->SetDataLength(sLengths[n], buffer_1);

                    if (sLengths[n] > (theSecondContext->end_buffer - theSecondContext->payload_ptr))
                    {
                        NL_TEST_ASSERT(
                            inSuite, theSecondContext->buf->len == (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite,
                                       theSecondContext->buf->tot_len ==
                                           (theSecondContext->end_buffer - theSecondContext->payload_ptr));
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == NULL);

                        NL_TEST_ASSERT(inSuite,
                                       theFirstContext->buf->tot_len ==
                                           (theFirstContext->init_len +
                                            static_cast<int32_t>(theSecondContext->end_buffer - theSecondContext->payload_ptr) -
                                            static_cast<int32_t>(theSecondContext->init_len)));
                    }
                    else
                    {
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->len == sLengths[n]);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->tot_len == sLengths[n]);
                        NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == NULL);

                        NL_TEST_ASSERT(inSuite,
                                       theFirstContext->buf->tot_len ==
                                           (theFirstContext->init_len + static_cast<int32_t>(sLengths[n]) -
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
 *  Test InetBuffer::TotalLength() function.
 */
static void CheckTotalLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->TotalLength() == theContext->init_len);

        theContext++;
    }
}

/**
 *  Test InetBuffer::MaxDataLength() function.
 */
static void CheckMaxDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(inSuite, buffer->MaxDataLength() == (theContext->end_buffer - theContext->payload_ptr));

        theContext++;
    }
}

/**
 *  Test InetBuffer::AvailableDataLength() function.
 */
static void CheckAvailableDataLength(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);

        NL_TEST_ASSERT(
            inSuite, buffer->AvailableDataLength() == ((theContext->end_buffer - theContext->payload_ptr) - theContext->init_len));

        theContext++;
    }
}

/**
 *  Test InetBuffer::ReservedSize() function.
 */
static void CheckReservedSize(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);

        if (INET_BUF_HEADER_SIZE + theContext->reserved_size > INET_BUF_SIZE)
        {
            NL_TEST_ASSERT(inSuite, buffer->ReservedSize() == (INET_BUF_SIZE - INET_BUF_HEADER_SIZE));
        }
        else
        {
            NL_TEST_ASSERT(inSuite, buffer->ReservedSize() == theContext->reserved_size);
        }

        theContext++;
    }
}

/**
 *  Test InetBuffer::AddToEnd() function.
 *
 *  Description: Take three initial configurations of InetBuffer from
 *               inContext, create three InetBuffers based on those
 *               configurations and then link those buffers together with
 *               InetBuffer:AddToEnd(). Then, assert that after connecting
 *               buffers together, their internal states are correctly updated.
 *               This test function tests linking any combination of three
 *               buffer-configurations passed within inContext.
 */
static void CheckAddToEnd(nlTestSuite * inSuite, void * inContext)
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
                InetBuffer * buffer_1 = NULL;
                InetBuffer * buffer_2 = NULL;
                InetBuffer * buffer_3 = NULL;

                if (theFirstContext == theSecondContext || theFirstContext == theThirdContext ||
                    theSecondContext == theThirdContext)
                {
                    theThirdContext++;
                    continue;
                }

                buffer_1 = PrepareTestBuffer(theFirstContext);
                buffer_2 = PrepareTestBuffer(theSecondContext);
                buffer_3 = PrepareTestBuffer(theThirdContext);

                buffer_1->AddToEnd(buffer_2);

                NL_TEST_ASSERT(inSuite, theFirstContext->buf->tot_len == (theFirstContext->init_len + theSecondContext->init_len));
                NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == theSecondContext->buf);
                NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == NULL);

                NL_TEST_ASSERT(inSuite, theThirdContext->buf->next == NULL);

                buffer_1->AddToEnd(buffer_3);

                NL_TEST_ASSERT(inSuite,
                               theFirstContext->buf->tot_len ==
                                   (theFirstContext->init_len + theSecondContext->init_len + theThirdContext->init_len));
                NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == theSecondContext->buf);
                NL_TEST_ASSERT(inSuite, theSecondContext->buf->next == theThirdContext->buf);
                NL_TEST_ASSERT(inSuite, theThirdContext->buf->next == NULL);

                theThirdContext++;
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test InetBuffer::DetachTail() function.
 *
 *  Description: Take two initial configurations of InetBuffer from
 *               inContext and create two InetBuffer instances based on those
 *               configurations. Next, link those buffers together, with the first
 *               buffer instance pointing to the second one. Then, call DetachTail()
 *               on the first buffer to unlink the second buffer. After the call,
 *               verify correct internal state of the first buffer.
 */
static void CheckDetachTail(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            InetBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
            InetBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);
            InetBuffer * returned = NULL;

            if (theFirstContext != theSecondContext)
            {
                theFirstContext->buf->next = theSecondContext->buf;
                theFirstContext->buf->tot_len += theSecondContext->init_len;
            }

            returned = buffer_1->DetachTail();

            NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == NULL);
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
 *  Test InetBuffer::CompactHead() function.
 *
 *  Description: Take two initial configurations of InetBuffer from
 *               inContext and create two InetBuffer instances based on those
 *               configurations. Next, set both buffers' data length to any
 *               combination of values from sLengths[] and link those buffers
 *               into a chain. Then, call CompactHead() on the first buffer in
 *               the chain. After calling the method, verify correctly adjusted
 *               state of the first buffer.
 */
static void CheckCompactHead(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            // start with various initial length for the first buffer
            for (size_t k = 0; k < kTestLengths; k++)
            {
                // start with various initial length for the second buffer
                for (size_t l = 0; l < kTestLengths; l++)
                {
                    InetBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
                    InetBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);
                    uint16_t len1         = 0;
                    uint16_t len2         = 0;

                    buffer_1->SetDataLength(sLengths[k], buffer_1);
                    len1 = buffer_1->DataLength();

                    if (theFirstContext != theSecondContext)
                    {
                        theFirstContext->buf->next = theSecondContext->buf;

                        // Add various lengths to the second buffer
                        buffer_2->SetDataLength(sLengths[l], buffer_1);
                        len2 = buffer_2->DataLength();
                    }

                    buffer_1->CompactHead();

                    NL_TEST_ASSERT(inSuite,
                                   theFirstContext->buf->payload == (theFirstContext->start_buffer + INET_BUF_HEADER_SIZE));

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
                            NL_TEST_ASSERT(inSuite, theFirstContext->buf->next == NULL);
                            theSecondContext->buf = NULL;
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
 *  Test InetBuffer::ConsumeHead() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               for any value from sLengths[], pass it to the buffer's
 *               instance through ConsumeHead() method. Then, verify that
 *               the internal state of the buffer has been correctly
 *               adjusted according to the value passed into the method.
 */
static void CheckConsumeHead(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        for (size_t n = 0; n < kTestLengths; n++)
        {
            InetBuffer * buffer = PrepareTestBuffer(theContext);

            buffer->ConsumeHead(sLengths[n]);

            if (sLengths[n] > theContext->init_len)
            {
                NL_TEST_ASSERT(inSuite, theContext->buf->payload == (theContext->payload_ptr + theContext->init_len));
                NL_TEST_ASSERT(inSuite, theContext->buf->len == 0);
                NL_TEST_ASSERT(inSuite, theContext->buf->tot_len == 0);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, theContext->buf->payload == (theContext->payload_ptr + sLengths[n]));
                NL_TEST_ASSERT(inSuite, theContext->buf->len == (theContext->buf->len - sLengths[n]));
                NL_TEST_ASSERT(inSuite, theContext->buf->tot_len == (theContext->buf->tot_len - sLengths[n]));
            }

            if (theContext->buf->ref == 0)
            {
                theContext->buf = NULL;
            }
        }

        theContext++;
    }
}

/**
 *  Test InetBuffer::Consume() function.
 *
 *  Description: Take two different initial configurations of InetBuffer from
 *               inContext and create two InetBuffer instances based on those
 *               configurations. Next, set both buffers' data length to any
 *               combination of values from sLengths[]  and link those buffers
 *               into a chain. Then, call Consume() on the first buffer in
 *               the chain with all values from sLengths[]. After calling the
 *               method, verify correctly adjusted the state of the first
 *               buffer and appropriate return pointer from the method's call.
 */
static void CheckConsume(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            // consume various amounts of memory
            for (size_t c = 0; c < kTestLengths; c++)
            {
                // start with various initial length for the first buffer
                for (size_t k = 0; k < kTestLengths; k++)
                {
                    // start with various initial length for the second buffer
                    for (size_t l = 0; l < kTestLengths; l++)
                    {
                        InetBuffer * buffer_1;
                        InetBuffer * buffer_2;
                        InetBuffer * returned;
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
                        buffer_1->SetDataLength(sLengths[k], buffer_1);
                        buffer_2->SetDataLength(sLengths[l], buffer_1);

                        buf_1_len = theFirstContext->buf->len;
                        buf_2_len = theSecondContext->buf->len;

                        returned = buffer_1->Consume(sLengths[c]);

                        if (sLengths[c] == 0)
                        {
                            NL_TEST_ASSERT(inSuite, returned == buffer_1);
                            continue;
                        }

                        if (sLengths[c] < buf_1_len)
                        {
                            NL_TEST_ASSERT(inSuite, returned == buffer_1);
                        }
                        else if ((sLengths[c] >= buf_1_len) &&
                                 ((sLengths[c] < buf_1_len + buf_2_len) ||
                                  ((sLengths[c] == buf_1_len + buf_2_len) && buf_2_len == 0)))
                        {
                            NL_TEST_ASSERT(inSuite, returned == buffer_2);
                            theFirstContext->buf = NULL;
                        }
                        else if (sLengths[c] >= (buf_1_len + buf_2_len))
                        {
                            NL_TEST_ASSERT(inSuite, returned == NULL);
                            theFirstContext->buf  = NULL;
                            theSecondContext->buf = NULL;
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
 *  Test InetBuffer::EnsureReservedSize() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               manually specify how much space is reserved in the buffer.
 *               Then, verify that EnsureReservedSize() method correctly
 *               retrieves the amount of the reserved space.
 */
static void CheckEnsureReservedSize(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        for (size_t n = 0; n < kTestLengths; n++)
        {
            InetBuffer * buffer    = PrepareTestBuffer(theContext);
            uint16_t reserved_size = theContext->reserved_size;

            if (INET_BUF_HEADER_SIZE + theContext->reserved_size > INET_BUF_SIZE)
            {
                reserved_size = INET_BUF_SIZE - INET_BUF_HEADER_SIZE;
            }

            if (sLengths[n] <= reserved_size)
            {
                NL_TEST_ASSERT(inSuite, buffer->EnsureReservedSize(sLengths[n]) == true);
                continue;
            }

            if ((sLengths[n] + theContext->init_len) > (INET_BUF_SIZE - INET_BUF_HEADER_SIZE))
            {
                NL_TEST_ASSERT(inSuite, buffer->EnsureReservedSize(sLengths[n]) == false);
                continue;
            }

            NL_TEST_ASSERT(inSuite, buffer->EnsureReservedSize(sLengths[n]) == true);
            NL_TEST_ASSERT(inSuite, theContext->buf->payload == (theContext->payload_ptr + sLengths[n] - reserved_size));
        }

        theContext++;
    }
}

/**
 *  Test InetBuffer::AlignPayload() function.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance according to the configuration. Next,
 *               manually specify how much space is reserved and the
 *               required payload shift. Then, verify that AlignPayload()
 *               method correctly aligns the payload start pointer.
 */
static void CheckAlignPayload(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        for (size_t n = 0; n < kTestLengths - 1; n++)
        {
            InetBuffer * buffer = PrepareTestBuffer(theContext);

            if (sLengths[n] == 0)
            {
                NL_TEST_ASSERT(inSuite, buffer->AlignPayload(sLengths[n]) == false);
                continue;
            }

            uint16_t reserved_size = theContext->reserved_size;
            if (INET_BUF_HEADER_SIZE + theContext->reserved_size > INET_BUF_SIZE)
            {
                reserved_size = INET_BUF_SIZE - INET_BUF_HEADER_SIZE;
            }

            uint16_t payload_offset = (unsigned long) buffer->Start() % sLengths[n];
            uint16_t payload_shift  = 0;
            if (payload_offset > 0)
                payload_shift = sLengths[n] - payload_offset;

            if (payload_shift <= INET_BUF_SIZE - INET_BUF_HEADER_SIZE - reserved_size)
            {
                NL_TEST_ASSERT(inSuite, buffer->AlignPayload(sLengths[n]) == true);
                NL_TEST_ASSERT(inSuite, ((unsigned long) buffer->Start() % sLengths[n]) == 0);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, buffer->AlignPayload(sLengths[n]) == false);
            }
        }

        theContext++;
    }
}

/**
 *  Test InetBuffer::Next() function.
 */
static void CheckNext(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            InetBuffer * buffer_1 = PrepareTestBuffer(theFirstContext);
            InetBuffer * buffer_2 = PrepareTestBuffer(theSecondContext);

            if (theFirstContext != theSecondContext)
            {
                theFirstContext->buf->next = theSecondContext->buf;

                NL_TEST_ASSERT(inSuite, buffer_1->Next() == buffer_2);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, buffer_1->Next() == NULL);
            }

            NL_TEST_ASSERT(inSuite, buffer_2->Next() == NULL);
            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test InetBuffer::AddRef() function.
 */
static void CheckAddRef(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        InetBuffer * buffer = PrepareTestBuffer(theContext);
        buffer->AddRef();

        NL_TEST_ASSERT(inSuite, theContext->buf->ref == 2);

        theContext++;
    }
}

/**
 *  Test InetBuffer::New() and InetBuffer::Free() functions.
 *
 *  Description: For every buffer-configuration from inContext, create a
 *               buffer's instance using New() method. Then, verify that
 *               when the size of the reserved space passed to New() is
 *               greater than INET_BUF_SIZE - INET_BUF_HEADER_SIZE, the method
 *               returns NULL. Otherwise, check for correctness of initializing
 *               the new buffer's internal state. Finally, free the buffer.
 */
static void CheckNewAndFree(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);
    InetBuffer * buffer;

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct pbuf * pb = NULL;

        buffer = InetBuffer::New(theContext->reserved_size);

        if (theContext->reserved_size + INET_BUF_HEADER_SIZE > INET_BUF_SIZE)
        {
            NL_TEST_ASSERT(inSuite, buffer == NULL);
            theContext++;
            continue;
        }

        NL_TEST_ASSERT(inSuite, buffer != NULL);

        if (buffer != NULL)
        {
            pb = INET_TO_PBUF(buffer);

            NL_TEST_ASSERT(inSuite, pb->len == 0);
            NL_TEST_ASSERT(inSuite, pb->tot_len == 0);
            NL_TEST_ASSERT(inSuite, pb->next == NULL);
            NL_TEST_ASSERT(inSuite, pb->ref == 1);
        }

        InetBuffer::Free(buffer);

        theContext++;
    }

    // Use the rest of the buffer space
    do
    {
        buffer = InetBuffer::New();
    } while (buffer != NULL);
}

/**
 *  Test InetBuffer::Free() function.
 *
 *  Description: Take two different initial configurations of InetBuffer from
 *               inContext and create two InetBuffer instances based on those
 *               configurations. Next, chain two buffers together and set each
 *               buffer's reference count to one of the values from
 *               init_ret_count[]. Then, call Free() on the first buffer in
 *               the chain and verify correctly adjusted states of the two
 *               buffers.
 */
static void CheckFree(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            const uint16_t init_ref_count[] = { 1, 2, 3 };
            const int refs                  = sizeof(init_ref_count) / sizeof(uint16_t);

            // start with various buffer ref counts
            for (size_t r = 0; r < refs; r++)
            {
                InetBuffer * buffer_1;

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

                InetBuffer::Free(buffer_1);

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
                    theFirstContext->buf = NULL;
                }

                if (theSecondContext->buf->ref == 0)
                {
                    theSecondContext->buf = NULL;
                }
            }

            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test InetBuffer::FreeHead() function.
 *
 *  Description: Take two different initial configurations of InetBuffer from
 *               inContext and create two InetBuffer instances based on those
 *               configurations. Next, chain two buffers together. Then, call
 *               FreeHead() on the first buffer in the chain and verify that
 *               the method returned pointer to the second buffer.
 */
static void CheckFreeHead(nlTestSuite * inSuite, void * inContext)
{
    struct TestContext * theFirstContext = static_cast<struct TestContext *>(inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        struct TestContext * theSecondContext = static_cast<struct TestContext *>(inContext);

        for (size_t jth = 0; jth < kTestElements; jth++)
        {
            InetBuffer * buffer_1;
            InetBuffer * buffer_2;
            InetBuffer * returned = NULL;

            if (theFirstContext == theSecondContext)
            {
                continue;
            }

            buffer_1 = PrepareTestBuffer(theFirstContext);
            buffer_2 = PrepareTestBuffer(theSecondContext);

            theFirstContext->buf->next = theSecondContext->buf;

            returned = InetBuffer::FreeHead(buffer_1);

            NL_TEST_ASSERT(inSuite, returned == buffer_2);

            theFirstContext->buf = NULL;
            theSecondContext++;
        }

        theFirstContext++;
    }
}

/**
 *  Test InetBuffer::BuildFreeList() function.
 */
static void CheckBuildFreeList(nlTestSuite * inSuite, void * inContext)
{
    // BuildFreeList() is a private method called automatically.
    (void) inSuite;
    (void) inContext;
}

// Test Suite

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("InetBuffer::Start", CheckStart),
                                 NL_TEST_DEF("InetBuffer::SetStart", CheckSetStart),
                                 NL_TEST_DEF("InetBuffer::DataLength", CheckDataLength),
                                 NL_TEST_DEF("InetBuffer::SetDataLength", CheckSetDataLength),
                                 NL_TEST_DEF("InetBuffer::TotalLength", CheckTotalLength),
                                 NL_TEST_DEF("InetBuffer::MaxDataLength", CheckMaxDataLength),
                                 NL_TEST_DEF("InetBuffer::AvailableDataLength", CheckAvailableDataLength),
                                 NL_TEST_DEF("InetBuffer::ReservedSize", CheckReservedSize),
                                 NL_TEST_DEF("InetBuffer::AddToEnd", CheckAddToEnd),
                                 NL_TEST_DEF("InetBuffer::DetachTail", CheckDetachTail),
                                 NL_TEST_DEF("InetBuffer::CompactHead", CheckCompactHead),
                                 NL_TEST_DEF("InetBuffer::ConsumeHead", CheckConsumeHead),
                                 NL_TEST_DEF("InetBuffer::Consume", CheckConsume),
                                 NL_TEST_DEF("InetBuffer::EnsureReservedSize", CheckEnsureReservedSize),
                                 NL_TEST_DEF("InetBuffer::AlignPayload", CheckAlignPayload),
                                 NL_TEST_DEF("InetBuffer::Next", CheckNext),
                                 NL_TEST_DEF("InetBuffer::AddRef", CheckAddRef),
                                 NL_TEST_DEF("InetBuffer::New & InetBuffer::Free", CheckNewAndFree),
                                 NL_TEST_DEF("InetBuffer::Free", CheckFree),
                                 NL_TEST_DEF("InetBuffer::FreeHead", CheckFreeHead),
                                 NL_TEST_DEF("InetBuffer::BuildFreeList", CheckBuildFreeList),
                                 NL_TEST_SENTINEL() };

/**
 *  Set up the test suite.
 *  This is a work-around to initiate InetBuffer protected class instance's
 *  data and set it to a known state, before an instance is created.
 */
static int TestSetup(void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        BufferAlloc(theContext);

        theContext++;
    }

    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 *  Free memory reserved at TestSetup.
 */
static int TestTeardown(void * inContext)
{
    struct TestContext * theContext = (struct TestContext *) (inContext);

    for (size_t ith = 0; ith < kTestElements; ith++)
    {
        BufferFree(theContext);

        theContext++;
    }

    return (SUCCESS);
}

int TestInetBuffer(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "inet-buffer",
        &sTests[0],
        TestSetup,
        TestTeardown
    };
    // clang-format on

    // Run test suit againt one context.
    nlTestRunner(&theSuite, &sContext);

    return (nlTestRunnerStats(&theSuite));
}

#else // !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

int TestInetBuffer(void)
{
    return (0);
}

#endif // !INET_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
