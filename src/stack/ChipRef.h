/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stack/Stack.h>

namespace chip {

/*
 * @brief
 *   Base class for any objects which is holding a reference to chip stack.
 *
 *   When CHIP_CONFIG_CHIP_IS_SINGLETON is true, this class has no overhead.
 */
class ChipRef {
public:
    Init(ChipRef & ref) {
#if !CHIP_CONFIG_CHIP_IS_SINGLETON
        mChip = ref.mChip;
#endif // CHIP_CONFIG_CHIP_IS_SINGLETON
    }

#if CHIP_CONFIG_CHIP_IS_SINGLETON
    // When chip is singleton, the application must implement this function to return a chip stack.
    static Stack & GetChipStack();
#else
    Stack & GetChipStack() { return mChip; }
#endif // CHIP_CONFIG_CHIP_IS_SINGLETON

private:
#if !CHIP_CONFIG_CHIP_IS_SINGLETON
    Stack * mChip;
#endif // CHIP_CONFIG_CHIP_IS_SINGLETON
}

} // namespace chip
