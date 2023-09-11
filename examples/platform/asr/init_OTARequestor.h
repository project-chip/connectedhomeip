/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "stdint.h"

class OTAInitializer
{
public:
    static OTAInitializer & Instance(void)
    {
        static OTAInitializer sInitOTA;
        return sInitOTA;
    }
    void InitOTARequestor(void);
    void ReloadQueryTimeout(uint32_t timeout);
};
