/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#ifndef ICD_SLEEP_TIME_JITTER_MS
#define ICD_SLEEP_TIME_JITTER_MS (CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL * 0.75)
#endif

#ifndef ICD_ACTIVE_TIME_JITTER_MS
#define ICD_ACTIVE_TIME_JITTER_MS 300
#endif

namespace chip {
namespace app {

class ICDStateObserver
{
public:
    virtual ~ICDStateObserver() {}
    virtual void OnEnterActiveMode()  = 0;
    virtual void OnTransitionToIdle() = 0;
};

} // namespace app
} // namespace chip
