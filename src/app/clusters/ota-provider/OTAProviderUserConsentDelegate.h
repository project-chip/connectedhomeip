/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <app/OTAUserConsentCommon.h>

namespace chip {
namespace ota {

class OTAProviderUserConsentDelegate
{
public:
    virtual ~OTAProviderUserConsentDelegate() = default;

    virtual UserConsentState GetUserConsentState(const UserConsentSubject & subject) = 0;

    const char * UserConsentStateToString(UserConsentState state)
    {
        switch (state)
        {
        case kGranted:
            return "Granted";
        case kObtaining:
            return "Obtaining";
        case kDenied:
            return "Denied";
        default:
            return "Unknown";
        }
    }
};

} // namespace ota
} // namespace chip
