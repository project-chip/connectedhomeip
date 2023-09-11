/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace chip {
namespace app {

template <typename T>
struct ObjectList
{
    T mValue;
    ObjectList * mpNext = nullptr;

    size_t Count() const
    {
        size_t ret = 0;
        for (auto p = this; p != nullptr; p = p->mpNext)
        {
            ret++;
        }
        return ret;
    }
};

} // namespace app
} // namespace chip
