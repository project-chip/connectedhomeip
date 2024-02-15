/*
 *    Copyright (c) 2024 Project CHIP Authors
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

/**
 * @brief The file implements the Matter Check-In counter
 *
 */
#pragma once

#include <crypto/RandUtils.h>
#include <lib/support/PersistedCounter.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

/**
 * @brief
 *
 */
class CheckInCounter : public PersistedCounter<uint32_t>
{
public:
    CheckInCounter(){};
    ~CheckInCounter() override{};

    /**
     * @brief Returns the next Check-In counter Value to use
     *
     * @return uint32_t Check-in Counter value to use
     */
    inline uint32_t GetNextCheckInCounterValue() { return (mCounterValue + 1); };

private:
    /**
     * @brief Get the Initial Counter Value for the Check-In counter
     *        Initial value is randomized for the Check-In counter
     *
     * @return uint32_t random value
     */
    inline uint32_t GetInitialCounterValue() override { return chip::Crypto::GetRandU32(); }
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
