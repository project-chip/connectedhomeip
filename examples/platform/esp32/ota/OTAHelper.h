/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
