/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/clusters/window-covering-server/window-covering-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

class WindowCoveringManager : public Delegate
{
public:
    void Init(chip::EndpointId endpoint);
    CHIP_ERROR HandleMovement(WindowCoveringType type) override;
    CHIP_ERROR HandleStopMotion() override;

private:
    OperationalState mState;
    NPercent100ths mCurrentLiftPosition;
    NPercent100ths mTargetLiftPosition;
    NPercent100ths mCurrentTiltPosition;
    NPercent100ths mTargetTiltPosition;

    /**
     * @brief
     *  The callback function to be called when "movement timer" expires.
     */
    static void HandleLiftMovementTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleTiltMovementTimer(chip::System::Layer * layer, void * aAppState);
};

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
