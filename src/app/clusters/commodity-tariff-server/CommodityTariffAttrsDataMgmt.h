/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include "CommodityTariffConsts.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace chip {

// Span comparison operators
inline bool operator==(const Span<const char> & a, const Span<const char> & b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

inline bool operator!=(const Span<const char> & a, const Span<const char> & b)
{
    return !(a == b);
}

inline bool operator==(const Span<const uint32_t> & a, const Span<const uint32_t> & b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

inline bool operator!=(const Span<const uint32_t> & a, const Span<const uint32_t> & b)
{
    return !(a == b);
}

namespace app {

namespace Clusters {
namespace Globals {
namespace Structs {

// CurrencyStruct
inline bool operator==(const CurrencyStruct::Type & lhs, const CurrencyStruct::Type & rhs)
{
    return (lhs.currency == rhs.currency) && (lhs.decimalPoints == rhs.decimalPoints);
}

inline bool operator!=(const CurrencyStruct::Type & lhs, const CurrencyStruct::Type & rhs)
{
    return !(lhs == rhs);
}

// PowerThresholdStruct
inline bool operator==(const PowerThresholdStruct::Type & lhs, const PowerThresholdStruct::Type & rhs)
{
    return (lhs.powerThresholdSource == rhs.powerThresholdSource) && (lhs.powerThreshold == rhs.powerThreshold) &&
        (lhs.apparentPowerThreshold == rhs.apparentPowerThreshold);
}

inline bool operator!=(const PowerThresholdStruct::Type & lhs, const PowerThresholdStruct::Type & rhs)
{
    return !(lhs == rhs);
}

} // namespace Structs
} // namespace Globals

namespace CommodityTariff {
namespace Structs {

// TariffPriceStruct
inline bool operator==(const TariffPriceStruct::Type & lhs, const TariffPriceStruct::Type & rhs)
{
    return (lhs.priceType == rhs.priceType) && (lhs.price == rhs.price) && (lhs.priceLevel == rhs.priceLevel);
}

inline bool operator!=(const TariffPriceStruct::Type & lhs, const TariffPriceStruct::Type & rhs)
{
    return !(lhs == rhs);
}

// AuxiliaryLoadSwitchSettingsStruct
inline bool operator==(const AuxiliaryLoadSwitchSettingsStruct::Type & lhs, const AuxiliaryLoadSwitchSettingsStruct::Type & rhs)
{
    return (lhs.number == rhs.number) && (lhs.requiredState == rhs.requiredState);
}

inline bool operator!=(const AuxiliaryLoadSwitchSettingsStruct::Type & lhs, const AuxiliaryLoadSwitchSettingsStruct::Type & rhs)
{
    return !(lhs == rhs);
}

// PeakPeriodStruct
inline bool operator==(const PeakPeriodStruct::Type & lhs, const PeakPeriodStruct::Type & rhs)
{
    return (lhs.severity == rhs.severity) && (lhs.peakPeriod == rhs.peakPeriod);
}

inline bool operator!=(const PeakPeriodStruct::Type & lhs, const PeakPeriodStruct::Type & rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const DayStruct::Type & lhs, const DayStruct::Type & rhs)
{
    return (lhs.date == rhs.date) && (lhs.dayType == rhs.dayType) && (lhs.dayEntryIDs == rhs.dayEntryIDs);
}

inline bool operator!=(const DayStruct::Type & lhs, const DayStruct::Type & rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const DayEntryStruct::Type & lhs, const DayEntryStruct::Type & rhs)
{
    return (lhs.dayEntryID == rhs.dayEntryID) && (lhs.startTime == rhs.startTime) && (lhs.duration == rhs.duration) &&
        (lhs.randomizationOffset == rhs.randomizationOffset) && (lhs.randomizationType == rhs.randomizationType);
}

inline bool operator!=(const DayEntryStruct::Type & lhs, const DayEntryStruct::Type & rhs)
{
    return !(lhs == rhs);
}



} // namespace Structs
} // namespace CommodityTariff

} // namespace Clusters

namespace CommodityTariffAttrsDataMgmt {

/// @brief Helper for copying spans to Matter data model lists
/// @tparam T Type of elements to copy
template <typename T>
struct SpanCopier
{
    /// @brief Copies span data to a newly allocated list
    /// @param source Input span to copy from
    /// @param destination Output list to populate
    /// @param maxCount Maximum number of elements to copy (default: unlimited)
    /// @return true if copy succeeded, false on memory allocation failure
    static bool Copy(const Span<const T> & source, DataModel::List<const T> & destination,
                     size_t maxCount = std::numeric_limits<size_t>::max())
    {
        if (source.empty())
        {
            destination = DataModel::List<const T>();
            return true;
        }

        const size_t elementsToCopy = std::min(source.size(), maxCount);
        auto * buffer               = static_cast<T *>(Platform::MemoryCalloc(elementsToCopy, sizeof(T)));

        if (!buffer)
            return false;

        std::copy(source.begin(), source.begin() + elementsToCopy, buffer);
        destination = DataModel::List<const T>(Span<const T>(buffer, elementsToCopy));
        return true;
    }
};

/// @brief Specialization for character spans with consistent maxCount semantics
template <>
struct SpanCopier<char>
{
    /// @brief Copies character span to a nullable CharSpan
    /// @param source Input span to copy from
    /// @param destination Output span to populate
    /// @param maxCount Maximum number of characters to copy (default: unlimited)
    /// @return true if copy succeeded, false on memory allocation or size limit failure
    static bool Copy(const CharSpan & source, DataModel::Nullable<CharSpan> & destination,
                     size_t maxCount = std::numeric_limits<size_t>::max())
    {
        if (source.size() > maxCount)
        {
            return false;
        }

        if (source.empty())
        {
            destination.SetNull();
            return true;
        }

        char * buffer = static_cast<char *>(Platform::MemoryCalloc(1, source.size()));
        if (!buffer)
            return false;

        std::copy(source.begin(), source.end(), buffer);
        destination.SetNonNull(CharSpan(buffer, source.size()));
        return true;
    }
};

/// @brief Helper for string to span conversions
struct StrToSpan
{
    /// @brief Copies std::string to a CharSpan
    /// @param source Input string to copy from
    /// @param destination Output span to populate
    /// @param maxCount Maximum number of characters to copy (default: unlimited)
    /// @return CHIP_NO_ERROR on success, error code on failure
    static CHIP_ERROR Copy(const std::string & source, CharSpan & destination, size_t maxCount = std::numeric_limits<size_t>::max())
    {
        if (source.empty())
        {
            destination = CharSpan();
            return CHIP_NO_ERROR;
        }

        if (source.size() > maxCount)
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }

        char * buffer = static_cast<char *>(Platform::MemoryAlloc(source.size()));
        if (!buffer)
            return CHIP_ERROR_NO_MEMORY;

        memcpy(buffer, source.data(), source.size());
        destination           = CharSpan(buffer, source.size());
        return CHIP_NO_ERROR;
    }

    /// @brief Releases memory allocated by a CharSpan
    static void Release(CharSpan & span)
    {
        if (!span.empty())
        {
            Platform::MemoryFree(const_cast<char *>(span.data()));
            span = CharSpan();
        }
    }
};

using namespace CommodityTariffConsts;
using namespace chip::app::Clusters::CommodityTariff::Structs;

template <typename T>
inline CHIP_ERROR CopyData(const T& input, T& output);

template <>
inline CHIP_ERROR CopyData<TariffInformationStruct::Type>(const TariffInformationStruct::Type & input, TariffInformationStruct::Type & output)
{
    output.tariffLabel.SetNull();
    output.providerName.SetNull();
    output.currency.ClearValue();
    output.blockMode.SetNull();

    if (!input.tariffLabel.IsNull())
    {
        if (!SpanCopier<char>::Copy(input.tariffLabel.Value(), output.tariffLabel, input.tariffLabel.Value().size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (!input.providerName.IsNull())
    {
        if (!SpanCopier<char>::Copy(input.providerName.Value(), output.providerName, input.providerName.Value().size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }

    if (input.currency.HasValue())
    {
        output.currency.Emplace();
        if (input.currency.Value().IsNull())
        {
            output.currency.Value().SetNull();
        }
        else
        {
            output.currency.Value().SetNonNull(input.currency.Value().Value());
        }
    }

    if (!input.blockMode.IsNull())
    {
        output.blockMode.SetNonNull(input.blockMode.Value());
    }

    return CHIP_NO_ERROR;
}

template <>
inline CHIP_ERROR CopyData<DayEntryStruct::Type>(const DayEntryStruct::Type & input, DayEntryStruct::Type & output)
{
    output.dayEntryID = input.dayEntryID;
    output.startTime = input.startTime;
    
    output.duration.ClearValue();
    if (input.duration.HasValue())
    {
        output.duration.SetValue(input.duration.Value());
    }
    
    output.randomizationOffset.ClearValue();
    if (input.randomizationOffset.HasValue())
    {
        output.randomizationOffset.SetValue(input.randomizationOffset.Value());
    }
    
    output.randomizationType.ClearValue();
    if (input.randomizationType.HasValue())
    {
        output.randomizationType.SetValue(input.randomizationType.Value());
    }
    
    return CHIP_NO_ERROR;
}

template <>
inline CHIP_ERROR CopyData<TariffComponentStruct::Type>(const TariffComponentStruct::Type & input, TariffComponentStruct::Type & output)
{
    output.tariffComponentID = input.tariffComponentID;
    
    output.price.ClearValue();
    if (input.price.HasValue())
    {
        output.price.Emplace();
        output.price.Value().SetNull();
        if (!input.price.Value().IsNull())
        {
            auto & priceInput = input.price.Value().Value();            
            TariffPriceStruct::Type tmp_price;

            tmp_price.priceType = priceInput.priceType;
            
            if (priceInput.price.HasValue())
            {
                tmp_price.price.SetValue(priceInput.price.Value());
            }
            
            if (priceInput.priceLevel.HasValue())
            {
                tmp_price.priceLevel.SetValue(priceInput.priceLevel.Value());
            }

            output.price.Value().SetNonNull(tmp_price);
        }
    }
    
    output.friendlyCredit.ClearValue();
    if (input.friendlyCredit.HasValue())
    {
        output.friendlyCredit.SetValue(input.friendlyCredit.Value());
    }
    
    output.auxiliaryLoad.ClearValue();
    if (input.auxiliaryLoad.HasValue())
    {
        output.auxiliaryLoad.Emplace();
        output.auxiliaryLoad.Value().number = input.auxiliaryLoad.Value().number;
        output.auxiliaryLoad.Value().requiredState = input.auxiliaryLoad.Value().requiredState;
    }
    
    output.peakPeriod.ClearValue();
    if (input.peakPeriod.HasValue())
    {
        output.peakPeriod.Emplace();
        output.peakPeriod.Value().severity = input.peakPeriod.Value().severity;
        output.peakPeriod.Value().peakPeriod = input.peakPeriod.Value().peakPeriod;
    }
    
    output.powerThreshold.ClearValue();
    if (input.powerThreshold.HasValue())
    {
        output.powerThreshold.Emplace();
        // Assuming PowerThresholdStruct has simple fields that can be directly copied
        output.powerThreshold.Value() = input.powerThreshold.Value();
    }
    
    output.threshold.SetNull();
    if (!input.threshold.IsNull())
    {
        output.threshold.SetNonNull(input.threshold.Value());
    }
    
    output.label.ClearValue();
    if (input.label.HasValue())
    {
        output.label.Emplace();
        output.label.Value().SetNull();
        if (!input.label.Value().IsNull())
        {
            if (!SpanCopier<char>::Copy(
                input.label.Value().Value(), output.label.Value(), input.label.Value().Value().size()))
            {
                return CHIP_ERROR_NO_MEMORY;
            }
        }
    }
    
    output.predicted.ClearValue();
    if (input.predicted.HasValue())
    {
        output.predicted.SetValue(input.predicted.Value());
    }
    
    return CHIP_NO_ERROR;
}

template <>
inline CHIP_ERROR CopyData<TariffPeriodStruct::Type>(const TariffPeriodStruct::Type & input, TariffPeriodStruct::Type & output)
{
    output.label.SetNull();
    if (!input.label.IsNull())
    {
        if (!SpanCopier<char>::Copy(
            input.label.Value(), output.label, input.label.Value().size()))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    
    // For lists, we assume the output has already been initialized with sufficient capacity
    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()), output.dayEntryIDs, kTariffPeriodItemMaxIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    
    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.tariffComponentIDs.data(), input.tariffComponentIDs.size()), output.tariffComponentIDs, kTariffPeriodItemMaxIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    
    return CHIP_NO_ERROR;
}

template <>
inline CHIP_ERROR CopyData<DayPatternStruct::Type>(const DayPatternStruct::Type & input, DayPatternStruct::Type & output)
{
    output.dayPatternID = input.dayPatternID;
    output.daysOfWeek = input.daysOfWeek;
    
    // For lists, we assume the output has already been initialized with sufficient capacity
    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()), output.dayEntryIDs, kDayPatternItemMaxDayEntryIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    
    return CHIP_NO_ERROR;
}

template <>
inline CHIP_ERROR CopyData<DayStruct::Type>(const DayStruct::Type & input, DayStruct::Type & output)
{
    output.date = input.date;
    output.dayType = input.dayType;
    
    // For lists, we assume the output has already been initialized with sufficient capacity
    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayEntryIDs.data(), input.dayEntryIDs.size()), output.dayEntryIDs, kDayStructItemMaxDayEntryIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    
    return CHIP_NO_ERROR;
}

template <>
inline CHIP_ERROR CopyData<CalendarPeriodStruct::Type>(const CalendarPeriodStruct::Type & input, CalendarPeriodStruct::Type & output)
{
    output.startDate.SetNull();
    if (!input.startDate.IsNull())
    {
        output.startDate.SetNonNull(input.startDate.Value());
    }
    
    // For lists, we assume the output has already been initialized with sufficient capacity
    if (!SpanCopier<uint32_t>::Copy(chip::Span<const uint32_t>(input.dayPatternIDs.data(), input.dayPatternIDs.size()), output.dayPatternIDs, kCalendarPeriodItemMaxDayPatternIDs))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    
    return CHIP_NO_ERROR;
}

/// @brief Type trait for nullable types
template <typename U>
struct IsNullable : std::false_type
{
};
template <typename U>
struct IsNullable<DataModel::Nullable<U>> : std::true_type
{
};

/// @brief Type trait for list types
template <typename U>
struct IsList : std::false_type
{
};
template <typename U>
struct IsList<DataModel::List<U>> : std::true_type
{
};

/// @brief Type categorization traits
template <typename U>
struct IsNumeric : std::integral_constant<bool, std::is_integral<U>::value || std::is_floating_point<U>::value>
{
};
template <typename U>
struct IsEnum : std::is_enum<U>
{
};
template <typename U>
struct IsStruct
    : std::integral_constant<bool, !IsList<U>::value && !IsNumeric<U>::value && !IsEnum<U>::value && !std::is_pointer<U>::value>
{
};

/// @brief Type extraction utilities
template <typename U>
struct ExtractWrappedType
{
    using type = U;
};
template <typename U>
struct ExtractWrappedType<DataModel::Nullable<U>>
{
    using type = typename ExtractWrappedType<U>::type;
};
template <typename U>
using ExtractWrappedType_t = typename ExtractWrappedType<U>::type;
template <typename U>
struct ExtractPayloadType
{
    using type = U; // Base case - not a wrapper
};
template <typename U>
struct ExtractPayloadType<DataModel::Nullable<U>>
{
    using type = typename ExtractPayloadType<U>::type;
};
template <typename U>
struct ExtractPayloadType<DataModel::List<U>>
{
    using type = typename ExtractPayloadType<U>::type;
};
template <typename U>
using ExtractPayloadType_t = typename ExtractPayloadType<U>::type;

/**
 * @class CTC_BaseDataClass
 * @tparam T The attribute value type (nullable list , nullable struct or primitive)
 * @brief Base template class for thread-safe attribute data management with atomic update support
 *
 * Provides core functionality for:
 * - Atomic value updates with validation
 * - Change detection and notification
 * - Memory management for complex types
 * - Type-specific behavior through template specialization
 *
 * The class handles three types of attributes differently:
 * 1. Nullable types (DataModel::Nullable<U>)
 * 2. List types (DataModel::List<U>)
 * 3. Primitive/struct types
 *
 * Key Features:
 * - Double-buffered storage for atomic updates
 * - Automatic memory management for complex types
 * - Strict state machine for update flow
 * - Thread-safe value access patterns
 * - Customizable validation hooks
 *
 * @section thread_safety Thread Safety
 * @brief Thread-safe usage patterns
 *
 * The class provides the following thread-safety guarantees:
 * - Concurrent read access is always safe via GetValue()
 * - Write operations must be externally synchronized
 * - The update state machine ensures consistent transitions
 *
 * @section memory_management Memory Management
 * @brief Automatic cleanup for complex types
 *
 * The class handles cleanup for:
 * - List memory (automatically freed)
 * - Nested structs (via virtual CleanupStructValue)
 * - Nullable state transitions
 *
 * @section update_flow Update Flow
 * @brief State machine for managing atomic attribute updates
 *
 * The complete update process requires explicit UpdateFinish() call:
 *
 * @dot
 * digraph update_flow {
 *   kIdle -> kInitialized [label="CreateNewValue()"];
 *   kInitialized -> kAssigned [label="MarkAsAssigned()"];
 *   kAssigned -> kValidated [label="UpdateBegin()"];
 *   kValidated ->  kIdle  [label="UpdateFinish() (mandatory)"];
 *
 *   // Early termination paths
 *   kInitialized -> kIdle [label="UpdateFinish()" style="dashed"];
 *   kAssigned -> kIdle [label="UpdateFinish()" style="dashed"];
 *   kValidated -> kIdle [label="UpdateFinish()" style="dashed"];
 * }
 * @enddot
 *
 * ### Complete Successful Sequence:
 * 1. CreateNewValue()   // kIdle → kInitialized
 * 2. Modify value       // 
 * 3. MarkAsAssigned()   // kInitialized → kAssigned
 * 4. UpdateBegin()      // kAssigned → kValidated
 * 6. UpdateFinish()     // kValidated →  kIdle (mandatory cleanup)
 *
 * ### Critical Notes:
 * - Omitting UpdateFinish() will leak resources
 * - UpdateFinish() can be called at any state for cleanup
 *
 * @see CreateNewValue()
 * @see MarkAsAssigned()
 * @see UpdateBegin()
 * @see UpdateFinish()
 */
template <typename T>
class CTC_BaseDataClass
{
public:
    using ValueType   = T;
    using WrappedType = ExtractWrappedType_t<ValueType>;
    using PayloadType = ExtractPayloadType_t<WrappedType>;

    /**
     * @brief Construct a new data class instance
     * @param aAttrId Attribute ID for callback identification
     *
     * Initializes the storage based on type:
     * - Nullable types: set to null
     * - List types: initialized as empty list
     * - Others: left uninitialized
     */
    explicit CTC_BaseDataClass(AttributeId aAttrId) : mAttrId(aAttrId)
    {
        if constexpr (IsValueNullable())
        {
            GetValueRef().SetNull();
        }
        else if constexpr (IsValueList())
        {
            GetValueRef() = ValueType();
        }
        mHoldState[mActiveValueIdx]   = StorageState::kEmpty;
        mUpdateState = UpdateState::kIdle;
    }

    /// @brief Virtual destructor for proper cleanup
    virtual ~CTC_BaseDataClass() { Cleanup(); }

    /**
     * @brief Get mutable reference to stored value
     * @return ValueType & Reference to the active value storage
     */
    ValueType & GetValue() { return GetValueRef(); }
    ValueType & GetNewValue() { return GetNewValueRef(); }

    /**
     * @brief Check if current update is validated
     * @return true if in kValidated state
     */
    bool IsValid() const { return (mUpdateState == UpdateState::kValidated); }

    /**
     * @brief Check if value storage contains valid data
     * @return true if storage is in kHold state
     */
    bool HasValue() const { return (mHoldState[mActiveValueIdx] == StorageState::kHold); }
    bool HasNewValue() const { return (mHoldState[!mActiveValueIdx] == StorageState::kHold); }

    /**
     * @brief Prepares a new value for modification
     * @param size Number of elements to allocate (for list types only)
     * @return CHIP_NO_ERROR on success, or:
     *         - CHIP_ERROR_NO_MEMORY if allocation fails
     *         - CHIP_ERROR_INVALID_LIST_LENGTH if size < 1 for list types
     *         - CHIP_ERROR_INCORRECT_STATE if called during active update
     *
     * @note For list types:
     *       - Allocates memory for specified number of elements
     *       - Initializes list storage
     *       - Transitions state to kInitialized
     * @note For non-list types:
     *       - Creates default-initialized value
     *       - Ignores size parameter
     */
    CHIP_ERROR CreateNewListValue(size_t size)
    {
        if (mUpdateState != UpdateState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if constexpr (!IsValueList())
        {
            return CHIP_ERROR_INTERNAL;
        }

        if (size >= 1)
        {
            auto * buffer = static_cast<PayloadType *>(Platform::MemoryCalloc(size, sizeof(PayloadType)));
            if (!buffer)
            {
                return CHIP_ERROR_NO_MEMORY;
            }
            if constexpr (IsValueNullable())
            {
                GetNewValueRef().SetNonNull(DataModel::List<PayloadType>(buffer, size));
            }
            else
            {
                GetNewValueRef() = DataModel::List<PayloadType>(buffer, size);
            }
        }
        else
        {
            if constexpr (IsValueNullable())
            {
                GetNewValueRef().SetNull();
            }
            else
            {
                GetNewValueRef() = DataModel::List<PayloadType>();
            }
        }

        mUpdateState = UpdateState::kInitialized;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CreateNewSingleValue()
    {
        if (mUpdateState != UpdateState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if constexpr (IsValueList())
        {
            return CHIP_ERROR_INTERNAL;
        }

        if constexpr (IsValueNullable())
        {
            GetNewValueRef().SetNull();
        }
        else
        {
            GetNewValueRef() = ValueType();
        }

        mUpdateState = UpdateState::kInitialized;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetNewValue(const ValueType & aValue)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        if (aValue.IsNull())
        {
            CleanupByIdx(!mActiveValueIdx);
            mUpdateState = UpdateState::kInitialized;
            MarkAsAssigned();
            return CHIP_NO_ERROR;
        }

        auto & newValue = aValue.Value();

        if constexpr (IsValueList())
        {
            err = CreateNewListValue(newValue.size());

            if (CHIP_NO_ERROR == err)
            {
                auto buffer = GetNewValueRef().Value().data();
                for (size_t idx = 0; idx < newValue.size(); idx++)
                {
                    if (CHIP_NO_ERROR == (err = CopyData<PayloadType>(newValue[idx], buffer[idx])))
                    {
                        continue;
                    }
                    break;
                }
            }
        }
        else if constexpr (IsValueStruct())
        {
            err = CreateNewSingleValue();

            if (CHIP_NO_ERROR == err)
            {
                PayloadType tmpValue = newValue;
                err = CopyData<PayloadType>(newValue, tmpValue); 
                if (err == CHIP_NO_ERROR)
                {
                    GetNewValueRef().SetNonNull(tmpValue);
                }
            }
        }
        else if constexpr (IsValueScalar())
        {
            err = CreateNewSingleValue();

            if (CHIP_NO_ERROR == err)
            {
                GetNewValueRef().SetNonNull(newValue);
            }
        }

        if (err == CHIP_NO_ERROR)
        {
            mHoldState[!mActiveValueIdx] = StorageState::kHold;
            MarkAsAssigned();
        }
        else
        {
            CleanupByIdx(!mActiveValueIdx);
        }

        return err;
    }

    /**
     * @brief Signals completion of new value modifications
     * @return CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kInitialized state
     */
    CHIP_ERROR MarkAsAssigned()
    {
        if (mUpdateState != UpdateState::kInitialized)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        mUpdateState = UpdateState::kAssigned;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Validates and prepares the new value for commit
     * @param aUpdCtx Context pointer for callback
     * @return CHIP_NO_ERROR if validation succeeds
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kAssigned state
     */
    CHIP_ERROR UpdateBegin(void * aUpdCtx)
    {
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState == UpdateState::kIdle)
        {
            return CHIP_NO_ERROR;
        }

        if (mUpdateState != UpdateState::kAssigned)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        CHIP_ERROR err = CHIP_NO_ERROR;

        if (aUpdCtx != nullptr)
        {
            mAuxData = aUpdCtx;            
            err = ValidateNewValue();
        }

        if (err == CHIP_NO_ERROR)
        {
            mUpdateState = UpdateState::kValidated;
        }
        else
        {
            ChipLogError(NotSpecified, "The value of attr %d is not valid!", mAttrId);
        }

        return err;
    }

    /**
     * @brief the function performs a correct completion of the value update process
     * @return The return value indicates that the stored value has changed.
     * @note Performs cleanup and resets to idle state
     */
    bool UpdateFinish()
    {
        bool ret = false;
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState == UpdateState::kIdle)
        {
            return false;
        }

        if ((mUpdateState == UpdateState::kValidated) && (HasChanged()))
        {
            if (HasValue())
            {
                CleanupByIdx(mActiveValueIdx); // Cleanup current value
            }

            SwapActiveValueStorage();

            ret = true;
        }
        else
        {
            CleanupByIdx(!mActiveValueIdx);
        }

        mUpdateState = UpdateState::kIdle;

        return ret;
    }

    /**
     * @brief Full cleanup of the value storage
     * @return The return value indicates that the stored value is changed.
     */
    bool Cleanup()
    {
        bool ret = false;
        if (HasValue())
        {
            ret = true;
        }

        CleanupByIdx(!mActiveValueIdx);
        CleanupByIdx(mActiveValueIdx);

        return ret;
    }

    AttributeId GetAttrId()
    {
        return  mAttrId;
    }

private:
    uint8_t mActiveValueIdx = 0;

    ValueType & GetValueRef() { return mValueStorage[mActiveValueIdx]; }

    ValueType & GetNewValueRef() { return mValueStorage[!mActiveValueIdx]; }

    void SwapActiveValueStorage() { mActiveValueIdx = !mActiveValueIdx; }

    static constexpr bool IsValueNullable() { return IsNullable<ValueType>::value; }

    static constexpr bool IsValueList() { return IsList<WrappedType>::value; }

    static constexpr bool IsValueStruct() { return IsStruct<WrappedType>::value; }

    static constexpr bool IsValueScalar() { return (IsNumeric<WrappedType>::value || IsEnum<WrappedType>::value) ; }

    enum class StorageState : uint8_t
    {
        kEmpty, // Value not initialized (default state)
        kHold,  // GetValueRef() holds valid data
    };

    enum class UpdateState : uint8_t
    {
        kIdle,        // No active update
        kInitialized, // New value initialized
        kAssigned,    // New value populated
        kValidated,   // New value validated
    };

    CHIP_ERROR ValidateNewValue() { return Validate(GetNewValueRef()); }

    bool HasChanged()
    {
        if constexpr (IsValueNullable())
        {
            return NullableNotEqual(GetNewValueRef(), GetValueRef());
        }
        else if constexpr (IsValueList())
        {
            return ListsNotEqual(GetNewValueRef(), GetValueRef());
        }
        else
        {
            return GetNewValueRef() != GetValueRef();
        }
    }

    void CleanupByIdx(uint8_t aIdx)
    {
        if (mActiveValueIdx == aIdx)
        {
            CleanupValueByRef(mValueStorage[mActiveValueIdx]);
            mHoldState[mActiveValueIdx] = StorageState::kEmpty;
        }
        else
        {
            CleanupValueByRef(mValueStorage[!mActiveValueIdx]);
            mHoldState[!mActiveValueIdx] = StorageState::kEmpty;
        }
    }

    void CleanupValueByRef(ValueType & aValue)
    {
        if constexpr (IsValueNullable())
        {
            if (!aValue.IsNull())
            {
                if constexpr (IsList<WrappedType>::value)
                {
                    CleanupList(aValue.Value());
                }
                else if constexpr (IsValueStruct())
                {
                    CleanupStruct(aValue.Value());
                }
            }
            aValue.SetNull();
        }
        else if constexpr (IsValueList())
        {
            CleanupList(aValue);
        }
    }

    void CleanupList(DataModel::List<PayloadType> & list)
    {
        for (auto & item : list)
        {
            CleanupStruct(item);
        }
        if (list.data())
        {
            Platform::MemoryFree(list.data());
            list = DataModel::List<PayloadType>();
        }
    }

    void CleanupStruct(PayloadType & aValue) { CleanupStructValue(aValue); }

protected:
    ValueType mValueStorage[2]; // Double-buffered storage
    StorageState mHoldState[2]  = {StorageState::kEmpty};

    void * mAuxData = nullptr;                  // Validation context
    const AttributeId mAttrId;                     // Attribute identifier

    UpdateState mUpdateState = UpdateState::kIdle;

    virtual CHIP_ERROR Validate(const ValueType & aValue) const { return CHIP_NO_ERROR; }

    /**
     * @brief Compares two structured values for inequality
     * @param source The new/input value being compared
     * @param destination The existing/stored value being compared against
     * @return bool
     *   - true if values are different (needs update)
     *   - false if values are identical (no update needed)
     * @note This intentionally uses "Compare" in the name but implements "not equal" semantics
     *       to match the expected behavior in change detection flows.
     * @warning Must be overridden for any struct payload types. The base implementation
     *          always returns false (no change) and logs an error.
     *
     * Example override:
     * @code
     * bool CompareStructValue(const MyStruct& src, const MyStruct& dst) const override {
     *     return src.field1 != dst.field1 ||
     *            src.field2 != dst.field2;
     * }
     * @endcode
     */
    virtual bool CompareStructValue(const PayloadType & source, const PayloadType & destination) const
    {
        ChipLogError(NotSpecified, "CompareStructValue must be overridden for struct types!");
        return false;
    }

    bool ListsNotEqual(const DataModel::List<PayloadType> & source, const DataModel::List<PayloadType> & destination)
    {
        if (source.size() != destination.size())
        {
            return true;
        }

        for (size_t i = 0; i < source.size(); i++)
        {
            if (CompareStructValue(source[i], destination[i]))
            {
                return true;
            }
        }

        return false;
    }

    bool NullableNotEqual(const ValueType & a, const ValueType & b)
    {
        bool is_neq = false;
        if (a.IsNull() || b.IsNull())
        {
            is_neq = a.IsNull() != b.IsNull();
        }
        else
        {
            if constexpr (IsList<WrappedType>::value)
            {
                is_neq = ListsNotEqual(a.Value(), b.Value());
            }
            else if constexpr (IsValueStruct())
            {
                is_neq = CompareStructValue(a.Value(), b.Value());
            }
            else
            {
                is_neq = (a.Value() != b.Value());
            }
        }

        return is_neq;
    }

    virtual void CleanupStructValue(PayloadType & aValue) { (void) aValue; }
};

} // namespace CommodityTariffAttrsDataMgmt
} // namespace app
} // namespace chip
