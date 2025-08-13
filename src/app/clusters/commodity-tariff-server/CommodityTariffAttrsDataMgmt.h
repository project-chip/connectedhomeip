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

#define SCALAR_ATTRS                                                                                                               \
    X(Clusters::Globals::TariffUnitEnum)                                                                                           \
    X(uint32_t)                                                                                                                    \
    X(int16_t)                                                                                                                     \
    X(Clusters::CommodityTariff::DayEntryRandomizationTypeEnum)

#define COMPLEX_ATTRIBUTES                                                                                                         \
    X(Clusters::CommodityTariff::Structs::TariffInformationStruct::Type)                                                           \
    X(DataModel::List<Clusters::CommodityTariff::Structs::DayEntryStruct::Type>)                                                   \
    X(DataModel::List<Clusters::CommodityTariff::Structs::DayPatternStruct::Type>)                                                 \
    X(DataModel::List<Clusters::CommodityTariff::Structs::TariffComponentStruct::Type>)                                            \
    X(DataModel::List<Clusters::CommodityTariff::Structs::TariffPeriodStruct::Type>)                                               \
    X(DataModel::List<Clusters::CommodityTariff::Structs::DayStruct::Type>)                                                        \
    X(DataModel::List<Clusters::CommodityTariff::Structs::CalendarPeriodStruct::Type>)

#define ALL_ATTRIBUTES                                                                                                             \
    SCALAR_ATTRS                                                                                                                   \
    COMPLEX_ATTRIBUTES

template <typename T>
CHIP_ERROR CopyData(const T & input, T & output);

// Declare the specializations (no definitions here)
#define X(attrType)                                                                                                                \
    template <>                                                                                                                    \
    CHIP_ERROR CopyData<attrType>(const attrType & input, attrType & output);
COMPLEX_ATTRIBUTES
#undef X

// Primary template declaration
template <typename T>
void CleanupStructValue(T & aValue);

// Specialization declarations
#define X(attrType)                                                                                                                \
    template <>                                                                                                                    \
    void CleanupStructValue<attrType>(attrType & aValue);
COMPLEX_ATTRIBUTES
#undef X

template <typename T>
CHIP_ERROR Validate(const T & aValue, void * aCtx);

#define X(attrType)                                                                                                                \
    template <>                                                                                                                    \
    CHIP_ERROR Validate<DataModel::Nullable<attrType>>(const DataModel::Nullable<attrType> & aValue, void * aCtx);
ALL_ATTRIBUTES
#undef X

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
 * - Loop for list items: CopyData<ListEntryType>(src_item[i], dst_item[i]);   //copy list elements...
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
template <typename T>
class CTC_BaseDataClass
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

    T mValueStorage[2];                                         ///< Double-buffered value storage
    StorageState mHoldState[2] = { StorageState::kEmpty };      ///< Storage state tracking
    std::atomic<UpdateState> mUpdateState{UpdateState::kIdle};  ///< Current update state
    std::atomic<uint8_t> mActiveValueIdx{0};                    ///< Index of active value storage    
public:
    /// The exposed attribute value type
    using ValueType = T;

    /// The non-nullable version of the value type
    using NonNullableType = ExtractNonNullableType_t<ValueType>;
    using ListEntryType   = std::conditional_t<IsList<NonNullableType>::value,
                                             ExtractNestedType_t<NonNullableType>, // Extract the list element type
                                             void *>;

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
        if constexpr (IsValueNullable())
        {
            GetValueRef().SetNull();
        }
        else if constexpr (IsValueList())
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
    bool IsValid() const { return (mUpdateState.load() == UpdateState::kValidated); }

    /**
     * @brief Check if active storage contains valid data
     * @return true if storage is in kHold state, false otherwise
     */
    bool HasValue() const { return (mHoldState[mActiveValueIdx.load()] == StorageState::kHold); }

    /**
     * @brief Check if new value storage contains valid data
     * @return true if new storage is in kHold state, false otherwise
     */
    bool HasNewValue() const { return (mHoldState[1-mActiveValueIdx.load()] == StorageState::kHold); }

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

        if constexpr (!IsValueList())
        {
            return CHIP_ERROR_INTERNAL;
        }

        if (size >= 1)
        {
            auto * buffer = static_cast<ListEntryType *>(Platform::MemoryCalloc(size, sizeof(ListEntryType)));
            if (!buffer)
            {
                return CHIP_ERROR_NO_MEMORY;
            }
            if constexpr (IsValueNullable())
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
            if constexpr (IsValueNullable())
            {
                GetNewValueRef().SetNull();
            }
            else
            {
                GetNewValueRef() = DataModel::List<ListEntryType>();
            }
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

        if (aValue.IsNull())
        {
            CleanupByIdx(1-mActiveValueIdx.load());
            mUpdateState.store(UpdateState::kInitialized);
            MarkAsAssigned();
            return CHIP_NO_ERROR;
        }

        auto & newValue = aValue.Value();

        if constexpr (IsValueList())
        {
            assertChipStackLockedByCurrentThread();

            err = CreateNewListValue(newValue.size());

            if (CHIP_NO_ERROR == err)
            {
                auto buffer = GetNewValueRef().Value().data();
                for (size_t idx = 0; idx < newValue.size(); idx++)
                {
                    if (CHIP_NO_ERROR == (err = CopyData<ListEntryType>(newValue[idx], buffer[idx])))
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
                NonNullableType tmpValue = newValue;
                err                      = CopyData<NonNullableType>(newValue, tmpValue);
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
            mHoldState[1-mActiveValueIdx.load()] = StorageState::kHold;
            MarkAsAssigned();
        }
        else
        {
            CleanupByIdx(1-mActiveValueIdx.load());
        }

        return err;
    }

    /**
     * @brief Signals completion of new value modifications
     * @return CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kInitialized state
     *
     * @post Transitions state to kAssigned if successful
     */
    CHIP_ERROR MarkAsAssigned()
    {
        if (mUpdateState.load() != UpdateState::kInitialized)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        mUpdateState.store(UpdateState::kAssigned);
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
    CHIP_ERROR UpdateBegin(void * aUpdCtx)
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
            ChipLogError(NotSpecified, "The value of attr %d is not valid!", mAttrId);
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
    bool UpdateFinish(bool aUpdateAllow)
    {
        bool ret = false;
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState.load() == UpdateState::kIdle)
        {
            return false;
        }

        if (aUpdateAllow && (mUpdateState.load() == UpdateState::kValidated) && (HasChanged()))
        {
            if (HasValue())
            {
                CleanupByIdx(mActiveValueIdx.load()); // Cleanup current value
            }

            SwapActiveValueStorage();

            ret = true;
        }
        else
        {
            CleanupByIdx(1-mActiveValueIdx.load());
        }

        mUpdateState.store(UpdateState::kIdle);

        return ret;
    }

    /**
     * @brief Full cleanup of all value storage
     * @return true if active value was changed, false otherwise
     */
    bool Cleanup()
    {
        bool ret = false;
        if (HasValue())
        {
            ret = true;
        }

        CleanupByIdx(1-mActiveValueIdx.load());
        CleanupByIdx(mActiveValueIdx.load());

        return ret;
    }

    /**
     * @brief Cleans up an external list entry
     * @param[in,out] entry The list entry to clean up
     */
    void CleanupExtListEntry(ListEntryType & entry) { CleanupStructValue<ListEntryType>(entry); }

    /**
     * @brief Gets the attribute ID
     * @return The attribute ID this instance manages
     */
    AttributeId GetAttrId() { return mAttrId; }

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
    ValueType & GetNewValueRef() { return mValueStorage[1-mActiveValueIdx.load()]; }

    /**
     * @brief Swaps active and new value storage indices
     */
    void SwapActiveValueStorage() { mActiveValueIdx.store(1-mActiveValueIdx.load()); }

    /**
     * @brief Checks if value type is nullable
     * @return true if nullable, false otherwise
     */
    static constexpr bool IsValueNullable() { return IsNullable<ValueType>::value; }

    /**
     * @brief Checks if value type is a list
     * @return true if list type, false otherwise
     */
    static constexpr bool IsValueList() { return IsList<NonNullableType>::value; }

    /**
     * @brief Checks if value type is a struct
     * @return true if struct type, false otherwise
     */
    static constexpr bool IsValueStruct() { return IsStruct<NonNullableType>::value; }

    /**
     * @brief Checks if value type is scalar (numeric or enum)
     * @return true if scalar type, false otherwise
     */
    static constexpr bool IsValueScalar() { return (IsNumeric<NonNullableType>::value || IsEnum<NonNullableType>::value); }

    // Internal implementation methods...

    /**
     * @brief Validates the new value using type-specific validation
     * @return CHIP_ERROR Validation result
     */
    CHIP_ERROR ValidateNewValue() { return Validate<ValueType>(GetNewValueRef(), mAuxData); }

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
            if constexpr (IsList<NonNullableType>::value)
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
            CleanupValueByRef(mValueStorage[1-mActiveValueIdx.load()]);
            mHoldState[1-mActiveValueIdx.load()] = StorageState::kEmpty;
        }
    }

    /**
     * @brief Cleans up a value reference
     * @param aValue Reference to value to clean up
     */
    void CleanupValueByRef(ValueType & aValue)
    {
        if constexpr (IsValueNullable())
        {
            if (!aValue.IsNull())
            {
                if constexpr (IsList<NonNullableType>::value)
                {
                    CleanupList(aValue.Value());
                }
                else if constexpr (IsValueStruct())
                {
                    CleanupStruct<NonNullableType>(aValue.Value());
                }
            }
            aValue.SetNull();
        }
        else if constexpr (IsValueList())
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

        for (auto & item : list)
        {
            CleanupStruct<ListEntryType>(item);
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
    template <typename StructType>
    void CleanupStruct(StructType & aValue)
    {
        CleanupStructValue<StructType>(aValue);
    }

    void * mAuxData = nullptr;                     ///< Validation context data
    const AttributeId mAttrId;                     ///< Managed attribute ID
};

} // namespace CommodityTariffAttrsDataMgmt
} // namespace app
} // namespace chip
