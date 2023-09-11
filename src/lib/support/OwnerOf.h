/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace chip {

template <class ClassType, class MemberType>
constexpr ClassType * OwnerOf(MemberType * ptr, const MemberType ClassType::*member)
{
    return reinterpret_cast<ClassType *>(reinterpret_cast<uintptr_t>(ptr) - offsetof(ClassType, member));
}

} // namespace chip
