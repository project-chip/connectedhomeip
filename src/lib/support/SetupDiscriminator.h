/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file defines the SetupDiscriminator type, which is used by
 *      low-level code (e.g. BLE) in addition to setup payload code.
 */

#pragma once

#include <cstdint>

#include <lib/support/CodeUtils.h>

namespace chip {

class SetupDiscriminator
{
public:
    constexpr SetupDiscriminator() : mDiscriminator(0), mIsShortDiscriminator(false) {}

    // See section 5.1.2. QR Code in the Matter specification
    static constexpr int kLongBits = 12;

    // See section 5.1.3. Manual Pairing Code in the Matter specification
    static constexpr int kShortBits = 4;

    void SetShortValue(uint8_t discriminator)
    {
        VerifyOrDie(discriminator == (discriminator & kShortMask));
        mDiscriminator        = (discriminator & kShortMask);
        mIsShortDiscriminator = true;
    }

    void SetLongValue(uint16_t discriminator)
    {
        VerifyOrDie(discriminator == (discriminator & kLongMask));
        mDiscriminator        = (discriminator & kLongMask);
        mIsShortDiscriminator = false;
    }

    bool IsShortDiscriminator() const { return mIsShortDiscriminator; }

    uint8_t GetShortValue() const
    {
        if (IsShortDiscriminator())
        {
            return static_cast<uint8_t>(mDiscriminator);
        }

        return LongToShortValue(mDiscriminator);
    }

    uint16_t GetLongValue() const
    {
        VerifyOrDie(!IsShortDiscriminator());
        return mDiscriminator;
    }

    bool MatchesLongDiscriminator(uint16_t discriminator) const
    {
        if (!IsShortDiscriminator())
        {
            return mDiscriminator == discriminator;
        }

        return mDiscriminator == LongToShortValue(discriminator);
    }

    bool operator==(const SetupDiscriminator & other) const
    {
        return mIsShortDiscriminator == other.mIsShortDiscriminator && mDiscriminator == other.mDiscriminator;
    }

private:
    static constexpr uint16_t kLongMask = (1 << kLongBits) - 1;
    static constexpr uint8_t kShortMask = (1 << kShortBits) - 1;

    static uint8_t LongToShortValue(uint16_t longValue)
    {
        // Short value consists of the high bits of the long value.
        constexpr int kLongToShortShift = kLongBits - kShortBits;
        return static_cast<uint8_t>(longValue >> kLongToShortShift);
    }

    // If long discriminator, all 12 bits are used.  If short discriminator,
    // only the low kShortBits bits are used, to store the value of the short
    // discriminator (which contains only the high bits of the complete 12-bit
    // discriminator).
    static_assert(kLongBits == 12, "Unexpected field length");
    static_assert(kShortBits <= kLongBits, "Unexpected field length");
    uint16_t mDiscriminator;
    bool mIsShortDiscriminator;
};

} // namespace chip
