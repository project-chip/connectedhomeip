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
#include <type_traits>

#include <app/data-model/Nullable.h>
#include <lib/support/BitFlags.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

enum class QuieterReportingPolicyEnum
{
    kMarkDirtyOnChangeToFromZero = (1u << 0),
    kMarkDirtyOnDecrement        = (1u << 1),
    kMarkDirtyOnIncrement        = (1u << 2),
};

enum class AttributeDirtyState
{
    kNoReportNeeded = 0,
    kMustReport     = 1,
};

using QuieterReportingPolicyFlags = BitFlags<QuieterReportingPolicyEnum>;

namespace detail {

using Timestamp = System::Clock::Milliseconds64;
template <typename T>
using Nullable = DataModel::Nullable<T>;

/**
 * This class helps track reporting state of an attribute to properly keep track of whether
 * it needs to be marked as dirty or not for purposes of reporting using
 * "7.7.9 Quieter Reporting Quality" (Q quality)
 *
 * The class can be configured via `policy()` to have some/all of the common reasons
 * for reporting (e.g. increment only, decrement only, change to/from zero).
 *
 * Changes of null to non-null or non-null to null are always considered dirty.
 *
 * It is possible to force mark the attribute as dirty (see `ForceDirty()`) such as
 * for conditions like "When there is any increase or decrease in the estimated time
 * remaining that was due to progressing insight of the server's control logic".
 *
 * Class maintains a `current value` and a timestamped `dirty` state. The `SetValue()`
 * method must be used to update the `current value` and will return AttributeDirtyState::kMustReport
 * if the attribute should be marked dirty/
 *
 * - `SetValue()` has internal rules for null/non-null changes and policy-based rules
 * - `SetValue()` with a `SufficientChangePredicate` uses the internal rules in addition to
 *    the predicate to determine dirty state
 *
 * See [QuieterReportingPolicyEnum] for policy flags on when a value is considered dirty
 * beyond non/non-null changes.
 *
 * Common quieter reporting usecases that can be supported by this class are:
 * - If attribute has changed due to a change in the X or Y attributes
 *   - Use SufficientChangePredicate version
 * - When it changes from 0 to any other value and vice versa
 *   - Use `kMarkDirtyOnChangeToFromZero` internal policy.
 * - When it changes from null to any other value and vice versa
 *   - Built-in rule.
 * - When it increases
 *   - Use `kMarkDirtyOnIncrement` internal policy.
 * - When it decreases
 *   - Use `kMarkDirtyOnDecrement` internal policy.
 * - When there is any increase or decrease in the estimated time remaining that was
 *   due to progressing insight of the server's control logic
 *   - Use SufficientChangePredicate version with an always-true predicate.
 * - When it changes at a rate significantly different from one unit per second.
 *   - Use SufficientChangePredicate version.
 * Example usage in-situ:
 *
 * Class has:
 *     QuieterReportingAttribute<uint8_t> mAttrib;
 *
 * Code at time of setting new value has:
 *
 *     uint8_t newValue = driver.GetNewValue();
 *     auto now = SystemClock().GetMonotonicTimestamp();
 *     if (mAttrib.SetValue(newValue, now) == AttributeDirtyState::kMustReport)
 *     {
 *         MatterReportingAttributeChangeCallback(path_for_attribute);
 *     }
 *
 * @tparam T - the type of underlying numerical value that will be held by the class.
 */
template <typename T, std::enable_if_t<std::is_arithmetic<T>::value, bool> = true>
class QuieterReportingAttribute
{
public:
    explicit QuieterReportingAttribute(const Nullable<T> & initialValue) : mValue(initialValue), mLastDirtyValue(initialValue) {}
    // constructor that works with arrays of QuieterReportingAttribute
    explicit QuieterReportingAttribute() : mValue(DataModel::NullNullable), mLastDirtyValue(DataModel::NullNullable) {}

    struct SufficientChangePredicateCandidate
    {
        // Timestamp of last time attribute was marked dirty.
        Timestamp lastDirtyTimestamp;
        // New (`now`) timestamp passed in `SetValue()`.
        Timestamp nowTimestamp;
        // Value last marked as dirty.
        const Nullable<T> & lastDirtyValue;
        // New value passed in `SetValue()`, to compare against lastDirtyValue for sufficient change if needed.
        const Nullable<T> & newValue;
    };

    using SufficientChangePredicate = std::function<bool(const SufficientChangePredicateCandidate &)>;

    /**
     * @brief Factory to generate a functor for "attribute was last reported" at least `minimumDurationMillis` ago.
     *
     * @param minimumDurationMillis - number of millis needed since last marked as dirty before we mark dirty again.
     * @return a functor usable for the `changedPredicate` arg of `SetValue()`
     */
    static SufficientChangePredicate
    GetPredicateForSufficientTimeSinceLastDirty(System::Clock::Milliseconds64 minimumDurationMillis)
    {
        return [minimumDurationMillis](const SufficientChangePredicateCandidate & candidate) -> bool {
            return (candidate.lastDirtyValue != candidate.newValue) &&
                ((candidate.nowTimestamp - candidate.lastDirtyTimestamp) >= minimumDurationMillis);
        };
    }

    Nullable<T> value() const { return mValue; }
    QuieterReportingPolicyFlags & policy() { return mPolicyFlags; }
    const QuieterReportingPolicyFlags & policy() const { return mPolicyFlags; }

    /**
     * Set the updated value of the attribute, computing whether it needs to be reported according to `changedPredicate` and
     * policies.
     *
     * - Any change of nullability between `newValue` and the old value will be considered dirty.
     * - The policies from `QuieterReportingPolicyEnum` and set via `SetPolicy()` are self-explanatory by name.
     * - The changedPredicate will be called with last dirty <timestamp, value> and new <timestamp value> and may override
     *   the dirty state altogether when it returns true. Use sparingly and default to a functor returning false.
     *   The changedPredicate is only called on change.
     *
     * Internal recording will be done about last dirty value and last dirty timestamp based on the policies having applied.
     *
     * @param newValue - new value to set for the attribute
     * @param now - system monotonic timestamp at the time of the call
     * @param changedPredicate - functor to possibly override dirty state
     * @return AttributeDirtyState::kMustReport if attribute must be marked dirty right away, or
     * AttributeDirtyState::kNoReportNeeded otherwise.
     */
    AttributeDirtyState SetValue(const DataModel::Nullable<T> & newValue, Timestamp now, SufficientChangePredicate changedPredicate)
    {
        bool isChangeOfNull         = newValue.IsNull() != mValue.IsNull();
        bool areBothValuesNonNull   = !newValue.IsNull() && !mValue.IsNull();
        bool areBothValuesDifferent = areBothValuesNonNull && (newValue.Value() != mValue.Value());

        bool changeToFromZero = areBothValuesNonNull && areBothValuesDifferent && (newValue.Value() == 0 || mValue.Value() == 0);
        bool isIncrement      = areBothValuesNonNull && (newValue.Value() > mValue.Value());
        bool isDecrement      = areBothValuesNonNull && (newValue.Value() < mValue.Value());

        bool isNewlyDirty = isChangeOfNull;
        isNewlyDirty =
            isNewlyDirty || (mPolicyFlags.Has(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero) && changeToFromZero);
        isNewlyDirty = isNewlyDirty || (mPolicyFlags.Has(QuieterReportingPolicyEnum::kMarkDirtyOnDecrement) && isDecrement);
        isNewlyDirty = isNewlyDirty || (mPolicyFlags.Has(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement) && isIncrement);

        // Only execute predicate on value change from last marked dirty.
        if (newValue != mLastDirtyValue)
        {
            SufficientChangePredicateCandidate candidate{
                mLastDirtyTimestampMillis, // lastDirtyTimestamp
                now,                       // nowTimestamp
                mLastDirtyValue,           // lastDirtyValue
                newValue                   // newValue
            };
            isNewlyDirty = isNewlyDirty || changedPredicate(candidate);
        }

        mValue = newValue;

        if (isNewlyDirty)
        {
            mLastDirtyValue           = newValue;
            mLastDirtyTimestampMillis = now;
        }

        return isNewlyDirty ? AttributeDirtyState::kMustReport : AttributeDirtyState::kNoReportNeeded;
    }

    /**
     * Same as the other `SetValue`, but assumes a changedPredicate that never overrides to dirty.
     *
     * This is the easy/common case.
     *
     * @param newValue - new value to set for the attribute
     * @param now - system monotonic timestamp at the time of the call
     * @return AttributeDirtyState::kMustReport if attribute must be marked dirty right away, or
     * AttributeDirtyState::kNoReportNeeded otherwise.
     */
    AttributeDirtyState SetValue(const DataModel::Nullable<T> & newValue, Timestamp now)
    {
        return SetValue(newValue, now, [](const SufficientChangePredicateCandidate &) -> bool { return false; });
    }

protected:
    // Current value of the attribute.
    DataModel::Nullable<T> mValue;
    // Last value that was marked as dirty (to use in comparisons for change, e.g. by SufficientChangePredicate).
    DataModel::Nullable<T> mLastDirtyValue;
    // Enabled internal change detection policies.
    QuieterReportingPolicyFlags mPolicyFlags{ 0 };
    // Timestamp associated with the last time the attribute was marked dirty (to use in comparisons for change).
    chip::System::Clock::Timestamp mLastDirtyTimestampMillis{};
};

} // namespace detail

using detail::QuieterReportingAttribute;

} // namespace app
} // namespace chip
