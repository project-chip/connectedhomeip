/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the chip::Optional class to handle values which may
 *      or may not be present.
 *
 */
#pragma once

namespace chip {

/// InPlace is disambiguation tags that can be passed to the constructors to indicate that the contained object should be
/// constructed in-place
struct InPlaceType
{
    explicit InPlaceType() = default;
};
constexpr InPlaceType InPlace{};

template <class T>
struct InPlaceTemplateType
{
    explicit InPlaceTemplateType() = default;
};
template <class T>
constexpr InPlaceTemplateType<T> InPlaceTemplate{};

} // namespace chip
