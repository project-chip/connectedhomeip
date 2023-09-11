/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace chip {
namespace Access {

// Using bitfield values so privilege set and auth mode can be stored together.
// Privilege set can have more than one value expressed (e.g. View,
// ProxyView, and Operate).
enum class Privilege : uint8_t
{
    kView       = 1 << 0,
    kProxyView  = 1 << 1,
    kOperate    = 1 << 2,
    kManage     = 1 << 3,
    kAdminister = 1 << 4
};

} // namespace Access
} // namespace chip
