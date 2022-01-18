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
 *      algorithm for base-32 strings.
 *
 */

#include "Verhoeff.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef VERHOEFF32_NO_MULTIPLY_TABLE

const uint8_t Verhoeff32::sMultiplyTable[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16,
    2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17,
    3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18,
    4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19,
    5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20,
    6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  23, 24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22,
    8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23,
    9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
    12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
    13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    14, 15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    15, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 31, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 0,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,
    17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 1,  0,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,
    18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 2,  1,  0,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,
    19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 3,  2,  1,  0,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,
    20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 4,  3,  2,  1,  0,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,
    21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 5,  4,  3,  2,  1,  0,  15, 14, 13, 12, 11, 10, 9,  8,  7,  6,
    22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 23, 6,  5,  4,  3,  2,  1,  0,  15, 14, 13, 12, 11, 10, 9,  8,  7,
    23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 24, 7,  6,  5,  4,  3,  2,  1,  0,  15, 14, 13, 12, 11, 10, 9,  8,
    24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 25, 8,  7,  6,  5,  4,  3,  2,  1,  0,  15, 14, 13, 12, 11, 10, 9,
    25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 26, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  15, 14, 13, 12, 11, 10,
    26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 27, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  15, 14, 13, 12, 11,
    27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 28, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  15, 14, 13, 12,
    28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 29, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  15, 14, 13,
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 30, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  15, 14,
    30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 31, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  15,
    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
};

#endif // VERHOEFF32_NO_MULTIPLY_TABLE

const uint8_t Verhoeff32::sPermTable[] = {
    // Detects all single digit and adjacent transposition errors, and 97.076613% of jump transposition errors.
    7, 2, 1, 30, 16, 20, 27, 11, 31, 6, 8, 13, 29, 5, 10, 21, 22, 3, 24, 0, 23, 25, 12, 9, 28, 14, 4, 15, 17, 18, 19, 26,
};

const int8_t Verhoeff32::sCharToValTable[] = {
    // NOTE: table starts at ASCII 30h
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17,
    -1, 18, 19, 20, 21, 22, -1, 23, -1, 24, 25, 26, 27, 28, 29, 30, 31, -1, -1, -1, -1, -1, -1, -1, 10,
    11, 12, 13, 14, 15, 16, 17, -1, 18, 19, 20, 21, 22, -1, 23, -1, 24, 25, 26, 27, 28, 29, 30, 31
};

const char Verhoeff32::sValToCharTable[] = "0123456789ABCDEFGHJKLMNPRSTUVWXY";

char Verhoeff32::ComputeCheckChar(const char * str)
{
    return ComputeCheckChar(str, strlen(str));
}

char Verhoeff32::ComputeCheckChar(const char * str, size_t strLen)
{
    int c = 0;

    for (size_t i = 1; i <= strLen; i++)
    {
        char ch = str[strLen - i];

        int val = CharToVal(ch);
        if (val < 0)
            return 0; // invalid character

        int p = Verhoeff::Permute(val, sPermTable, Base, i);

#ifdef VERHOEFF32_NO_MULTIPLY_TABLE
        c = Verhoeff::DihedralMultiply(c, p, PolygonSize);
#else
        c = sMultiplyTable[c * Base + p];
#endif
    }

    c = Verhoeff::DihedralInvert(c, PolygonSize);

    return ValToChar(c);
}

bool Verhoeff32::ValidateCheckChar(char checkChar, const char * str)
{
    return ValidateCheckChar(checkChar, str, strlen(str));
}

bool Verhoeff32::ValidateCheckChar(char checkChar, const char * str, size_t strLen)
{
    return (ComputeCheckChar(str, strLen) == checkChar);
}

bool Verhoeff32::ValidateCheckChar(const char * str)
{
    return ValidateCheckChar(str, strlen(str));
}

bool Verhoeff32::ValidateCheckChar(const char * str, size_t strLen)
{
    if (strLen == 0)
        return false;
    return ValidateCheckChar(str[strLen - 1], str, strLen - 1);
}

int Verhoeff32::CharToVal(char ch)
{
    if (ch >= '0' && ch <= 'y')
        return sCharToValTable[static_cast<int>(ch) - '0'];
    return -1;
}

char Verhoeff32::ValToChar(int val)
{
    if (val >= 0 && val < Base)
        return sValToCharTable[val];
    return 0;
}
