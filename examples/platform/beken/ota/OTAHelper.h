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
