/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
