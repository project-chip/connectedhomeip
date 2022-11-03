/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

class OTAInitializer
{
public:
    static OTAInitializer & Instance(void)
    {
        static OTAInitializer sInitOTA;
        return sInitOTA;
    }
    void InitOTARequestor(void);
};
