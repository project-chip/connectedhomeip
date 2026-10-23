/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPError.h>

// Note: We can use a constructor here to avoid "ToPyChipError", but it will result in compilers warning for incompatible with C
// linkage.
struct PyChipError
{
    uint32_t mCode;
    uint32_t mLine;
    const char * mFile;
};

// To avoid the error message "user-defined type 'PyChipError' which is incompatible with C"

inline bool operator!(const PyChipError & aError)
{
    return aError.mCode != 0;
}

inline bool operator==(const PyChipError & aError, const CHIP_ERROR & aOther)
{
    return aError.mCode == aOther.AsInteger();
}

inline bool operator==(const CHIP_ERROR & aOther, const PyChipError & aError)
{
    return aError.mCode == aOther.AsInteger();
}

inline bool operator!=(const PyChipError & aError, const CHIP_ERROR & aOther)
{
    return !(aError.mCode == aOther.AsInteger());
}

inline bool operator!=(const CHIP_ERROR & aOther, const PyChipError & aError)
{
    return !(aError.mCode == aOther.AsInteger());
}

inline PyChipError ToPyChipError(const CHIP_ERROR & aError)
{
    return PyChipError
    {
        .mCode = aError.AsInteger(),
#if CHIP_CONFIG_ERROR_SOURCE
        .mLine = aError.GetLine(), .mFile = aError.GetFile(),
#else
        .mLine = 0, .mFile = nullptr,
#endif
    };
}

// A version of ReturnErrorOnFailure which can use PyChipError as expr
// Note: we should consider implement operator == for CHIP_ERROR then we can overload it easily.
#define PyReturnErrorOnFailure(expr)                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

static_assert(std::is_same<decltype(PyChipError::mCode), chip::ChipError::StorageType>::value,
              "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {
void pychip_FormatError(PyChipError * apError, char * apBuf, uint32_t aBufSize);
}
