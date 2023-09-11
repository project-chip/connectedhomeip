/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace chip {

/**
 * Enum used to control iteration (e.g. via a callback function that is called
 * for each of a set of elements).
 *
 * When used as the callback return type:
 *   Continue: Continue the iteration.
 *   Break: Stop the iteration.
 *
 * When used as the return type of the entire iteration procedure:
 *   Break: Some callback returned Break.
 *   Finish: All callbacks returned Continue.
 */
enum class Loop : uint8_t
{
    Continue,
    Break,
    Finish,
};

} // namespace chip
