#include "SetupCodeUtils.h"

#include <map>
#include <string>

using namespace std;

namespace chip
{

map <int, char> _getBase45CharacterMap()
{  
    map <int, char> characterMap;
    // Special characters
    characterMap[36] =  ' ';
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
    string result;
    int radix = 45;
    do {
        int remainder = input % radix;  
        char base45char;
        if (remainder >= 0 && remainder <= 9) {
            base45char = '0' + remainder;
        }
        else if (remainder >= 10 && remainder <= 35) {
            base45char = 'A' + (remainder - 10);
        }
        else {
            base45char = _getBase45CharacterMap()[remainder];
        }
        result += base45char;
        input = input / radix;
    } while (input != 0);

    while (result.length() < minLength ){
        result.append("0");
    }

    reverse(result.begin(), result.end());
    return result;
}

}