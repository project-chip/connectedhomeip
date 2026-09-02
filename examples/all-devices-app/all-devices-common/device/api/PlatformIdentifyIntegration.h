/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * Platform-provided defaults for code-driven `IdentifyCluster` instances
 * created by devices in the `all-devices-app` reference application.
 *
 * An instance of this class is owned by the application entry point
 * (typically the platform's `main` or `AppTask`) and is passed explicitly
 * into the device factory / device constructors that require it. There
 * is no global singleton: the owner constructs and configures the
 * instance and then registers it with `DeviceFactory::Context` so that
 * every device receives the same platform integration.
 *
 * Platforms (e.g. `silabs/`, `esp32/`) may install a platform identify
 * delegate at boot (typically driving a status LED) and advertise the
 * appropriate `Identify::IdentifyTypeEnum` (e.g. `kVisibleIndicator`).
 * Device types pick these up when they Create() their `IdentifyCluster`
 * via `MakeConfig(...)`, without every device constructor having to
 * accept a delegate directly.
 *
 * Defaults (nullptr delegate, `kNone` type) preserve prior behavior for
 * platforms that do not install anything.
 *
 * Usage inside a device `Register()` (with a reference stored as
 * `mPlatformIdentify` on the device):
 * @code
 *   mIdentifyCluster.Create(mPlatformIdentify.MakeConfig(endpoint, mTimerDelegate));
 * @endcode
 *
 * Devices that own their own `IdentifyDelegate` (e.g. `LoggingOnOffLoad`)
 * keep it wired via `IdentifyCluster::Config::WithDelegate(...)` for
 * cluster-level callbacks and additionally notify the platform via
 * `NotifyIdentifyStart/Stop/TriggerEffect(...)` so the platform LED
 * indicator still runs on those endpoints.
 */
class PlatformIdentifyIntegration
{
public:
    PlatformIdentifyIntegration() = default;

    void SetDelegate(Clusters::IdentifyDelegate * delegate) { mPlatformDelegate = delegate; }
    Clusters::IdentifyDelegate * GetDelegate() const { return mPlatformDelegate; }

    void SetIdentifyType(Clusters::Identify::IdentifyTypeEnum type) { mIdentifyType = type; }
    Clusters::Identify::IdentifyTypeEnum GetIdentifyType() const { return mIdentifyType; }

    /**
     * Build an `IdentifyCluster::Config` seeded with the platform's identify
     * type and delegate. Devices without a per-endpoint delegate should call
     * this directly. Devices with a per-endpoint delegate should
     * `.WithDelegate(...)` after this call and additionally use the Notify*
     * helpers to fan out to the platform.
     */
    Clusters::IdentifyCluster::Config MakeConfig(EndpointId endpoint, TimerDelegate & timerDelegate) const;

    /// Forward cluster-level identify events to the installed platform
    /// delegate. Safe to call when no platform delegate is installed (no-op).
    void NotifyIdentifyStart(Clusters::IdentifyCluster & cluster) const;
    void NotifyIdentifyStop(Clusters::IdentifyCluster & cluster) const;
    void NotifyTriggerEffect(Clusters::IdentifyCluster & cluster) const;

private:
    Clusters::IdentifyDelegate * mPlatformDelegate     = nullptr;
    Clusters::Identify::IdentifyTypeEnum mIdentifyType = Clusters::Identify::IdentifyTypeEnum::kNone;
};

} // namespace app
} // namespace chip
