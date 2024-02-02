/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/TestEventTriggerDelegate.h>

namespace chip {

class OTATestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    static constexpr uint64_t kOtaQueryTrigger         = 0x002a'0000'0000'0100;
    static constexpr uint64_t kOtaQueryFabricIndexMask = 0xff;

    OTATestEventTriggerHandler() {}

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;
};

} // namespace chip
