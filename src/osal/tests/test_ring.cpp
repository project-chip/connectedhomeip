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

#include <limits.h>

#include "test_util.h"
#include <Ring.h>

#define TEST_ITERATIONS (2 * USHRT_MAX)

void test_ring_init(void)
{
    uint8_t val;
    uint8_t * out;
    Ring ring(sizeof(uint8_t), 4);

    val = 1;
    ring.push_back(&val);
    val = 2;
    ring.push_back(&val);

    out = (uint8_t *) ring.front();
    VerifyOrQuit(*out == 1, "test_ring: first pushed element incorrect");
    ring.pop_front();

    out = (uint8_t *) ring.front();
    VerifyOrQuit(*out == 2, "test_ring: first pushed element incorrect");
}

void test_ring_empty(void)
{
    uint8_t val;
    Ring ring(sizeof(uint8_t), 4);

    VerifyOrQuit(ring.empty(), "test_ring: new ring not empty");

    val = 1;
    ring.push_back(&val);
    VerifyOrQuit(!ring.empty(), "test_ring: 1 element ring reporting empty");
    ring.pop_front();

    VerifyOrQuit(ring.empty(), "test_ring: expecting empty ring");
}

void test_ring_full(void)
{
    uint8_t val;
    Ring ring(sizeof(uint8_t), 4);

    VerifyOrQuit(!ring.full(), "test_ring: new ring reporting as full");

    val = 1;
    ring.push_back(&val);
    val = 2;
    ring.push_back(&val);
    val = 3;
    ring.push_back(&val);
    VerifyOrQuit(!ring.full(), "test_ring: ring reporting as full");
    val = 4;
    ring.push_back(&val);
    VerifyOrQuit(ring.full(), "test_ring: full ring not reporting as full");

    ring.pop_front();
    VerifyOrQuit(!ring.full(), "test_ring: ring reporting as full");
}

void test_ring_pow2(void)
{
    Ring ring2(sizeof(uint8_t), 2);
    Ring ring4(sizeof(uint8_t), 4);
    Ring ring8(sizeof(uint8_t), 8);
    Ring ring16(sizeof(uint8_t), 16);
}

void test_ring_stress(void)
{
    uint8_t val;
    uint8_t * out;
    Ring ring(sizeof(uint8_t), 4);

    for (int i = 0; i < TEST_ITERATIONS; i++)
    {
        val = i + 1;
        ring.push_back(&val);
        val = i + 2;
        ring.push_back(&val);
        val = i + 3;
        ring.push_back(&val);
        VerifyOrQuit(!ring.full(), "test_ring: ring reporting as full");
        val = i + 4;
        ring.push_back(&val);
        VerifyOrQuit(ring.full(), "test_ring: expected full ring");

        val = i + 1;
        out = (uint8_t *) ring.front();
        VerifyOrQuit(*out == val, "test_ring: popped element incorrect != 1");
        ring.pop_front();

        val = i + 2;
        out = (uint8_t *) ring.front();
        VerifyOrQuit(*out == val, "test_ring: popped element incorrect != 2");
        ring.pop_front();

        val = i + 3;
        out = (uint8_t *) ring.front();
        VerifyOrQuit(*out == val, "test_ring: popped element incorrect != 3");
        ring.pop_front();

        val = i + 4;
        out = (uint8_t *) ring.front();
        VerifyOrQuit(*out == val, "test_ring: popped element incorrect != 4");
        ring.pop_front();

        VerifyOrQuit(ring.empty(), "test_ring: expected empty ring");
    }
}

int main(void)
{
    test_ring_init();
    test_ring_empty();
    test_ring_full();
    test_ring_pow2();
    test_ring_stress();

    return PASS;
}
