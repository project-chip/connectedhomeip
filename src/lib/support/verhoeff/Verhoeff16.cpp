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
 *      This file implements an object for Verhoeff's check-digit
 *      algorithm for base-16 strings.
 *
 */
#include "Verhoeff.h"

#include <stdint.h>
#include <string.h>

#ifndef VERHOEFF16_NO_MULTIPLY_TABLE

const uint8_t Verhoeff16::sMultiplyTable[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 1,  2,  3,  4,  5,  6,  7,  0,  9,  10, 11, 12, 13, 14, 15, 8,
    2,  3,  4,  5,  6,  7,  0,  1,  10, 11, 12, 13, 14, 15, 8,  9,  3,  4,  5,  6,  7,  0,  1,  2,  11, 12, 13, 14, 15, 8,  9,  10,
    4,  5,  6,  7,  0,  1,  2,  3,  12, 13, 14, 15, 8,  9,  10, 11, 5,  6,  7,  0,  1,  2,  3,  4,  13, 14, 15, 8,  9,  10, 11, 12,
    6,  7,  0,  1,  2,  3,  4,  5,  14, 15, 8,  9,  10, 11, 12, 13, 7,  0,  1,  2,  3,  4,  5,  6,  15, 8,  9,  10, 11, 12, 13, 14,
    8,  15, 14, 13, 12, 11, 10, 9,  0,  7,  6,  5,  4,  3,  2,  1,  9,  8,  15, 14, 13, 12, 11, 10, 1,  0,  7,  6,  5,  4,  3,  2,
    10, 9,  8,  15, 14, 13, 12, 11, 2,  1,  0,  7,  6,  5,  4,  3,  11, 10, 9,  8,  15, 14, 13, 12, 3,  2,  1,  0,  7,  6,  5,  4,
    12, 11, 10, 9,  8,  15, 14, 13, 4,  3,  2,  1,  0,  7,  6,  5,  13, 12, 11, 10, 9,  8,  15, 14, 5,  4,  3,  2,  1,  0,  7,  6,
    14, 13, 12, 11, 10, 9,  8,  15, 6,  5,  4,  3,  2,  1,  0,  7,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
};

#endif // VERHOEFF16_NO_MULTIPLY_TABLE

const uint8_t Verhoeff16::sPermTable[] = { 4, 7, 5, 14, 8, 12, 15, 0, 2, 11, 3, 13, 10, 6, 9, 1 };

char Verhoeff16::ComputeCheckChar(const char * str)
{
    return ComputeCheckChar(str, strlen(str));
}

char Verhoeff16::ComputeCheckChar(const char * str, size_t strLen)
{
    int c = 0;

    for (size_t i = 1; i <= strLen; i++)
    {
        char ch = str[strLen - i];

        int val = CharToVal(ch);
        if (val < 0)
            return 0; // invalid character

        int p = Verhoeff::Permute(val, sPermTable, Base, i);

#ifdef VERHOEFF16_NO_MULTIPLY_TABLE
        c = Verhoeff::DihedralMultiply(c, p, PolygonSize);
#else
        c = sMultiplyTable[c * Base + p];
#endif
    }

    c = Verhoeff::DihedralInvert(c, PolygonSize);

    return ValToChar(c);
}

bool Verhoeff16::ValidateCheckChar(char checkChar, const char * str)
{
    return ValidateCheckChar(checkChar, str, strlen(str));
}

bool Verhoeff16::ValidateCheckChar(char checkChar, const char * str, size_t strLen)
{
    return (ComputeCheckChar(str, strLen) == checkChar);
}

bool Verhoeff16::ValidateCheckChar(const char * str)
{
    return ValidateCheckChar(str, strlen(str));
}

bool Verhoeff16::ValidateCheckChar(const char * str, size_t strLen)
{
    if (strLen == 0)
        return false;
    return ValidateCheckChar(str[strLen - 1], str, strLen - 1);
}

int Verhoeff16::CharToVal(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';

    if (ch >= 'A' && ch <= 'F')
        return (ch - 'A') + 10;

    if (ch >= 'a' && ch <= 'f')
        return (ch - 'a') + 10;

    return -1;
}

char Verhoeff16::ValToChar(int val)
{
    if (val >= 0 && val < 10)
        return static_cast<char>('0' + val);

    if (val >= 10 && val < Base)
        return static_cast<char>('A' + (val - 10));

    return 0;
}
