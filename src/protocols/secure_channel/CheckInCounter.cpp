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
 */

#include <protocols/secure_channel/CheckInCounter.h>
#include <stdint.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

CHIP_ERROR CheckInCounter::InvalidateHalfCheckInCounterValues()
{
    // Increases the current counter value by half of its maximum range and updates underlying counter storage.
    // CheckInCounter is allowed to roll over.
    ReturnErrorOnFailure(AdvanceBy((UINT32_MAX / 2)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CheckInCounter::InvalidateAllCheckInCounterValues()
{
    // Increase current counter value by its maximum value to mimic N -> UINT32_MAX -> N-1 to invalidate all Check-In counter
    // values. CheckInCounter is allowed to roll over.
    ReturnErrorOnFailure(AdvanceBy(UINT32_MAX));
    return CHIP_NO_ERROR;
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
