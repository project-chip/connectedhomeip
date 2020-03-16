/*
 *
 *    <COPYRIGHT>
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
 *      This file implements converting an input into a Base45 String
 *
 */

#include "SetupCodeUtils.h"

#include <string>

using namespace std;

namespace chip {

char base45CharacterSet[45] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
                                'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%', '*', '+', '-', '.', '/', ':' };

string base45EncodedString(uint64_t input, size_t minLength)
{
    string result;
    int radix = 45;
    do
    {
        char base45char = base45CharacterSet[input % radix];
        result += base45char;
        input = input / radix;
    } while (input != 0);

    while (result.length() < minLength)
    {
        result.append("0");
    }

    reverse(result.begin(), result.end());
    return result;
}

} // namespace chip
