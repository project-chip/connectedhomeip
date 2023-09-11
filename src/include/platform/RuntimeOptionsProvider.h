/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

namespace chip {
namespace app {
/**
 * @brief This class provides a mechanism for clusters to access runtime options set for the app.
 */
class RuntimeOptionsProvider
{
public:
    static RuntimeOptionsProvider & Instance();
    void SetSimulateNoInternalTime(bool simulateNoInternalTime) { mSimulateNoInternalTime = simulateNoInternalTime; }
    bool GetSimulateNoInternalTime() { return mSimulateNoInternalTime; }

private:
    bool mSimulateNoInternalTime = false;
};
} // namespace app
} // namespace chip
