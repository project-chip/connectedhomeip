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
 *      algorithm for base-10 strings.
 *
 */

#include "Verhoeff.h"

#include <stdint.h>
#include <string.h>

const uint8_t Verhoeff10::sMultiplyTable[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 0, 6, 7, 8, 9, 5, 2, 3, 4, 0, 1, 7, 8, 9, 5, 6, 3, 4, 0, 1,
    2, 8, 9, 5, 6, 7, 4, 0, 1, 2, 3, 9, 5, 6, 7, 8, 5, 9, 8, 7, 6, 0, 4, 3, 2, 1, 6, 5, 9, 8, 7, 1, 0, 4,
    3, 2, 7, 6, 5, 9, 8, 2, 1, 0, 4, 3, 8, 7, 6, 5, 9, 3, 2, 1, 0, 4, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
};

const uint8_t Verhoeff10::sPermTable[] = { 1, 5, 7, 6, 2, 8, 3, 0, 9, 4 };

char Verhoeff10::ComputeCheckChar(const char * str)
{
    return ComputeCheckChar(str, strlen(str));
}

char Verhoeff10::ComputeCheckChar(const char * str, size_t strLen)
{
    int c = 0;

    for (size_t i = 1; i <= strLen; i++)
    {
        char ch = str[strLen - i];

        int val = CharToVal(ch);
        if (val < 0)
            return 0; // invalid character

        int p = Verhoeff::Permute(val, sPermTable, Base, i);

        c = sMultiplyTable[c * Base + p];
    }

    c = Verhoeff::DihedralInvert(c, PolygonSize);

    return ValToChar(c);
}

bool Verhoeff10::ValidateCheckChar(char checkChar, const char * str)
{
    return ValidateCheckChar(checkChar, str, strlen(str));
}

bool Verhoeff10::ValidateCheckChar(char checkChar, const char * str, size_t strLen)
{
    return (ComputeCheckChar(str, strLen) == checkChar);
}

bool Verhoeff10::ValidateCheckChar(const char * str)
{
    return ValidateCheckChar(str, strlen(str));
}

bool Verhoeff10::ValidateCheckChar(const char * str, size_t strLen)
{
    if (strLen == 0)
        return false;
    return ValidateCheckChar(str[strLen - 1], str, strLen - 1);
}

int Verhoeff10::CharToVal(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    return -1;
}

char Verhoeff10::ValToChar(int val)
{
    if (val >= 0 && val <= Base)
        return static_cast<char>('0' + val);
    return 0;
}
