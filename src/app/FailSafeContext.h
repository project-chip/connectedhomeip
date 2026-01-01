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

/**
 *    @file
 *          A 'Fail Safe Context' SHALL be created on the receiver, to track fail-safe
 *          state information while the fail-safe is armed.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

class FailSafeContext
{
public:
    // ===== Members for internal use by other Device Layer components.

    /**
     * @brief
     *  Only a single fail-safe timer is started on the device, if this function is called again
     *  when the fail-safe timer is currently armed, the currently-running fail-safe timer will
     *  first be cancelled, then the fail-safe timer will be re-armed.
     */
    CHIP_ERROR ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Seconds16 expiryLengthSeconds);

    /**
     * @brief Cleanly disarm failsafe timer, such as on CommissioningComplete
     */
    void DisarmFailSafe();
    void SetAddNocCommandInvoked(FabricIndex nocFabricIndex)
    {
        mContextFlags.Set(ContextFlags::kAddNocCommandInvoked);
        mFabricIndex = nocFabricIndex;
    }
    void SetUpdateNocCommandInvoked() { mContextFlags.Set(ContextFlags::kUpdateNocCommandInvoked); }
    void SetAddTrustedRootCertInvoked() { mContextFlags.Set(ContextFlags::kAddTrustedRootCertInvoked); }
    void SetCsrRequestForUpdateNoc(bool isForUpdateNoc)
    {
        mContextFlags.Set(ContextFlags::kIsCsrRequestForUpdateNoc, isForUpdateNoc);
    }
    void SetUpdateTermsAndConditionsHasBeenInvoked() { mContextFlags.Set(ContextFlags::kUpdateTermsAndConditionsInvoked); }
    void RecordSetVidVerificationStatementHasBeenInvoked() { mContextFlags.Set(ContextFlags::kSetVidVerificationStatementInvoked); }
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    void SetAddICACHasBeenInvoked() { mContextFlags.Set(ContextFlags::kAddICACInvoked); }
#endif

    /**
     * @brief
     *   Schedules a work to cleanup the FailSafe Context asynchronously after various cleanup work
     *   has completed.
     */
    void ScheduleFailSafeCleanup(FabricIndex fabricIndex, bool addNocCommandInvoked, bool updateNocCommandInvoked);

    bool IsFailSafeArmed(FabricIndex accessingFabricIndex) const
    {
        return IsFailSafeArmed() && MatchesFabricIndex(accessingFabricIndex);
    }

    // Returns true if the fail-safe is in a state where commands that require an armed
    // fail-safe can no longer execute, but a new fail-safe can't be armed yet.
    bool IsFailSafeBusy() const { return mFailSafeBusy; }

    bool IsFailSafeArmed() const { return mFailSafeArmed; }

    // True if it is possible to do an initial arming of the failsafe if needed.
    // To be used in places where some action should take place only if the
    // fail-safe could be armed after that action.
    bool IsFailSafeFullyDisarmed() const { return !IsFailSafeArmed() && !IsFailSafeBusy(); }

    bool MatchesFabricIndex(FabricIndex accessingFabricIndex) const
    {
        VerifyOrDie(IsFailSafeArmed());
        return (accessingFabricIndex == mFabricIndex);
    }

    bool NocCommandHasBeenInvoked() const { return AddNocCommandHasBeenInvoked() || UpdateNocCommandHasBeenInvoked(); }
    bool AddNocCommandHasBeenInvoked() const { return mContextFlags.Has(ContextFlags::kAddNocCommandInvoked); }
    bool UpdateNocCommandHasBeenInvoked() const { return mContextFlags.Has(ContextFlags::kUpdateNocCommandInvoked); }
    bool AddTrustedRootCertHasBeenInvoked() const { return mContextFlags.Has(ContextFlags::kAddTrustedRootCertInvoked); }
    bool IsCsrRequestForUpdateNoc() const { return mContextFlags.Has(ContextFlags::kIsCsrRequestForUpdateNoc); }
    bool UpdateTermsAndConditionsHasBeenInvoked() const
    {
        return mContextFlags.Has(ContextFlags::kUpdateTermsAndConditionsInvoked);
    }
    bool HasSetVidVerificationStatementHasBeenInvoked() const
    {
        return mContextFlags.Has(ContextFlags::kSetVidVerificationStatementInvoked);
    }
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    bool AddICACCommandHasBeenInvoked() const { return mContextFlags.Has(ContextFlags::kAddICACInvoked); }
#endif

    FabricIndex GetFabricIndex() const
    {
        VerifyOrDie(IsFailSafeArmed());
        return mFabricIndex;
    }

    // Immediately disarms the timer and schedules a failsafe timer expiry.
    // If the failsafe is not armed, this is a no-op.
    void ForceFailSafeTimerExpiry();

private:
    enum class ContextFlags : uint8_t
    {
        kAddNocCommandInvoked               = 0x01,
        kUpdateNocCommandInvoked            = 0x02,
        kAddTrustedRootCertInvoked          = 0x04,
        kIsCsrRequestForUpdateNoc           = 0x08, /* The fact of whether a CSR occurred at all is stored elsewhere. */
        kUpdateTermsAndConditionsInvoked    = 0x10,
        kSetVidVerificationStatementInvoked = 0x20,
#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
        kAddICACInvoked = 0x40,
#endif
    };

    BitFlags<ContextFlags> mContextFlags;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;

    bool mFailSafeArmed = false;
    bool mFailSafeBusy  = false;

    /**
     * @brief
     *  The callback function to be called when "fail-safe timer" expires.
     */
    static void HandleArmFailSafeTimer(System::Layer * layer, void * aAppState);

    /**
     * @brief
     *  The callback function to be called when max cumulative time expires.
     */
    static void HandleMaxCumulativeFailSafeTimer(System::Layer * layer, void * aAppState);

    /**
     * @brief
     *  The callback function to be called asynchronously after various cleanup work has completed
     *  to actually disarm the fail-safe.
     */
    static void HandleDisarmFailSafe(intptr_t arg);

    void SetFailSafeArmed(bool armed);

    /**
     * @brief Reset to unarmed basic state
     */
    void ResetState()
    {
        SetFailSafeArmed(false);

        mFailSafeBusy = false;
        mContextFlags.ClearAll();
    }

    void FailSafeTimerExpired();
};

} // namespace app
} // namespace chip
