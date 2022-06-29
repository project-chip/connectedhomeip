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
#pragma once

#include <stdint.h>
class OTAHelpers
{
public:
    static OTAHelpers & Instance(void)
    {
        static OTAHelpers sInitOTA;
        return sInitOTA;
    }
    static constexpr uint32_t kInitOTARequestorDelaySec = 3;
    void InitOTARequestor(void);
};

namespace chip {
namespace Shell {

class OTARequestorCommands
{
public:
    // delete the copy constructor
    OTARequestorCommands(const OTARequestorCommands &) = delete;
    // delete the move constructor
    OTARequestorCommands(OTARequestorCommands &&) = delete;
    // delete the assignment operator
    OTARequestorCommands & operator=(const OTARequestorCommands &) = delete;

    static OTARequestorCommands & GetInstance()
    {
        static OTARequestorCommands instance;
        return instance;
    }

    // Register the OTA requestor commands
    void Register();

private:
    OTARequestorCommands() {}
};

} // namespace Shell
} // namespace chip
