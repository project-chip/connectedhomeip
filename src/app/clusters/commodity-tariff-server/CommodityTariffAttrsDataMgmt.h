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

        char * buffer = static_cast<char *>(Platform::MemoryCalloc(1, source.size() + 1));
        if (!buffer)
            return false;

        std::copy(source.begin(), source.end(), buffer);
        buffer[source.size()] = '\0';
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

        char * buffer = static_cast<char *>(Platform::MemoryAlloc(source.size() + 1));
        if (!buffer)
            return CHIP_ERROR_NO_MEMORY;

        memcpy(buffer, source.data(), source.size());
        buffer[source.size()] = '\0';
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
// Type extraction utilities (as shown previously)
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
// Type categorization traits
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

/**
 * @class CTC_BaseDataClass
 * @tparam T The attribute value type (nullable, list or primitive)
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
 * @defgroup tariff_update_flow Tariff Data Update Flow
 * @brief State machine for managing atomic attribute updates
 *
 * The update process follows a strict state sequence:
 *
 * @dot
 * digraph update_flow {
 *   kIdle -> kInitialized [label="CreateNewValue()"];
 *   kInitialized -> kAssigned [label="MarkAsAssigned()"];
 *   kAssigned -> kValidated [label="UpdateBegin()"];
 *   kValidated -> kUpdated  [label="UpdateCommit()"];
 *   kUpdated -> kIdle  [label="UpdateEnd()"];
 *   kInitialized -> kIdle [label="UpdateEnd()" style="dashed"];
 *   kAssigned -> kIdle [label="UpdateEnd()" style="dashed"];
 *   kValidated -> kIdle [label="UpdateEnd()" style="dashed"];
 * }
 * @enddot
 *
 * ### Typical Update Sequence:
 *
 * 1. **Initialization Phase**:
 *    @code
 *    /// Create new value container
 *    err = dataObj.CreateNewValue(elementCount);
 *    @endcode
 *    - Allocates memory for list types
 *    - Initializes default values
 *    - State: kIdle → kInitialized
 *
 * 2. **Modification Phase**:
 *    @code
 *    /// Get and modify the value
 *    if (auto* newVal = dataObj.GetNewValueData()) {
 *        *newVal = configuredValue;
 *        err = dataObj.MarkAsAssigned();
 *    }
 *    @endcode
 *    - State: kInitialized → kAssigned
 *
 * 3. **Validation Phase**:
 *    @code
 *    /// Validate and prepare for commit
 *    err = dataObj.UpdateBegin(context, callback, valid_is_req);
 *    @endcode
 *    - Runs custom validation
 *    - State: kAssigned → kValidated
 *
 * 4. **Commit Phase**:
 *    @code
 *    /// Finalize the update
 *    if (CHIP_NO_ERROR == err) {
 *        dataObj.UpdateCommit(); // Or UpdateEnd() on failure
 *    }
 *    @endcode
 *    - Atomically swaps value buffers if validated
 *    - Invokes callback on success
 *    - State: kValidated → kUpdated → kIdle
 *
 * ### Error Recovery:
 * - At any point, UpdateEnd() can be called to:
 *   - Discard pending changes (dashed transitions in diagram)
 *   - Clean up allocated resources
 *   - Reset to kIdle state
 *
 * ### State Transition Rules:
 * | Current State    | Valid Operations                     |
 * |------------------|--------------------------------------|
 * | kIdle            | CreateNewValue()                     |
 * | kInitialized     | GetNewValueData(), MarkAsAssigned(), UpdateEnd() |
 * | kAssigned        | UpdateBegin(), UpdateEnd()           |
 * | kValidated       | UpdateCommit(), UpdateEnd()          |
 * | kUpdated         | UpdateEnd()                          |
 *
 * @see CreateNewValue()
 * @see GetNewValueData()
 * @see MarkAsAssigned()
 * @see UpdateBegin()
 * @see UpdateCommit()
 * @see UpdateEnd()
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
    explicit CTC_BaseDataClass(uint32_t aAttrId) : mAttrId(aAttrId)
    {
        if constexpr (IsValueNullable())
        {
            GetValueRef().SetNull();
        }
        else if constexpr (IsValueList())
        {
            GetValueRef() = ValueType();
        }
        mHoldState   = StorageState::kEmpty;
        mUpdateState = UpdateState::kIdle;
    }

    /// @brief Virtual destructor for proper cleanup
    virtual ~CTC_BaseDataClass() { Cleanup(); }

    /**
     * @brief Get mutable reference to stored value
     * @return T& Reference to the active value storage
     * @warning Not thread-safe for write operations
     */
    T & GetValue() { return GetValueRef(); }

    /**
     * @brief Get const reference to stored value
     * @return const T& Const reference to the active value storage
     * @note Thread-safe for concurrent reads
     */
    const T & GetValue() const { return GetValueRef(); }

    /**
     * @brief Get pointer to the raw payload data
     * @return ExtractPayloadType_t<ValueType>* Pointer to payload if available
     * @retval nullptr if:
     *         - Not in initialized state
     *         - Value is null (for nullable types)
     *         - List is empty (for list types)
     */
    auto GetValueData() -> ExtractPayloadType_t<ValueType> *
    {
        if (mUpdateState < UpdateState::kInitialized)
        {
            return nullptr;
        }

        if constexpr (IsValueNullable())
        {
            if (GetValueRef().IsNull())
            {
                return nullptr;
            }
            if constexpr (IsList<WrappedType>::value)
            {
                return GetValueRef().Value().data();
            }
            else
            {
                return &GetValueRef().Value();
            }
        }
        else if constexpr (IsValueList())
        {
            return GetValueRef().data();
        }
        else
        {
            return &GetValueRef();
        }
    }

    /**
     * @brief Check if current update is validated
     * @return true if in kValidated state
     */
    bool IsValid() const { return (mUpdateState == UpdateState::kValidated); }

    /**
     * @brief Check if value storage contains valid data
     * @return true if storage is in kHold state
     */
    bool HasValue() const { return (mHoldState == StorageState::kHold); }

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
    CHIP_ERROR CreateNewValue(size_t size)
    {
        if (mUpdateState != UpdateState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if constexpr (IsValueList() || IsList<WrappedType>::value)
        {
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
            }
        }
        else if constexpr (IsValueNullable())
        {
            WrappedType tmp = WrappedType();
            GetNewValueRef().SetNonNull(tmp);
        }
        else
        {
            GetNewValueRef() = ValueType();
        }

        mUpdateState = UpdateState::kInitialized;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Accesses the pending new value's payload for modification
     * @return Pointer to modifiable payload value if available
     * @retval nullptr if:
     *         - Not in kInitialized state
     *         - Value is null (for nullable types)
     *         - List is empty (for list types)
     *
     * @warning Pointer becomes invalid after UpdateCommit()/UpdateEnd()
     */
    auto GetNewValueData() -> ExtractPayloadType_t<ValueType> *
    {
        if (mUpdateState < UpdateState::kInitialized)
        {
            return nullptr;
        }

        if constexpr (IsValueNullable())
        {
            if (GetNewValueRef().IsNull())
            {
                return nullptr;
            }
            if constexpr (IsList<WrappedType>::value)
            {
                return GetNewValueRef().Value().data();
            }
            else
            {
                return &GetNewValueRef().Value();
            }
        }
        else if constexpr (IsValueList())
        {
            return GetNewValueRef().data();
        }
        else
        {
            return &GetNewValueRef();
        }
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
     * @param aUpdCb Callback to invoke on successful commit
     * @return CHIP_NO_ERROR if validation succeeds
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kAssigned state
     */
    CHIP_ERROR UpdateBegin(void * aUpdCtx, void (*aUpdCb)(uint32_t, void *), bool valid_is_req)
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

        // Bypass validation if ctx not set
        if (valid_is_req && aUpdCtx != nullptr)
        {
            mAuxData = aUpdCtx;
            err      = ValidateNewValue();
        }

        if (err == CHIP_NO_ERROR)
        {
            if (aUpdCb != nullptr)
            {
                mAuxCb = aUpdCb;
            }

            mUpdateState = UpdateState::kValidated;
        }
        else
        {
            ChipLogError(NotSpecified, "The value of attr %d is not valid!", mAttrId);
        }

        return err;
    }

    /**
     * @brief Commits the validated new value
     * @note Performs atomic swap if value changed
     */
    void UpdateCommit()
    {
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState == UpdateState::kIdle)
        {
            return;
        }

        assert(mUpdateState == UpdateState::kValidated);

        if (HasChanged())
        {
            if (mHoldState == StorageState::kHold)
            {
                Cleanup(); // Cleanup current value
            }
            mHoldState   = StorageState::kHold;
            mUpdateState = UpdateState::kUpdated;
        }

        UpdateEnd();
    }

    /**
     * @brief Aborts the current update process
     * @note Performs cleanup and resets to idle state
     */
    void UpdateEnd()
    {
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState == UpdateState::kIdle)
        {
            return;
        }

        if (mUpdateState == UpdateState::kUpdated)
        {
            SwapActiveValueStorage();

            if (mAuxCb)
            {
                mAuxCb(mAttrId, mAuxData);
            }
        }
        else
        {
            CleanupValue(GetNewValueRef());
        }

        mUpdateState = UpdateState::kIdle;
    }

    /**
     * @brief Clean up current active value storage
     */
    void Cleanup()
    {
        CleanupValue(GetValueRef());
        mHoldState = StorageState::kEmpty;
    }

private:
    uint8_t mActiveValueIdx = 0;

    ValueType & GetValueRef() { return mValueStorage[mActiveValueIdx]; }

    ValueType & GetNewValueRef() { return mValueStorage[!mActiveValueIdx]; }

    void SwapActiveValueStorage() { mActiveValueIdx = !mActiveValueIdx; }

    static constexpr bool IsValueNullable() { return IsNullable<T>::value; }

    static constexpr bool IsValueList() { return IsList<T>::value; }

    enum class StorageState : uint8_t
    {
        kEmpty, // Value not initiated (default state)
        kHold,  // GetValueRef() holds valid data
    };

    enum class UpdateState : uint8_t
    {
        kIdle,        // No active update
        kInitialized, // New value initialized
        kAssigned,    // New value populated
        kValidated,   // New value validated
        kUpdated,     // New value committed
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

    void CleanupValue(ValueType & aValue)
    {
        if constexpr (IsValueNullable())
        {
            if (!aValue.IsNull())
            {
                if constexpr (IsList<WrappedType>::value)
                {
                    CleanupList(aValue.Value());
                }
                else if constexpr (IsStruct<WrappedType>::value)
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
    T mValueStorage[2]; // Double-buffered storage

    void * mAuxData = nullptr;                  // Validation context
    const uint32_t mAttrId;                     // Attribute identifier
    void (*mAuxCb)(uint32_t, void *) = nullptr; // Update callback

    StorageState mHoldState  = StorageState::kEmpty;
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

    bool NullableNotEqual(const T & a, const T & b)
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
            else if constexpr (IsStruct<WrappedType>::value)
            {
                is_neq = CompareStructValue(a.Value(), b.Value());
            }
            else if constexpr (IsNumeric<WrappedType>::value || IsEnum<WrappedType>::value)
            {
                is_neq = (a.Value() != b.Value());
            }
            else
            {
                static_assert(false, "Unexpected Nullable wrapped type");
            }
        }

        return is_neq;
    }

    virtual void CleanupStructValue(PayloadType & aValue) { (void) aValue; }
};

} // namespace CommodityTariffAttrsDataMgmt
} // namespace app
} // namespace chip
