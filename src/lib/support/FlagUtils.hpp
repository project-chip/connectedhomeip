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
 *      This file defines functions for manipulating Boolean flags in
 *      a bitfield.
 *
 */

#ifndef CHIP_CHIP_SUPPORT_FLAGUTILS_HPP
#define CHIP_CHIP_SUPPORT_FLAGUTILS_HPP

#include <stdint.h>

namespace chip {

template <typename FlagsT, typename FlagT>
inline bool GetFlag(const FlagsT & inFlags, const FlagT inFlag)
{
    return (inFlags & static_cast<FlagsT>(inFlag)) != 0;
}

template <typename FlagsT, typename FlagT>
inline void ClearFlag(FlagsT & inFlags, const FlagT inFlag)
{
    inFlags &= ~static_cast<FlagsT>(inFlag);
}

template <typename FlagsT, typename FlagT>
inline void SetFlag(FlagsT & inFlags, const FlagT inFlag)
{
    inFlags |= static_cast<FlagsT>(inFlag);
}

template <typename FlagsT, typename FlagT>
inline void SetFlag(FlagsT & inFlags, const FlagT inFlag, const bool inValue)
{
    if (inValue)
    {
        SetFlag(inFlags, inFlag);
    }
    else
    {
        ClearFlag(inFlags, inFlag);
    }
}

} // namespace chip

#endif // CHIP_CHIP_SUPPORT_FLAGUTILS_HPP
