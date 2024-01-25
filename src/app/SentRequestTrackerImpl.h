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

#pragma once

#include <unordered_set>

#include "SentRequestTracker.h"

namespace chip {
namespace app {

/**
 * @brief An implementation of SentRequestTracker.
 */
class SentRequestTrackerImpl : public SentRequestTracker
{
public:
    CHIP_ERROR AddCommand(uint16_t aCommandRef) override;
    CHIP_ERROR RemoveCommand(uint16_t aCommandRef) override;
    bool IsCommandTracked(uint16_t aCommandRef) override;
    size_t Count() override;
    Optional<uint16_t> PopCommand() override;
private:
    std::unordered_set<uint16_t> mCommandReferenceSet;
};

} // namespace app
} // namespace chip