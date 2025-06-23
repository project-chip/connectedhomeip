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
namespace app {

/**
 * @class CTC_BaseDataClass
 * @tparam T The attribute value type (nullable, list, or primitive)
 * @brief Base template class for attribute data management
 *
 * Provides core functionality for:
 * - Value storage and access
 * - Change detection
 * - Memory management
 * - Type-specific behavior through template specialization
 *
 * The class handles three types of attributes differently:
 * 1. Nullable types (DataModel::Nullable<U>)
 * 2. List types (DataModel::List<U>)
 * 3. Primitive/struct types
 *
 * Key Features:
 * - Automatic initialization of storage based on type
 * - Change detection before updates
 * - Proper cleanup of complex types
 * - Thread-safe value access
 * - The Tariff Data Update Flow
 *
 * @section update_flow Update Flow
 * @defgroup tariff_update_flow Tariff Data Update Flow
 * @brief State machine for managing tariff attribute updates
 *
 * The update process follows a strict state sequence:
 *
 * @dot
 * digraph update_flow {
 *   kIdle -> kInitiated [label="CreateNewValue()"];
 *   kInitiated -> kAssigned [label="MarkAsAssigned()"];
 *   kAssigned -> kValidated [label="UpdateBegin()"];
 *   kValidated -> kUpdated  [label="UpdateCommit()"];
 *   kUpdated -> kIdle  [label="UpdateEnd()"];
 *   kInitiated -> kIdle [label="UpdateEnd()"];
 *   kAssigned -> kIdle [label="UpdateEnd()"];
 *   kValidated -> kIdle [label="UpdateEnd()"];
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
 *    - State: kIdle → kInitiated
 *
 * 2. **Modification Phase**:
 *    @code
 *    /// Get and modify the value
 *    if (auto* newVal = dataObj.GetNewValue()) {
 *        *newVal = configuredValue;
 *        err = dataObj.MarkAsAssigned();
 *    }
 *    @endcode
 *    - State: kInitiated → kAssigned
 *
 * 3. **Validation Phase**:
 *    @code
 *    /// Validate and prepare for commit
 *    err = dataObj.UpdateBegin(context, callback);
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
 *    - Applies changes if validated
 *    - Invokes callback on success ( to indicate attr changing)
 *    - State: kValidated → kUpdated
 *
 * ### Error Recovery:
 * - At any point, UpdateEnd() can be called to:
 *   - Discard pending changes
 *   - Clean up allocated resources
 *   - Reset to kIdle state
 *
 * ### State Transition Rules:
 * | Current State    | Valid Operations                     |
 * |------------------|--------------------------------------|
 * | kIdle            | CreateNewValue()                     |
 * | kInitiated       | GetNewValue(), MarkAsAssigned(), UpdateEnd() |
 * | kAssigned        | UpdateBegin(), UpdateEnd()         |
 * | kValidated       | UpdateCommit(), UpdateEnd()        |
 * | kUpdated         | UpdateEnd()        |
 *
 * @see CreateNewValue()
 * @see GetNewValue()
 * @see MarkAsAssigned()
 * @see UpdateBegin()
 * @see UpdateCommit()
 * @see UpdateEnd()
 */
template <typename T>
class CTC_BaseDataClass
{
private:
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
    struct IsStruct : std::integral_constant<bool, !IsNumeric<U>::value && !IsEnum<U>::value && !std::is_pointer<U>::value>
    {
    };

    static constexpr bool IsValueNullable()
    {
        return IsNullable<T>::value; //&& !IsList<WrappedType>::value;
    }

    static constexpr bool IsValueList() { return IsList<T>::value; }

    /// @brief Check if the final payload is a struct
    static constexpr bool IsPayloadStruct() { return IsStruct<PayloadType>::value; }

    /// @brief Check if the final payload is numeric
    static constexpr bool IsPayloadNumeric() { return IsNumeric<PayloadType>::value; }

    /// @brief Check if the final payload is an enum
    static constexpr bool IsPayloadEnum() { return IsEnum<PayloadType>::value; }

    /**
     * @enum StorageStates
     * @brief Tracks the progress of value updates
     */
    enum class StorageState : uint8_t
    {
        kEmpty, // A value not initiated (default state)
        kHold,  // mValue holds some valid data
    };

    /**
     * @enum UpdateState
     * @brief Tracks the progress of value updates
     */
    enum class UpdateState : uint8_t
    {
        kIdle,      // mNewValue not active, mValue has default/null state
        kInitiated, // mNewValue activated (memory allocated)
        kAssigned,  // mNewValue holds new data
        kValidated, // mNewValue data validated
        kUpdated,   // mValue replaced to mNewValue
    };

    using valueType   = T;
    using wrappedType = ExtractWrappedType_t<valueType>;
    using payloadType = ExtractPayloadType_t<wrappedType>;

    /**
     * @brief Validate a new value
     * @return CHIP_NO_ERROR if valid, an err code otherwise
     */
    CHIP_ERROR ValidateValue()
    {
        CHIP_ERROR err = Validate(mNewValue);
        if (err == CHIP_NO_ERROR)
        {
            mUpdateState = UpdateState::kValidated;
        }

        return err;
    }

    /**
     * @brief Checks that the newValue has changed against current mValue
     */
    bool HasChanged()
    {
        if constexpr (IsValueNullable())
        {
            return NullableNotEqual(mNewValue, mValue);
        }
        else if constexpr (IsValueList())
        {
            return ListsNotEqual(mNewValue, mValue);
        }
    }

    /**
     * @brief Clean up the current value based on type
     */
    void CleanupValue(valueType & aValue)
    {
        if constexpr (IsValueNullable())
        {
            if (!aValue.IsNull())
            {
                // For Nullable<Struct>, clean up the struct if needed
                if constexpr (IsList<wrappedType>::value)
                {
                    CleanupList(aValue.Value());
                }
                else if constexpr (IsStruct<wrappedType>::value)
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

    /**
     * @brief Clean up list memory
     * @param list The list to clean up
     *
     * Calls CleanupListItem for each element and frees the list memory
     */
    void CleanupList(DataModel::List<payloadType> & list)
    {
        for (auto & item : list)
        {
            CleanupStruct(item);
        }
        if (list.data())
        {
            chip::Platform::MemoryFree(list.data());
            list = DataModel::List<payloadType>();
        }
    }

    /**
     * @brief Clean up struct item resources
     * @param aValue Pointer to the item to clean up
     *
     * @note Derived classes should override for complex list items
     */
    void CleanupStruct(payloadType & aValue) { CleanupStructValue(aValue); }

public:
    using ValueType   = valueType;
    using WrappedType = wrappedType;
    using PayloadType = payloadType;

    /**
     * @brief Construct a new data class instance
     * @param aValueStorage Reference to external value storage
     *
     * Initializes the storage based on type:
     * - Nullable types: set to null
     * - List types: initialized as empty list
     * - Others: left uninitialized
     */
    explicit CTC_BaseDataClass(T & aValueStorage, uint32_t aAttrId) : mValue(aValueStorage), mAttrId(aAttrId)
    {
        if constexpr (IsValueNullable())
        {
            mValue.SetNull();
        }
        else if constexpr (IsValueList())
        {
            mValue = valueType();
        }

        mHoldState   = StorageState::kEmpty;
        mUpdateState = UpdateState::kIdle;
    }

    /// @brief Virtual destructor for proper cleanup
    virtual ~CTC_BaseDataClass() { Cleanup(); };

    /**
     * @brief Get mutable reference to stored value
     * @return T& Reference to the stored value
     */
    T & GetValue() { return mValue; };

    /**
     * @brief Get const reference to stored value
     * @return const T& Const reference to the stored value
     */
    const T & GetValue() const { return mValue; };

    /**
     * @brief Indicates that the newValue has changed against current mValue
     * @return true if value has changed
     */
    bool IsValid() { return (mUpdateState == UpdateState::kValidated); }

    /**
     * @brief Prepares a new value for modification before committing
     * @param size Number of elements to allocate (for list types only)
     * @return CHIP_NO_ERROR on success, or:
     *         - CHIP_ERROR_NO_MEMORY if allocation fails
     *         - CHIP_ERROR_INVALID_LIST_LENGTH if size < 1 for list types
     *         - CHIP_ERROR_INCORRECT_STATE if called during active update
     *
     * @note For list types:
     *       - Allocates memory for specified number of elements
     *       - Initializes list storage
     *       - Transitions state to kInitiated
     * @note For non-list types:
     *       - Creates default-initialized value
     *       - Ignores size parameter
     *
     * @pre Must be called in kIdle state
     * @post On success, transitions to kInitiated state
     */
    CHIP_ERROR CreateNewValue(size_t size)
    {
        if (mUpdateState != UpdateState::kIdle)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if constexpr (IsValueList() || IsList<wrappedType>::value)
        {
            if (size < 1)
            {
                return CHIP_ERROR_INVALID_LIST_LENGTH;
            }

            auto * buffer = static_cast<PayloadType *>(chip::Platform::MemoryCalloc(size, sizeof(PayloadType)));

            if (!buffer)
            {
                return CHIP_ERROR_NO_MEMORY;
            }

            if constexpr (IsValueNullable())
            {
                mNewValue.SetNoNull(DataModel::List<PayloadType>(buffer, size));
            }
            else
            {
                mNewValue = DataModel::List<PayloadType>(buffer, size);
            }
        }
        else if constexpr (IsValueNullable())
        {
            mNewValue = ValueType();
        }

        mUpdateState = UpdateState::kInitiated;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Accesses the pending new value for modification
     * @return Pointer to modifiable value if available
     * @retval nullptr if:
     *         - Not in kInitiated state
     *
     * @pre Must be in kInitiated state (after CreateNewValue())
     * @warning Pointer becomes invalid after UpdateCommit()/UpdateEnd()
     * @see MarkAsAssigned()
     */
    ValueType * GetNewValue()
    {
        if (mUpdateState != UpdateState::kInitiated)
        {
            return nullptr;
        }
        return &mNewValue;
    }

    /**
     * @brief Signals completion of new value modifications
     * @return CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if not in kInitiated state
     *
     * @pre Must be in kInitiated state (after GetNewValue() modifications)
     * @post On success, transitions to kAssigned state
     * @see UpdateBegin()
     */
    CHIP_ERROR MarkAsAssigned()
    {
        if (mUpdateState != UpdateState::kInitiated)
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
     * @retval Other validation errors from ValidateValue()
     *
     * @pre Must be in kAssigned state (after MarkAsAssigned())
     * @post On success, transitions to kValidated state
     */
    CHIP_ERROR UpdateBegin(void * aUpdCtx, void (*aUpdCb)(uint32_t, void *))
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
    
        //Bypass validation if ctx not set
        if (aUpdCtx != nullptr)
        {
            mAuxData = aUpdCtx;

            err = ValidateValue();
        }

        if ( aUpdCb != nullptr )
        {
            mAuxCb   = aUpdCb;
        }

        if (err == CHIP_NO_ERROR)
        {
            mUpdateState = UpdateState::kValidated;
        }
    
        return err;
    }

    /**
     * @brief Commits the validated new value
     *
     * @note Performs the following actions:
     *       1. Checks if value actually changed
     *       2. Cleans up old value if needed
     *       3. Updates stored value
     *       4. Invokes callback if provided
     *       5. Resets to kIdle state
     *
     * @pre Must be in kValidated state (after UpdateBegin())
     * @post Always transitions to kIdle state
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
                Cleanup();
            }

            mValue     = mNewValue;
            mHoldState = StorageState::kHold;

            if (mAuxCb)
            {
                mAuxCb(mAttrId, mAuxData);
            }

            mUpdateState = UpdateState::kUpdated;
        }

        UpdateEnd();
    }

    /**
     * @brief Aborts the current update process
     *
     * @note Performs the following actions:
     *       1. Discards pending new value
     *       2. Cleans up if needed
     *       3. Resets to kIdle state
     *
     * @post Always transitions to kIdle state
     */
    void UpdateEnd()
    {
        /* Skip if the attribute object has no new attached data */
        if (mUpdateState == UpdateState::kIdle)
        {
            return;
        }
        /* In cases if new data was initialized/activated but has not assigned to Value storage */
        if (mUpdateState != UpdateState::kUpdated)
        {
            CleanupValue(mNewValue);
        }
        mUpdateState = UpdateState::kIdle;
    }

    void Cleanup() { 
        CleanupValue(mValue);
        mHoldState = StorageState::kEmpty;
    }

protected:
    T & mValue;             // Reference to the applied value storage
    T mNewValue;            // Value for updating
    void * mAuxData;        // Pointer to an auxiliary data which can be used in some method implementations
    const uint32_t mAttrId;
    void (*mAuxCb)(uint32_t, void *);

    StorageState mHoldState  = StorageState::kEmpty;
    UpdateState mUpdateState = UpdateState::kIdle;

    bool is_changed = false;

    /**
     * @brief Validate a new value
     * @param newValue The value to validate
     * @return CHIP_NO_ERROR if valid, an err code otherwise
     *
     * @note Derived classes should override for custom validation
     */
    virtual CHIP_ERROR Validate(const valueType & aValue) const { return CHIP_NO_ERROR; }

    virtual bool CompareStructValue(const PayloadType & source, const PayloadType & destination) const
    {
        return memcmp(&destination, &source, sizeof(PayloadType));
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

    /**
     * @brief Compare nullable values for inequality
     * @param a First value to compare
     * @param b Second value to compare
     * @return true if values are not equal (including null state)
     */
    bool NullableNotEqual(const T & a, const T & b)
    {
        bool is_neq = false;
        if (a.IsNull() || b.IsNull())
        {
            /* if both are null then they are equal
               if one is null and the other isn't, we don't need to look at the values */
            is_neq = a.IsNull() != b.IsNull();
        }
        else
        {
            /* both are non null, so we can safely compare .Values() */
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
                is_neq = (a.Value() == b.Value());
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

template <typename T>
struct SpanCopier
{
    static bool Copy(const chip::Span<const T> & source, DataModel::List<const T> & destination,
                     size_t maxElements = std::numeric_limits<size_t>::max())
    {
        if (source.empty())
        {
            destination = DataModel::List<const T>();
            return true;
        }

        size_t elementsToCopy = std::min(source.size(), maxElements);
        auto * buffer         = static_cast<T *>(chip::Platform::MemoryCalloc(elementsToCopy, sizeof(T)));

        if (!buffer)
        {
            return false;
        }

        std::copy(source.begin(), source.begin() + elementsToCopy, buffer);
        destination = DataModel::List<const T>(chip::Span<const T>(buffer, elementsToCopy));
        return true;
    }
};

template <>
struct SpanCopier<char>
{
    static bool Copy(const chip::CharSpan & source, DataModel::Nullable<chip::CharSpan> & destination,
                     size_t maxLength = std::numeric_limits<size_t>::max())
    {
        if (source.size() > maxLength)
        {
            return false;
        }

        if (source.empty())
        {
            destination.SetNull();
            return true;
        }

        size_t bytesToCopy = std::min(source.size(), maxLength);
        char * buffer      = static_cast<char *>(chip::Platform::MemoryCalloc(1, bytesToCopy + 1)); // +1 for null terminator

        if (!buffer)
        {
            return false;
        }

        std::copy(source.begin(), source.begin() + bytesToCopy, buffer);
        buffer[bytesToCopy] = '\0';
        destination.SetNonNull(chip::CharSpan(buffer, bytesToCopy));
        return true;
    }
};

struct StrToSpan
{
    static CHIP_ERROR Copy(const std::string & source, chip::CharSpan & destination,
                           size_t maxLength = std::numeric_limits<size_t>::max())
    {
        // Handle empty string case
        if (source.empty())
        {
            destination = chip::CharSpan();
            return CHIP_NO_ERROR;
        }

        // Check length limit
        if (source.size() > maxLength)
        {
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }

        // Allocate memory (including null terminator)
        char * buffer = static_cast<char *>(chip::Platform::MemoryAlloc(source.size() + 1));
        if (buffer == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        // Copy data and null-terminate
        memcpy(buffer, source.data(), source.size());
        buffer[source.size()] = '\0';

        // Set output span
        destination = chip::CharSpan(buffer, source.size());
        return CHIP_NO_ERROR;
    }

    // Optional: Memory cleanup helper
    static void Release(chip::CharSpan & span)
    {
        if (!span.empty())
        {
            chip::Platform::MemoryFree(const_cast<char *>(span.data()));
            span = chip::CharSpan();
        }
    }
};

} // namespace app
} // namespace chip
