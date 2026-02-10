/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstdint>

namespace chip::app::reporting {

// u32 generations can generally wrap around. We try to do a best effort figuring out
// if First < Second considering wrap-around. So we cast their difference to i32 and
// see if that is positive.
inline constexpr bool AreGenerationsInOrder(uint32_t generationFirst, uint32_t generationSecond)
{
    return static_cast<int32_t>(generationSecond - generationFirst) > 0;
}

// essentially our unit tests
static_assert(AreGenerationsInOrder(1, 100));
static_assert(!AreGenerationsInOrder(100, 1));

static_assert(AreGenerationsInOrder(0xFFFFFFAB, 120));
static_assert(!AreGenerationsInOrder(120, 0xFFFFFFAB));

static_assert(AreGenerationsInOrder(1, 0x7FFFFFFF));
static_assert(!AreGenerationsInOrder(0x7FFFFFFF, 1));

// random increases from small numbers
static_assert(AreGenerationsInOrder(2, 0x80000000));
static_assert(AreGenerationsInOrder(100, 0x80000000));
static_assert(AreGenerationsInOrder(1000, 0x800000AB));

// wrap-arounds
static_assert(AreGenerationsInOrder(0x80000000 + 1000, 900));
static_assert(AreGenerationsInOrder(0x80000000 + 0x12345, 0x12344));

/// Represents a generation of an attribute. A wrapper of a u32 that does NOT auto-cast back to U32, so that we can
/// force comparisons to use the wrap-around aware compares
class AttributeGeneration
{
public:
    explicit AttributeGeneration(uint32_t value) : mValue(value) {}

    AttributeGeneration()                            = default;
    AttributeGeneration(const AttributeGeneration &) = default;
    AttributeGeneration(AttributeGeneration &&)      = default;

    AttributeGeneration & operator=(const AttributeGeneration &) = default;
    AttributeGeneration & operator=(AttributeGeneration &&)      = default;

    constexpr bool Before(const AttributeGeneration & other) const { return AreGenerationsInOrder(mValue, other.mValue); }
    constexpr bool After(const AttributeGeneration & other) const { return AreGenerationsInOrder(other.mValue, mValue); }

    // zero is a special marker, generally may be used as "not defined"
    constexpr bool IsZero() const { return mValue == 0; }

    // reset to zero value (since zero is used as a special/uninitialized marker)
    void Clear() { mValue = 0; }

    uint32_t Raw() const { return mValue; }

    // increment, guarantees 0 is NOT used as a value when incrementing and wrapping around
    void Increment()
    {
        mValue++;
        if (mValue == 0)
        {
            mValue = 1;
        }
    }

private:
    uint32_t mValue{};
};

} // namespace chip::app::reporting
