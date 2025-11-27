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

#include "CommodityTariffConsts.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <platform/LockTracker.h>

#include <atomic>
#include <cassert>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
namespace CurrencyStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.currency == rhs.currency) && (lhs.decimalPoints == rhs.decimalPoints);
}
} // namespace CurrencyStruct

namespace PowerThresholdStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.powerThresholdSource == rhs.powerThresholdSource) && (lhs.powerThreshold == rhs.powerThreshold) &&
        (lhs.apparentPowerThreshold == rhs.apparentPowerThreshold);
}
} // namespace PowerThresholdStruct

} // namespace Structs
} // namespace Globals

namespace CommodityTariff {
namespace Structs {

namespace TariffPriceStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.priceType == rhs.priceType) && (lhs.price == rhs.price) && (lhs.priceLevel == rhs.priceLevel);
}
} // namespace TariffPriceStruct

namespace AuxiliaryLoadSwitchSettingsStruct {
inline bool operator==(const AuxiliaryLoadSwitchSettingsStruct::Type & lhs, const AuxiliaryLoadSwitchSettingsStruct::Type & rhs)
{
    return (lhs.number == rhs.number) && (lhs.requiredState == rhs.requiredState);
}
} // namespace AuxiliaryLoadSwitchSettingsStruct

// PeakPeriodStruct
namespace PeakPeriodStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.severity == rhs.severity) && (lhs.peakPeriod == rhs.peakPeriod);
}
} // namespace PeakPeriodStruct

namespace TariffInformationStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.currency == rhs.currency) && (lhs.tariffLabel == rhs.tariffLabel) && (lhs.providerName == rhs.providerName) &&
        (lhs.blockMode == rhs.blockMode);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace TariffInformationStruct

namespace DayStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.date == rhs.date) && (lhs.dayType == rhs.dayType) && (lhs.dayEntryIDs == rhs.dayEntryIDs);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace DayStruct

namespace DayEntryStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.dayEntryID == rhs.dayEntryID) && (lhs.startTime == rhs.startTime) && (lhs.duration == rhs.duration) &&
        (lhs.randomizationOffset == rhs.randomizationOffset) && (lhs.randomizationType == rhs.randomizationType);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace DayEntryStruct

namespace DayPatternStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.dayPatternID == rhs.dayPatternID) && (lhs.daysOfWeek.Raw() == rhs.daysOfWeek.Raw()) &&
        (lhs.dayEntryIDs == rhs.dayEntryIDs);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace DayPatternStruct

namespace TariffComponentStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.label == rhs.label) && (lhs.price == rhs.price) && (lhs.friendlyCredit == rhs.friendlyCredit) &&
        (lhs.auxiliaryLoad == rhs.auxiliaryLoad) && (lhs.peakPeriod == rhs.peakPeriod) &&
        (lhs.powerThreshold == rhs.powerThreshold) && (lhs.predicted == rhs.predicted) && (lhs.threshold == rhs.threshold);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace TariffComponentStruct

namespace TariffPeriodStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.tariffComponentIDs == rhs.tariffComponentIDs) && (lhs.dayEntryIDs == rhs.dayEntryIDs) && (lhs.label == rhs.label);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace TariffPeriodStruct

namespace CalendarPeriodStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.startDate == rhs.startDate) && (lhs.dayPatternIDs == rhs.dayPatternIDs);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace CalendarPeriodStruct

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
    /// @return CHIP_NO_ERROR if copy succeeded, error code on failure
    static CHIP_ERROR Copy(const Span<const T> & source, DataModel::List<const T> & destination,
                           size_t maxCount = std::numeric_limits<size_t>::max())
    {
        if (source.empty())
        {
            destination = DataModel::List<const T>();
            return CHIP_NO_ERROR;
        }

        const size_t elementsToCopy = std::min(source.size(), maxCount);
        auto * buffer               = static_cast<T *>(Platform::MemoryCalloc(elementsToCopy, sizeof(T)));

        if (!buffer)
            return CHIP_ERROR_NO_MEMORY;

        std::copy(source.begin(), source.begin() + elementsToCopy, buffer);
        destination = DataModel::List<const T>(Span<const T>(buffer, elementsToCopy));
        return CHIP_NO_ERROR;
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
    /// @return CHIP_NO_ERROR if copy succeeded, error code on failure
    static CHIP_ERROR Copy(const CharSpan & source, DataModel::Nullable<CharSpan> & destination,
                           size_t maxCount = std::numeric_limits<size_t>::max())
    {
        if (source.size() > maxCount)
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }

        if (source.empty())
        {
            destination.SetNull();
            return CHIP_NO_ERROR;
        }

        char * buffer = static_cast<char *>(Platform::MemoryCalloc(1, source.size()));
        if (!buffer)
            return CHIP_ERROR_NO_MEMORY;

        std::copy(source.begin(), source.end(), buffer);
        destination.SetNonNull(CharSpan(buffer, source.size()));
        return CHIP_NO_ERROR;
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
    static CHIP_ERROR Copy(const std::string & source, CharSpan & destination,
                           size_t maxCount = CommodityTariffConsts::kDefaultStringValuesMaxBufLength)
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
        destination = CharSpan(buffer, source.size());
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

template <typename T, auto X>
void ListToMap(const DataModel::List<T> & aList, std::map<uint32_t, const T *> & aMap)
{
    for (const auto & item : aList)
    {
        // Insert into map with specified entry as key
        aMap.emplace(item.*X, &item);
    }
}

template <typename T, auto X>
void ListToMap(const DataModel::List<T> & aList, std::unordered_map<uint32_t, const T *> & aMap)
{
    for (const auto & item : aList)
    {
        // Insert into map with specified entry as key
        aMap.emplace(item.*X, &item);
    }
}

template <typename T>
CHIP_ERROR ValidateListEntry(const T & entryNewValue, void * aCtx);

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
struct ExtractNonNullableType
{
    using type = U;
};
template <typename U>
struct ExtractNonNullableType<DataModel::Nullable<U>>
{
    using type = typename ExtractNonNullableType<U>::type;
};
template <typename U>
using ExtractNonNullableType_t = typename ExtractNonNullableType<U>::type;

template <typename U>
struct ExtractNestedType
{
    using type = U; // Base case - not a wrapper
};
template <typename U>
struct ExtractNestedType<DataModel::Nullable<U>>
{
    using type = typename ExtractNestedType<U>::type;
};
template <typename U>
struct ExtractNestedType<DataModel::List<U>>
{
    using type = typename ExtractNestedType<U>::type;
};
template <typename U>
using ExtractNestedType_t = typename ExtractNestedType<U>::type;

/**
 * @class CTC_BaseDataClass
 * @tparam T The attribute value type (non-nullable/nullable list, struct or primitive)
 * @brief Atomic attribute data management with validation support
 *
 * @details This template class provides a robust foundation for managing attribute
 *          values in a thread-safe manner with support for atomic updates, validation,
 *          and automatic memory management. It handles three categories of attributes:
 *          1. Nullable types (DataModel::Nullable<U>, DataModel::Nullable<DataModel::List<U>>)
 *          2. List types (DataModel::List<U>)
 *          3. Primitive/struct types
 *
 * @section features Key Features
 * - Atomic value updates with validation
 * - Change detection and notification
 * - Automatic memory management for complex types
 * - Double-buffered storage for thread-safe access
 * - Strict state machine for update flow control
 * - Customizable validation hooks
 * - Support for nullable and list types
 *
 * @section type_handling Type Handling
 * The class automatically adapts its behavior based on the template type:
 * - For nullable types (DataModel::Nullable<U>):
 *   * Manages null state transitions
 *   * Handles both scalar and complex nullable values
 * - For list types (DataModel::List<U>):
 *   * Manages memory allocation/deallocation
 *   * Provides element-wise copy and cleanup
 * - For struct types:
 *   * Uses type-specific CopyData and CleanupStructValue specializations
 * - For primitive types:
 *   * Simple value storage with atomic update support
 *
 * @section memory_management Memory Management
 * The class handles automatic cleanup for:
 * - List memory (allocated via Platform::MemoryCalloc)
 * - Nested structs (via CleanupStructValue template)
 * - Nullable state transitions
 *
 * @section usage_patterns Usage Patterns
 *
 * @subsection basic_usage Basic Value Update
 * @code{.cpp}
 * CTC_BaseDataClass<Nullable<uint32_t>> data(attrId);
 * data.SetNewValue(5->aValue)
 * - CreateNewSingleValue();         // Initialize new value
 * - GetNewValueRef().SetNonNull(5); // Modify value
 * - MarkAsAssigned();               // Mark as ready for validation
 * data.UpdateBegin(nullptr);        // Validate (no context needed)
 * data.UpdateFinish(true);          // Commit changes
 * @endcode
 *
 * @subsection list_usage List Value Update
 * @code{.cpp}
 * List<DayEntryStruct> mListData = {...}
 * CTC_BaseDataClass<Nullable<List<DayEntryStruct>>> data(attrId);
 * data.SetNewValue(mListData->aValue)
 * - CreateNewListValue(aValue.Size());                 // Allocate 3 elements
 * - Loop for list items: CopyData(src_item[i], dst_item[i]);   //copy list elements...
 * - MarkAsAssigned();
 * data.UpdateBegin(validationCtx);
 * data.UpdateFinish(true);
 * @endcode
 *
 * @section update_flow Update State Machine
 * The class enforces a strict update workflow:
 *
 * @dot
 * digraph update_flow {
 *   node [shape=box, style=rounded];
 *   kIdle -> kInitialized [label="CreateNewValue()"];
 *   kInitialized -> kAssigned [label="MarkAsAssigned()"];
 *   kAssigned -> kValidated [label="UpdateBegin()"];
 *   kValidated -> kIdle [label="UpdateFinish() (mandatory)"];
 *
 *   // Error/cleanup paths
 *   kInitialized -> kIdle [label="UpdateFinish()" style="dashed"];
 *   kAssigned -> kIdle [label="UpdateFinish()" style="dashed"];
 *   kValidated -> kIdle [label="UpdateFinish()" style="dashed"];
 * }
 * @enddot
 *
 * @note UpdateFinish() must always be called to complete the update process,
 *       even if validation fails or the update is aborted.
 */
class CTC_BaseDataClassBase
{
public:
    virtual ~CTC_BaseDataClassBase() = default;

    // Common interface
    virtual bool IsValid() const                   = 0;
    virtual bool HasValue() const                  = 0;
    virtual bool HasNewValue() const               = 0;
    virtual CHIP_ERROR MarkAsAssigned()            = 0;
    virtual CHIP_ERROR UpdateBegin(void * aUpdCtx) = 0;
    virtual bool UpdateFinish(bool aUpdateAllow)   = 0;
    virtual bool Cleanup()                         = 0;
    virtual AttributeId GetAttrId() const          = 0;

    // Type-erased methods for generic access
    virtual CHIP_ERROR GetValueAsVoid(void *& outValue)        = 0;
    virtual CHIP_ERROR GetNewValueAsVoid(void *& outValue)     = 0;
    virtual CHIP_ERROR SetNewValueFromVoid(const void * value) = 0;
};

template <typename T>
class CTC_BaseDataClass : public CTC_BaseDataClassBase
{
    /// @brief Internal storage states
    enum class StorageState : uint8_t
    {
        kEmpty, // Value not initialized (default state)
        kHold,  // Storage holds valid data
    };

    /// @brief Update process states
    enum class UpdateState : uint8_t
    {
        kIdle,        // No active update
        kInitialized, // New value initialized but not populated
        kAssigned,    // New value populated but not validated
        kValidated,   // New value validated and ready for commit
    };

    T mValueStorage[2];                                          ///< Double-buffered value storage
    StorageState mHoldState[2] = { StorageState::kEmpty };       ///< Storage state tracking
    std::atomic<UpdateState> mUpdateState{ UpdateState::kIdle }; ///< Current update state
    std::atomic<uint8_t> mActiveValueIdx{ 0 };                   ///< Index of active value storage
public:
    /// The exposed attribute value type
    using ValueType = T;

    using DataType = ExtractNonNullableType_t<ValueType>;

    using ListEntryType = std::conditional_t<IsList<DataType>::value,
                                             ExtractNestedType_t<DataType>, // Extract the list element type
                                             void *>;

    using StructType = std::conditional_t<IsList<DataType>::value,
                                          ListEntryType, // Extract the list element type
                                          DataType>;

    /**
     * @brief Construct a new data class instance
     * @param[in] aAttrId Attribute ID for identification and callbacks
     *
     * @post Initializes storage based on type:
     * - Nullable types: set to null
     * - List types: initialized as empty list
     * - Others: default initialized
     * - Update state set to kIdle
     */
    explicit CTC_BaseDataClass(AttributeId aAttrId) : mAttrId(aAttrId)
    {
        if constexpr (TypeIsNullable<ValueType>())
        {
            GetValueRef().SetNull();
        }
        else if constexpr (TypeIsList<ValueType>())
        {
            GetValueRef() = ValueType();
        }
    }

    /// @brief Virtual destructor ensures proper cleanup of resources
    virtual ~CTC_BaseDataClass() { Cleanup(); }

    /**
     * @brief Get mutable reference to the active value
     * @return Reference to the currently active value storage
     * @warning Modifying the returned reference directly bypasses
     *          the update state machine and validation. Prefer using
     *          the proper update workflow.
     */
    ValueType & GetValue() { return GetValueRef(); }

    /**
     * @brief Get mutable reference to the new value (during update)
     * @return Reference to the new value storage
     * @pre Must be in kInitialized or kAssigned state
     */
    ValueType & GetNewValue() { return GetNewValueRef(); }

    /**
     * @brief Check if current update is validated
     * @return true if in kValidated state, false otherwise
     */
    bool IsValid() const override { return (mUpdateState.load() == UpdateState::kValidated); }

    /**
     * @brief Check if active storage contains valid data
     * @return true if storage is in kHold state, false otherwise
     */
    bool HasValue() const override { return (mHoldState[mActiveValueIdx.load()] == StorageState::kHold); }

    /**
     * @brief Check if new value storage contains valid data
     * @return true if new storage is in kHold state, false otherwise
     */
    bool HasNewValue() const override { return (mHoldState[1 - mActiveValueIdx.load()] == StorageState::kHold); }

    /**
     * @brief Prepares a new list value for modification
     * @param[in] size Number of elements to allocate
     * @return CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if called during active update
     * @retval CHIP_ERROR_NO_MEMORY if allocation fails
     * @retval CHIP_ERROR_INTERNAL if called on non-list type
     *
     * @post On success:
     * - Allocates memory for list elements
     * - Initializes new value storage
     * - Transitions state to kInitialized
     *
     * @note For zero size, creates an empty list or null value
     */
    CHIP_ERROR CreateNewListValue(size_t size)
    {
        if (mUpdateState.load() != UpdateState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if constexpr (TypeIsList<DataType>())
        {
            if (size >= 1)
            {
                auto * buffer = static_cast<ListEntryType *>(Platform::MemoryCalloc(size, sizeof(ListEntryType)));
                if (!buffer)
                {
                    return CHIP_ERROR_NO_MEMORY;
                }
                if constexpr (TypeIsNullable<ValueType>())
                {
                    GetNewValueRef().SetNonNull(DataModel::List<ListEntryType>(buffer, size));
                }
                else
                {
                    GetNewValueRef() = DataModel::List<ListEntryType>(buffer, size);
                }
            }
            else
            {
                return CHIP_ERROR_INVALID_LIST_LENGTH;
            }
        }
        else
        {
            return CHIP_ERROR_INTERNAL;
        }

        mUpdateState.store(UpdateState::kInitialized);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Prepares a new non-list value for modification
     * @return CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if called during active update
     * @retval CHIP_ERROR_INTERNAL if called on list type
     *
     * @post On success:
     * - Initializes new value storage
     * - Transitions state to kInitialized
     */
    CHIP_ERROR CreateNewSingleValue()
    {
        if (mUpdateState.load() != UpdateState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if constexpr (TypeIsList<DataType>())
        {
            return CHIP_ERROR_INTERNAL;
        }

        if constexpr (TypeIsNullable<ValueType>())
        {
            GetNewValueRef().SetNull();
        }
        else
        {
            GetNewValueRef() = ValueType();
        }

        mUpdateState.store(UpdateState::kInitialized);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Sets a new value from external source
     * @param[in] aValue The new value to set
     * @return CHIP_NO_ERROR on success
     *
     * @note Handles all type cases (nullable, list, struct, scalar)
     * @note Performs proper copy and memory management
     */
    CHIP_ERROR SetNewValue(const ValueType & aValue)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        // Handle nullable case first - early return for null
        if constexpr (TypeIsNullable<ValueType>())
        {
            if (aValue.IsNull())
            {
                mUpdateState.store(UpdateState::kInitialized);
                return MarkAsAssigned();
            }
        }

        // Get reference to the actual value (non-nullable part)
        const auto & actualValue = [&]() -> const auto & {
            if constexpr (TypeIsNullable<ValueType>())
            {
                return aValue.Value();
            }
            else
            {
                return aValue;
            }
        }();

        // Get reference to the storage location
        [[maybe_unused]] const auto getStorageRef = [this]() -> auto & {
            if constexpr (TypeIsNullable<ValueType>())
            {
                return GetNewValueRef().Value();
            }
            else
            {
                return GetNewValueRef();
            }
        };

        [[maybe_unused]] const auto assignStorageVal = [this](auto && value) -> auto & {
            if constexpr (TypeIsNullable<ValueType>())
            {
                return GetNewValueRef().SetNonNull(value);
            }
            else
            {
                return GetNewValueRef() = value;
            }
        };

        if constexpr (TypeIsList<DataType>())
        {
            assertChipStackLockedByCurrentThread();

            err = CreateNewListValue(actualValue.size());

            if (CHIP_NO_ERROR == err)
            {
                auto buffer = getStorageRef().data();
                for (size_t idx = 0; idx < actualValue.size(); idx++)
                {
                    if constexpr (TypeIsStruct<ListEntryType>())
                    {
                        if (CHIP_NO_ERROR == (err = CopyData(actualValue[idx], buffer[idx])))
                        {
                            continue;
                        }
                        break;
                    }
                    else
                    {
                        buffer[idx] = actualValue[idx];
                    }
                }
            }
        }
        else if constexpr (TypeIsStruct<DataType>())
        {
            err = CreateNewSingleValue();

            if (CHIP_NO_ERROR == err)
            {
                assignStorageVal(DataType()); // Default construct in place
                err = CopyData(actualValue, getStorageRef());
            }
        }
        else if constexpr (TypeIsScalar<DataType>())
        {
            err = CreateNewSingleValue();

            if (CHIP_NO_ERROR == err)
            {
                assignStorageVal(actualValue);
            }
        }

        ReturnErrorOnFailure(err);
        return MarkAsAssigned();
    }

    /**
     * @brief Signals completion of new value modifications
     * @return CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kInitialized state
     *
     * @post Transitions state to kAssigned if successful
     */
    CHIP_ERROR MarkAsAssigned() override
    {
        if (mUpdateState.load() != UpdateState::kInitialized)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        mUpdateState.store(UpdateState::kAssigned);
        mHoldState[1 - mActiveValueIdx.load()] = StorageState::kHold;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Validates and prepares the new value for commit
     * @param[in] aUpdCtx Context data for validation (may be nullptr)
     * @return CHIP_NO_ERROR if validation succeeds
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kAssigned state
     * @retval Other validation errors if validation fails
     *
     * @post On success, transitions state to kValidated
     * @note If aUpdCtx is nullptr, skips context validation
     */
    CHIP_ERROR UpdateBegin(void * aUpdCtx) override
    {
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState.load() == UpdateState::kIdle)
        {
            return CHIP_NO_ERROR;
        }

        if (mUpdateState.load() != UpdateState::kAssigned)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        CHIP_ERROR err = CHIP_NO_ERROR;

        if (aUpdCtx != nullptr)
        {
            mAuxData = aUpdCtx;
            err      = ValidateNewValue();
        }

        if (err == CHIP_NO_ERROR)
        {
            mUpdateState.store(UpdateState::kValidated);
        }
        else
        {
            ChipLogError(AppServer, "The value of attr %" PRIu32 " is not valid!", mAttrId);
        }

        return err;
    }

    /**
     * @brief Completes the update process
     * @param[in] aUpdateAllow Whether to commit the changes
     * @return true if value changed, false otherwise
     *
     * @post Always transitions state to kIdle
     * @note Performs cleanup of unused storage
     */
    bool UpdateFinish(bool aUpdateAllow) override
    {
        bool ret = false;
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState.load() == UpdateState::kIdle)
        {
            return false;
        }

        if (aUpdateAllow && (mUpdateState.load() == UpdateState::kValidated))
        {
            SwapActiveValueStorage();

            ret = HasChanged();
        }

        CleanupByIdx(1 - mActiveValueIdx.load());

        mUpdateState.store(UpdateState::kIdle);

        return ret;
    }

    /**
     * @brief Full cleanup of all value storage
     * @return true if active value was changed, false otherwise
     */
    bool Cleanup() override
    {
        bool ret = false;
        if (HasValue())
        {
            ret = true;
        }

        CleanupByIdx(1 - mActiveValueIdx.load());
        CleanupByIdx(mActiveValueIdx.load());

        return ret;
    }

    /**
     * @brief Cleans up an external list entry
     * @param[in,out] entry The list entry to clean up
     */
    void CleanupExtListEntry(ListEntryType & entry) { CleanupStruct(entry); }

    /**
     * @brief Gets the attribute ID
     * @return The attribute ID this instance manages
     */
    AttributeId GetAttrId() const override { return mAttrId; }

    // Type-erased implementations
    CHIP_ERROR GetValueAsVoid(void *& outValue) override
    {
        outValue = static_cast<void *>(&GetValueRef());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetNewValueAsVoid(void *& outValue) override
    {
        outValue = static_cast<void *>(&GetNewValueRef());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetNewValueFromVoid(const void * value) override
    {
        if (value == nullptr)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        const ValueType * typedValue = static_cast<const ValueType *>(value);
        return SetNewValue(*typedValue);
    }

private:
    /**
     * @brief Gets reference to active value storage
     * @return Reference to active storage
     */
    ValueType & GetValueRef() { return mValueStorage[mActiveValueIdx.load()]; }

    /**
     * @brief Gets reference to new value storage
     * @return Reference to new storage
     */
    ValueType & GetNewValueRef() { return mValueStorage[1 - mActiveValueIdx.load()]; }

    /**
     * @brief Swaps active and new value storage indices
     */
    void SwapActiveValueStorage() { mActiveValueIdx.store(1 - mActiveValueIdx.load()); }

    /**
     * @brief Checks if value type is nullable
     * @return true if nullable, false otherwise
     */
    template <typename U>
    static constexpr bool TypeIsNullable()
    {
        return IsNullable<U>::value;
    }

    /**
     * @brief Checks if value type is a list
     * @return true if list type, false otherwise
     */
    template <typename U>
    static constexpr bool TypeIsList()
    {
        return IsList<U>::value;
    }

    /**
     * @brief Checks if value type is a struct
     * @return true if struct type, false otherwise
     */
    template <typename U>
    static constexpr bool TypeIsStruct()
    {
        return IsStruct<U>::value;
    }

    /**
     * @brief Checks if value type is scalar (numeric or enum)
     * @return true if scalar type, false otherwise
     */
    template <typename U>
    static constexpr bool TypeIsScalar()
    {
        return (IsNumeric<U>::value || IsEnum<U>::value);
    }

    // Internal implementation methods...

    CHIP_ERROR CopyData(const StructType & input, StructType & output);

    /**
     * @brief Validates the new value using type-specific validation
     * @return CHIP_ERROR Validation result
     */
    CHIP_ERROR ValidateNewValue();

    /**
     * @brief Compares two lists for equality
     * @param source First list to compare
     * @param destination Second list to compare
     * @return true if lists differ, false if identical
     */
    bool ListsNotEqual(const DataModel::List<ListEntryType> & source, const DataModel::List<ListEntryType> & destination)
    {
        if (source.size() != destination.size())
        {
            return true;
        }

        assertChipStackLockedByCurrentThread();

        for (size_t i = 0; i < source.size(); i++)
        {
            if (source[i] != destination[i])
            {
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Compares two nullable values for equality
     * @param a First value to compare
     * @param b Second value to compare
     * @return true if values differ, false if identical
     */
    bool NullableNotEqual(const ValueType & a, const ValueType & b)
    {
        bool is_neq = false;
        if (a.IsNull() || b.IsNull())
        {
            is_neq = a.IsNull() != b.IsNull();
        }
        else
        {
            if constexpr (IsList<DataType>::value)
            {
                is_neq = ListsNotEqual(a.Value(), b.Value());
            }
            else
            {
                is_neq = (a.Value() != b.Value());
            }
        }

        return is_neq;
    }

    /**
     * @brief Checks if new value differs from current value
     * @return true if values differ, false if identical
     */
    bool HasChanged()
    {
        if constexpr (TypeIsNullable<ValueType>())
        {
            return NullableNotEqual(GetNewValueRef(), GetValueRef());
        }
        else if constexpr (TypeIsList<ValueType>())
        {
            return ListsNotEqual(GetNewValueRef(), GetValueRef());
        }
        else
        {
            return GetNewValueRef() != GetValueRef();
        }
    }

    /**
     * @brief Cleans up storage at specified index
     * @param aIdx Storage index to clean (0 or 1)
     */
    void CleanupByIdx(uint8_t aIdx)
    {
        if (mActiveValueIdx == aIdx)
        {
            CleanupValueByRef(mValueStorage[mActiveValueIdx.load()]);
            mHoldState[mActiveValueIdx.load()] = StorageState::kEmpty;
        }
        else
        {
            CleanupValueByRef(mValueStorage[1 - mActiveValueIdx.load()]);
            mHoldState[1 - mActiveValueIdx.load()] = StorageState::kEmpty;
        }
    }

    /**
     * @brief Cleans up a value reference
     * @param aValue Reference to value to clean up
     */
    void CleanupValueByRef(ValueType & aValue)
    {
        if constexpr (TypeIsNullable<ValueType>())
        {
            if (!aValue.IsNull())
            {
                if constexpr (TypeIsList<DataType>())
                {
                    CleanupList(aValue.Value());
                }
                else if constexpr (TypeIsStruct<DataType>())
                {
                    CleanupStruct(aValue.Value());
                }
            }
            aValue.SetNull();
        }
        else if constexpr (TypeIsList<ValueType>())
        {
            CleanupList(aValue);
        }
    }

    /**
     * @brief Cleans up a list and its elements
     * @param list List to clean up
     */
    void CleanupList(DataModel::List<ListEntryType> & list)
    {
        assertChipStackLockedByCurrentThread();

        if constexpr (TypeIsStruct<ListEntryType>())
        {

            for (auto & item : list)
            {
                CleanupStruct(item);
            }
        }

        if (list.data())
        {
            Platform::MemoryFree(list.data());
            list = DataModel::List<ListEntryType>();
        }
    }

    /**
     * @brief Cleans up a struct value
     * @param aValue Struct to clean up
     */
    void CleanupStruct(StructType & aValue);

    //{
    // CleanupStructValue<StructType>(aValue);
    //}

    void * mAuxData = nullptr; ///< Validation context data
    const AttributeId mAttrId; ///< Managed attribute ID
};

} // namespace CommodityTariffAttrsDataMgmt

namespace Clusters {
namespace CommodityTariff {
/**
 * @struct TariffUpdateCtx
 * @brief Context for validating tariff attribute updates and maintaining referential integrity
 *
 * This structure tracks relationships between tariff components during attribute updates
 * to ensure all references are valid and consistent. It serves as a validation context
 * that collects all IDs and references before checking their consistency.
 *
 * @section references Referential Integrity Tracking
 * The context maintains several sets of IDs to validate that:
 * - All referenced DayEntry IDs exist in the master set
 * - All referenced TariffComponent IDs exist in the master set
 * - All referenced DayPattern IDs exist in the master set
 * - No dangling references exist between tariff components
 *
 * @section lifecycle Lifecycle
 * - Created at the start of a tariff update operation
 * - Populated during attribute parsing/processing
 * - Used for validation before committing changes
 * - Destroyed after update completion
 */
struct TariffUpdateCtx
{
    BlockModeEnum blockMode;

    /**
     * @brief Reference to the tariff's start timestamp
     * @note This is a reference to allow validation against the actual attribute value
     */
    DataModel::Nullable<uint32_t> & TariffStartTimestamp;

    /// @name DayEntry ID Tracking
    /// @{
    /**
     * @brief Master set of all valid DayEntry IDs
     * @details Contains all DayEntry IDs that exist in the tariff definition
     */
    std::unordered_set<uint32_t> DayEntryKeyIDs;

    /**
     * @brief DayEntry IDs referenced by DayPattern items
     * @details Collected separately for reference validation
     */
    std::unordered_set<uint32_t> DayPatternsDayEntryIDs;

    /**
     * @brief DayEntry IDs referenced by IndividualDays items
     * @details Collected separately for reference validation
     */
    std::unordered_set<uint32_t> IndividualDaysDayEntryIDs;

    /**
     * @brief DayEntry IDs referenced by TariffPeriod items
     * @details Collected separately for reference validation
     */
    std::unordered_set<uint32_t> TariffPeriodsDayEntryIDs;

    /// @}

    /// @name TariffComponent ID Tracking
    /// @{
    /**
     * @brief Master set of all valid TariffComponent IDs
     * @details Contains all TariffComponent IDs that exist in the tariff definition
     */
    std::unordered_map<uint32_t, uint32_t> TariffComponentKeyIDsFeatureMap;

    /**
     * @brief TariffComponent IDs referenced by TariffPeriod items
     * @details Collected for validating period->component references
     */
    std::unordered_set<uint32_t> TariffPeriodsTariffComponentIDs;
    /// @}

    /// @name DayPattern ID Tracking
    /// @{
    /**
     * @brief Master set of all valid DayPattern IDs
     * @details Contains all DayPattern IDs that exist in the tariff definition
     */
    std::unordered_set<uint32_t> DayPatternKeyIDs;

    /**
     * @brief DayPattern IDs referenced by CalendarPeriod items
     * @details Collected for validating calendar->pattern references
     */
    std::unordered_set<uint32_t> CalendarPeriodsDayPatternIDs;
    /// @}

    /**
     * @brief Bitmask of active tariff features
     * @details Used to validate feature-dependent constraints
     */
    BitMask<Feature> mFeature;

    /**
     * @brief Timestamp when the tariff update was initiated
     * @note Used for change tracking and versioning
     */
    uint32_t TariffUpdateTimestamp;
};

/**
 * @brief Primary attributes for Commodity Tariff
 *
 * Primary attributes represent the fundamental tariff configuration that can only
 * be changed by authorized tariff updates. These are typically set by utility providers.
 */

// Generated classes for each attribute:

class TariffUnitDataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<Globals::TariffUnitEnum>>
{
public:
    TariffUnitDataClass() : CTC_BaseDataClass<DataModel::Nullable<Globals::TariffUnitEnum>>(Attributes::TariffUnit::Id) {}
    ~TariffUnitDataClass() override = default;
};

class StartDateDataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<uint32_t>>
{
public:
    StartDateDataClass() : CTC_BaseDataClass<DataModel::Nullable<uint32_t>>(Attributes::StartDate::Id) {}
    ~StartDateDataClass() override = default;
};

class DefaultRandomizationOffsetDataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<int16_t>>
{
public:
    DefaultRandomizationOffsetDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<int16_t>>(Attributes::DefaultRandomizationOffset::Id)
    {}
    ~DefaultRandomizationOffsetDataClass() override = default;
};

class DefaultRandomizationTypeDataClass
    : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<DayEntryRandomizationTypeEnum>>
{
public:
    DefaultRandomizationTypeDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DayEntryRandomizationTypeEnum>>(Attributes::DefaultRandomizationType::Id)
    {}
    ~DefaultRandomizationTypeDataClass() override = default;
};

class TariffInfoDataClass
    : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<Structs::TariffInformationStruct::Type>>
{
public:
    TariffInfoDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<Structs::TariffInformationStruct::Type>>(Attributes::TariffInfo::Id)
    {}
    ~TariffInfoDataClass() override = default;
};

class DayEntriesDataClass
    : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::DayEntryStruct::Type>>>
{
public:
    DayEntriesDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::DayEntryStruct::Type>>>(Attributes::DayEntries::Id)
    {}
    ~DayEntriesDataClass() override = default;
};

class DayPatternsDataClass
    : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::DayPatternStruct::Type>>>
{
public:
    DayPatternsDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::DayPatternStruct::Type>>>(Attributes::DayPatterns::Id)
    {}
    ~DayPatternsDataClass() override = default;
};

class TariffComponentsDataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<
                                      DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>>
{
public:
    TariffComponentsDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>>(
            Attributes::TariffComponents::Id)
    {}
    ~TariffComponentsDataClass() override = default;
};

class TariffPeriodsDataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<
                                   DataModel::Nullable<DataModel::List<Structs::TariffPeriodStruct::Type>>>
{
public:
    TariffPeriodsDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::TariffPeriodStruct::Type>>>(Attributes::TariffPeriods::Id)
    {}
    ~TariffPeriodsDataClass() override = default;
};

class IndividualDaysDataClass
    : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::DayStruct::Type>>>
{
public:
    IndividualDaysDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::DayStruct::Type>>>(Attributes::IndividualDays::Id)
    {}
    ~IndividualDaysDataClass() override = default;
};

class CalendarPeriodsDataClass : public CommodityTariffAttrsDataMgmt::CTC_BaseDataClass<
                                     DataModel::Nullable<DataModel::List<Structs::CalendarPeriodStruct::Type>>>
{
public:
    CalendarPeriodsDataClass() :
        CTC_BaseDataClass<DataModel::Nullable<DataModel::List<Structs::CalendarPeriodStruct::Type>>>(
            Attributes::CalendarPeriods::Id)
    {}
    ~CalendarPeriodsDataClass() override = default;
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
