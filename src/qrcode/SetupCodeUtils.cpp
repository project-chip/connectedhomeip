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

#include <map>
#include <string>

using namespace std;

namespace chip {

map <int, char> base45CharacterMap()
{
    map <int, char> characterMap;
    // Special characters
    characterMap[36] = ' ';
    characterMap[37] = '$';
    characterMap[38] = '%';
    characterMap[39] = '*';
    characterMap[40] = '+';
    characterMap[41] = '-';
    characterMap[42] = '.';
    characterMap[43] = '/';
    characterMap[44] = ':';

    return characterMap;
}

string base45EncodedString(uint64_t input, size_t minLength)
{
    static std::map<int, char> BS45_CHARS = base45CharacterMap();
    string result;
    int radix = 45;
    do {
        int remainder = input % radix;
        char base45char;
        if (remainder >= 0 && remainder <= 9) {
            // Numbers
            base45char = '0' + remainder;
        }
        else if (remainder >= 10 && remainder <= 35) {
            // Uppercase Characters
            base45char = 'A' + (remainder - 10);
        }
        else {
            // Special Characters
            base45char = BS45_CHARS[remainder];
        }
        result += base45char;
        input = input / radix;
    } while (input != 0);

    while (result.length() < minLength) {
        result.append("0");
    }

    reverse(result.begin(), result.end());
    return result;
}

} // namespace chip
