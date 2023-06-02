/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines objects for Verhoeff's check-digit algorithm
 *      for strings with various bases and an object for the core
 *      algorithm operations.
 *
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <lib/support/DLLUtil.h>

class VerhoeffTest;

// Verhoeff10 -- Implements Verhoeff's check-digit algorithm for base-10 strings.
//
class DLL_EXPORT Verhoeff10
{
    friend class VerhoeffTest;

public:
    enum
    {
        Base        = 10,
        PolygonSize = 5
    };

    // Compute a check character for a given string.
    static char ComputeCheckChar(const char * str);
    static char ComputeCheckChar(const char * str, size_t strLen);

    // Verify a check character against a given string.
    static bool ValidateCheckChar(char checkChar, const char * str);
    static bool ValidateCheckChar(char checkChar, const char * str, size_t strLen);

    // Verify a check character at the end of a given string.
    static bool ValidateCheckChar(const char * str);
    static bool ValidateCheckChar(const char * str, size_t strLen);

    // Convert between a character and its corresponding value.
    static int CharToVal(char ch);
    static char ValToChar(int val);

private:
    Verhoeff10()  = delete;
    ~Verhoeff10() = delete;

    static const uint8_t sMultiplyTable[];
    static const uint8_t sPermTable[];
};

// Verhoeff -- Implements core functions for Verhoeff's algorithm.
//
class Verhoeff
{
public:
    static int DihedralMultiply(int x, int y, int n);
    static int DihedralInvert(int val, int n);
    static int Permute(int val, const uint8_t * permTable, int permTableLen, uint64_t iterCount);
};
