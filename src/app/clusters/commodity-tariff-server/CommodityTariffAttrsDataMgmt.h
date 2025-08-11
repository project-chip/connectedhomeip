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
} // namespace namespace AuxiliaryLoadSwitchSettingsStruct

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
    return (lhs.currency == rhs.currency) &&
           (lhs.tariffLabel == rhs.tariffLabel) &&
           (lhs.providerName == rhs.providerName) &&
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
    return (lhs.dayPatternID == rhs.dayPatternID) &&
           (lhs.daysOfWeek.Raw() == rhs.daysOfWeek.Raw()) &&
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
    return (lhs.label == rhs.label) &&
           (lhs.price == rhs.price) &&
           (lhs.friendlyCredit == rhs.friendlyCredit) &&
           (lhs.auxiliaryLoad == rhs.auxiliaryLoad) &&
           (lhs.peakPeriod == rhs.peakPeriod) &&
           (lhs.powerThreshold == rhs.powerThreshold) &&
           (lhs.predicted == rhs.predicted) &&
           (lhs.threshold == rhs.threshold);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace TariffComponentStruct

namespace TariffPeriodStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.tariffComponentIDs == rhs.tariffComponentIDs) &&
           (lhs.dayEntryIDs == rhs.dayEntryIDs) &&
           (lhs.label == rhs.label);
}

inline bool operator!=(const Type & lhs, const Type & rhs)
{
    return !(lhs == rhs);
}
} // namespace TariffPeriodStruct

namespace CalendarPeriodStruct {
inline bool operator==(const Type & lhs, const Type & rhs)
{
    return (lhs.startDate == rhs.startDate) &&
           (lhs.dayPatternIDs == rhs.dayPatternIDs);
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

#define SCALAR_ATTRS                    \
    X(Clusters::Globals::TariffUnitEnum)                  \
    X(uint32_t)                         \
    X(int16_t)                          \
    X(Clusters::CommodityTariff::DayEntryRandomizationTypeEnum)

#define COMPLEX_ATTRIBUTES                          \
    X(Clusters::CommodityTariff::Structs::TariffInformationStruct::Type)                \
    X(DataModel::List<Clusters::CommodityTariff::Structs::DayEntryStruct::Type>)        \
    X(DataModel::List<Clusters::CommodityTariff::Structs::DayPatternStruct::Type>)      \
    X(DataModel::List<Clusters::CommodityTariff::Structs::TariffComponentStruct::Type>) \
    X(DataModel::List<Clusters::CommodityTariff::Structs::TariffPeriodStruct::Type>)    \
    X(DataModel::List<Clusters::CommodityTariff::Structs::DayStruct::Type>)             \
    X(DataModel::List<Clusters::CommodityTariff::Structs::CalendarPeriodStruct::Type>)

#define ALL_ATTRIBUTES                                                                   \
    SCALAR_ATTRS                                                                         \
    COMPLEX_ATTRIBUTES

template <typename T>
CHIP_ERROR CopyData(const T& input, T& output);

// Declare the specializations (no definitions here)
#define X(attrType) template <> \
CHIP_ERROR CopyData<attrType>(const attrType & input, attrType & output);
COMPLEX_ATTRIBUTES
#undef X

// Primary template declaration
template <typename T>
void CleanupStructValue(T& aValue);

// Specialization declarations
#define X(attrType) template <> \
void CleanupStructValue<attrType>(attrType & aValue);
COMPLEX_ATTRIBUTES
#undef X

template <typename T>
CHIP_ERROR Validate(const T& aValue, void * aCtx);

#define X(attrType) template <> \
CHIP_ERROR Validate<DataModel::Nullable<attrType>>(const DataModel::Nullable<attrType>& aValue, void * aCtx);
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
 * - Nested structs (via CleanupStructValue template)
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

    /**
     * @brief Prepares a new value for modification
     * @param size Number of elements to allocate (for list types only)
     * @return CHIP_NO_ERROR on success, or:
     *         - CHIP_ERROR_NO_MEMORY if allocation fails
     *         - CHIP_ERROR_INCORRECT_STATE if called during active update
     *         - CHIP_ERROR_INTERNAL if wrong type data
     *
     * @note For non-list types:
     *       - Creates default-initialized value
     *       - Ignores size parameter
     */
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
     * @param aUpdCtx Pointer to context data for implementing validation methods that are used when bulk updating attributes (for cross validation f.e)
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
     * @param aUpdateAllow allows the upper level of the application to determine the need to update the attribute value.
     * @return The return value indicates that the stored value has changed.
     * @note Performs cleanup and resets to idle state
     */
    bool UpdateFinish(bool aUpdateAllow)
    {
        bool ret = false;
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState == UpdateState::kIdle)
        {
            return false;
        }

        if (aUpdateAllow && (mUpdateState == UpdateState::kValidated) && (HasChanged()))
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

    void CleanupExtListEntry(PayloadType & entry)
    {
        CleanupStructValue<PayloadType>(entry);
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

    CHIP_ERROR ValidateNewValue() { return Validate<ValueType>(GetNewValueRef(), mAuxData); }

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

    void CleanupStruct(PayloadType & aValue) { CleanupStructValue<PayloadType>(aValue); }

protected:
    ValueType mValueStorage[2]; // Double-buffered storage
    StorageState mHoldState[2]  = {StorageState::kEmpty};

    void * mAuxData = nullptr;                  // Validation context
    const AttributeId mAttrId;                     // Attribute identifier

    UpdateState mUpdateState = UpdateState::kIdle;

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
            if (source[i] != destination[i])
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
            else
            {
                is_neq = (a.Value() != b.Value());
            }
        }

        return is_neq;
    }
};

} // namespace CommodityTariffAttrsDataMgmt
} // namespace app
} // namespace chip
