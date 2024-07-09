/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <functional>
#include <stdbool.h>

#include <lib/support/BitFlags.h>
#include <app/data-model/Nullable.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

// - If it has changed due to a change in the CurrentPhase or OperationalState attributes, or
// - When it changes from 0 to any other value and vice versa, or
// - When it changes from null to any other value and vice versa, or
// - When it increases, or
// - When there is any increase or decrease in the estimated time remaining that was due to progressing insight of the server's control logic, or
// - When it changes at a rate significantly different from one unit per second.

enum class QuieterReportingPolicyEnum
{
    kMarkDirtyOnChangeToFromZero = (1u << 0),
    kMarkDirtyOnDecrement = (1u << 1),
    kMarkDirtyOnIncrement = (1u << 2),
};

using QuieterReportingPolicyFlags = ::chip::BitFlags<QuieterReportingPolicyEnum>;

namespace detail {

using Timestamp = chip::System::Clock::Milliseconds64;
template <typename T>
using Nullable = chip::app::DataModel::Nullable<T>;

/**
 * This class helps track reporting state of an attribute to properly keep track of whether
 * it needs to be marked as dirty or not for purposes of reporting using Q quality.
 *
 * The class can be configured via `SetPolicy` to have some/all of the common reasons
 * for reporting (e.g. increment only, decrement only, change to/from zero).
 *
 * Changes of null to non-null or non-null to null are always considered dirty.
 *
 * It is possible to force mark the attribute as dirty (see `ForceMarkAsDirty`) such as
 * for conditions like "When there is any increase or decrease in the estimated time
 * remaining that was due to progressing insight of the server's control logic".
 *
 * Usage is simple:
 *
 * - Call `SetValue()` with the new value and current monotonic system timestamp
 *   - There is an overload with a `SufficientChangePredicate` which will apply externally
 *     provided checks between old/new value and time last marked dirty to allow for complex
 *     rules like marking dirty less than once per second, etc.
 * - *Maybe* call `ForceMarkAsDirty()` in some choice situations (e.g. know for sure it's
 *    an important update, like at the edge of an operational state change).
 * - Call `GetThenResetDirtyState()`. If it returns true, mark the attribute dirty, with the
 *   method most suitable at the call site (e.g. `MatterReportingAttributeChangeCallback` call
 *   or similar methods).
 *
 * @tparam T - the type of underlying numerical value that will be held by the class.
 */
template <typename T>
class QuieterReportingAttribute
{
  public:
    explicit QuieterReportingAttribute(const chip::app::DataModel::Nullable<T>& initialValue) : mValue(initialValue), mLastDirtyValue(initialValue) {}

    using SufficientChangePredicate = std::function<bool(Timestamp /* previousDirtyTime */, Timestamp /* now */, const Nullable<T> & /* previousDirtyValue */, const Nullable<T> & /* newValue */)>;

    /**
     * @brief Factory to generate a functor for "attribute was last reported" at least `minimumDurationMillis` ago.
     *
     * @param minimumDurationMillis - number of millis needed since last marked as dirty before we mark dirty again.
     * @return a functor usable for the `changedPredicate` arg of `SetValue()`
     */
    static SufficientChangePredicate GetPredicateForSufficientTimeSinceLastDirty(chip::System::Clock::Milliseconds64 minimumDurationMillis)
    {
      return [minimumDurationMillis](Timestamp previousDirtyTime, Timestamp now, const Nullable<T> &oldDirtyValue, const Nullable<T> &newValue) -> bool {
        return (oldDirtyValue != newValue) && ((now - previousDirtyTime) >= minimumDurationMillis);
      };
    }

    chip::app::DataModel::Nullable<T> value() const { return mValue; }
    QuieterReportingPolicyFlags policy() const { return mPolicyFlags; }

    void SetPolicy(QuieterReportingPolicyFlags policyFlags) { mPolicyFlags = policyFlags; }

    /**
     * When this returns true, attribute should be marked for reporting. Auto-resets to false after call.
     */
    bool GetThenResetDirtyState()
    {
        bool wasDirty = mIsDirty;
        mIsDirty = false;
        return wasDirty;
    }

    /**
     * Force marking this attribute as dirty, with the `now` timestamp given as the reference point.
     *
     * WARNING: Only call `ForceMarkAsDirty` after a `SetValue` call.
     */
    void ForceMarkAsDirty(Timestamp now)
    {
        mIsDirty = true;
        mLastDirtyTimestampMillis = now;
        mLastDirtyValue = mValue;
    }

    /**
     * Set the updated value of the attribute, computing whether it needs to be reported according to `changedPredicate` and policies.
     *
     * - Any change of nullability between `newValue` and the old value will be considered dirty.
     * - The policies from `QuieterReportingPolicyEnum` and set via `SetPolicy()` are self-explanatory by name.
     * - The changedPredicate will be called with last dirty <timestamp, value> and new <timestamp value> and may override
     *   the dirty state altogether when it returns true. Use sparingly and default to a functor returning false.
     *
     * Internal recording will be done about last dirty value and last dirty timestamp based on the policies having applied.
     *
     * @param newValue - new value to set for the attribute
     * @param now - system monotonic timestamp at the time of the call
     * @param changedPredicate - functor to possibly override dirty state
     */
    void SetValue(const chip::app::DataModel::Nullable<T>& newValue, Timestamp now, SufficientChangePredicate changedPredicate)
    {
        bool isChangeOfNull = newValue.IsNull() ^ mValue.IsNull();
        bool areBothValuesNonNull = !newValue.IsNull() && !mValue.IsNull();

        bool changeToFromZero = areBothValuesNonNull && (*newValue == 0 || *mValue == 0);
        bool isIncrement = areBothValuesNonNull && (*newValue > *mValue);
        bool isDecrement = areBothValuesNonNull && (*newValue < *mValue);

        bool wasDirty = mIsDirty;

        mIsDirty = mIsDirty || isChangeOfNull;
        mIsDirty = mIsDirty || (mPolicyFlags.Has(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero) && changeToFromZero);
        mIsDirty = mIsDirty || (mPolicyFlags.Has(QuieterReportingPolicyEnum::kMarkDirtyOnDecrement) && isDecrement);
        mIsDirty = mIsDirty || (mPolicyFlags.Has(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement) && isIncrement);
        mIsDirty = mIsDirty || changedPredicate(mLastDirtyTimestampMillis, now, mLastDirtyValue, newValue);

        mValue = newValue;

        if (!wasDirty && mIsDirty)
        {
            mLastDirtyValue = newValue;
            mLastDirtyTimestampMillis = now;
        }
    }

    /**
     * Same as the other `SetValue`, but assumes a changedPredicate that never overrides to dirty.
     *
     * This is the easy/common case.
     *
     * @param newValue - new value to set for the attribute
     * @param now - system monotonic timestamp at the time of the call
     */
    void SetValue(const chip::app::DataModel::Nullable<T>& newValue, Timestamp now)
    {
        SetValue(newValue, now, [](Timestamp, Timestamp, const Nullable<T> &, const Nullable<T> &) -> bool { return false; });
    }

  protected:
    chip::app::DataModel::Nullable<T> mValue;
    chip::app::DataModel::Nullable<T> mLastDirtyValue;
    bool mIsDirty = false;
    QuieterReportingPolicyFlags mPolicyFlags{0};
    chip::System::Clock::Milliseconds64 mLastDirtyTimestampMillis{};
};

} // namespace detail

using detail::QuieterReportingAttribute;

} // namespace app
} // namespace chip
