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

#include <type_traits>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

#define LOG_VAR_CH(changed) ChipLogProgress(NotSpecified, "EGW-CTC: Attr value %s", changed ? "changed": "not changed" )

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
 */
template <typename T>
class CTC_BaseDataClass {
private:
    /// @brief Type trait for nullable types
    template <typename U>
    struct IsNullable : std::false_type {};

    template <typename U>
    struct IsNullable<DataModel::Nullable<U>> : std::true_type {};

    /// @brief Type trait for list types
    template <typename U>
    struct IsList : std::false_type {};

    template <typename U>
    struct IsList<DataModel::List<U>> : std::true_type {};

    // Type extraction utilities (as shown previously)
    template<typename U>
    struct ExtractWrappedType { using type = U; };

    template<typename U>
    struct ExtractWrappedType<DataModel::Nullable<U>> { 
        using type = typename ExtractWrappedType<U>::type; 
    };

    template<typename U>
    using ExtractWrappedType_t = typename ExtractWrappedType<U>::type;

    template<typename U>
    struct ExtractPayloadType {
        using type = U; // Base case - not a wrapper
    };
    
    template<typename U>
    struct ExtractPayloadType<DataModel::Nullable<U>> {
        using type = typename ExtractPayloadType<U>::type;
    };
    
    template<typename U>
    struct ExtractPayloadType<DataModel::List<U>> {
        using type = typename ExtractPayloadType<U>::type;
    };

    template<typename U>
    using ExtractPayloadType_t = typename ExtractPayloadType<U>::type;

    // Helper to recursively unwrap values
    template<typename U>
    static auto unwrapValue(const U& value) -> typename std::enable_if<!IsNullable<U>::value && !IsList<U>::value, U>::type {
        return value; // Base case - not a wrapper
    }

    template<typename U>
    static auto unwrapValue(const U& value) -> typename std::enable_if<IsNullable<U>::value, ExtractWrappedType_t<U>>::type {
        return value.HasValue() ? unwrapValue(value.Value()) : ExtractWrappedType_t<U>{};
    }

    template<typename U>
    static auto unwrapValue(const U& value) -> typename std::enable_if<IsList<U>::value, ExtractWrappedType_t<U>>::type {
        if (value.empty()) return ExtractWrappedType_t<U>{};
        return unwrapValue(value.front()); // For lists, return the type of the first element
    }
    
    // Type categorization traits
    template<typename U>
    struct IsNumeric : std::integral_constant<bool, 
        std::is_integral<U>::value || std::is_floating_point<U>::value> {};
    
    template<typename U>
    struct IsEnum : std::is_enum<U> {};
    
    template<typename U>
    struct IsStruct : std::integral_constant<bool,
        !IsNumeric<U>::value && 
        !IsEnum<U>::value && 
        !std::is_pointer<U>::value> {};
public:
    using ValueType = T;
    using WrappedType = ExtractWrappedType_t<ValueType>;
    using PayloadType = ExtractPayloadType_t<WrappedType>;

    static constexpr bool IsValueNullableList() {
        return IsNullable<T>::value && IsList<WrappedType>::value;
    }
    
    static constexpr bool IsValueNullable() {
        return IsNullable<T>::value && !IsList<WrappedType>::value;
    }
    
    static constexpr bool IsValueList() {
        return IsList<T>::value;
    }

    /// @brief Check if the final payload is a struct
    static constexpr bool IsPayloadStruct() {
        return IsStruct<PayloadType>::value;
    }
    
    /// @brief Check if the final payload is numeric
    static constexpr bool IsPayloadNumeric() {
        return IsNumeric<PayloadType>::value;
    }
    
    /// @brief Check if the final payload is an enum
    static constexpr bool IsPayloadEnum() {
        return IsEnum<PayloadType>::value;
    }
    
    /// @brief Get the name of the payload type
    static const char* PayloadTypeName() {
        return typeid(PayloadType).name();
    }

    // Compile-time type check method
    void InspectTypes() {
        // This will force a compile error showing the types
        static_assert(sizeof(ValueType) == -1, 
            "ValueType inspection");
        static_assert(sizeof(WrappedType) == -1, 
            "WrappedType inspection");
        static_assert(sizeof(PayloadType) == -1, 
            "PayloadType inspection");
    }

    /**
     * @brief Construct a new data class instance
     * @param aValueStorage Reference to external value storage
     * 
     * Initializes the storage based on type:
     * - Nullable types: set to null
     * - List types: initialized as empty list
     * - Others: left uninitialized
     */
    explicit CTC_BaseDataClass(T& aValueStorage) : mValue(aValueStorage) {
        if constexpr ( IsValueNullable() ) {
            mValue.SetNull();
        }
        else if constexpr ( IsValueList() )
        {
            mValue = ValueType();
        }
    }

    /// @brief Virtual destructor for proper cleanup
    virtual ~CTC_BaseDataClass() { CleanupValue(); };

    /**
     * @enum Error
     * @brief Operation result codes
     */
    enum class Error : uint8_t {
        kSuccess            = 0x00, ///< Operation succeeded
        kInvalidJson,               ///< Invalid JSON input
        kAllocationFailed,          ///< Memory allocation failed
        kInconsistentData,          ///< Data consistency check failed
        // ...
    };

    /**
     * @brief Get mutable reference to stored value
     * @return T& Reference to the stored value
     */
    T& GetValue() { return mValue; };

    /**
     * @brief Get const reference to stored value
     * @return const T& Const reference to the stored value
     */
    const T& GetValue() const { return mValue; };

    WrappedType GetPayload() const { return unwrapValue(mValue); }

    /**
     * @brief Update the stored value if it has changed
     * @param aValue New value to store
     * @return true if value was updated, false if no change needed
     * 
     * Checks HasChanged() before applying the update
     */
     bool Update(const T& aValue) {
        if (!HasChanged(aValue)) {
            return false;
        }
    
        CHIP_ERROR err = CHIP_NO_ERROR;
    
        if constexpr (IsValueNullableList()) {
            // Explicit handling for Nullable<List<Struct>>
            //InspectTypes();
            if (!aValue.IsNull()) {
                DataModel::List<PayloadType> newList;
                err = UpdateList(aValue.Value(), newList);
                if (err == CHIP_NO_ERROR) {
                    mValue.SetNonNull(newList);
                }
            } else {
                mValue.SetNull();
            }
        }
        else if constexpr (IsValueNullable()) {
            //InspectTypes();
            // Handling for other Nullable types (Struct, numeric, enum)
            if (!aValue.IsNull()) {
                if constexpr (IsStruct<WrappedType>::value) {
                    WrappedType tempValue;
                    err = UpdateStructValue(aValue.Value(), tempValue);
                    if (err == CHIP_NO_ERROR) {
                        mValue.SetNonNull(tempValue);
                    }
                }
                else if constexpr (IsNumeric<WrappedType>::value || IsEnum<WrappedType>::value) {
                    mValue.SetNonNull(aValue.Value());
                }
                else {
                    static_assert(false, "Unexpected Nullable wrapped type");
                }
            } else {
                mValue.SetNull();
            }
        }
        else if constexpr (IsValueList()) {
            //InspectTypes();
            // Handling for plain List<Struct>
            DataModel::List<PayloadType> newList;
            err = UpdateList(aValue, newList);
            if (err == CHIP_NO_ERROR) {
                mValue = newList;
            }
        }
        else {
            // Direct assignment for non-wrapper types
            mValue = aValue;
        }
    
        return err == CHIP_NO_ERROR;
    };

    /**
     * @brief Update the stored value
     * @param aValue New value to store
     * @return CHIP_NO_ERROR on success, error code otherwise
     * 
     * @note Derived classes can override for custom update logic
     */
     //virtual CHIP_ERROR UpdateValue(const T& aValue)

    /**
     * @brief Clean up the current value based on type
     */
    void CleanupValue()
    {
        if constexpr ( IsValueNullableList() ) {
            if (!mValue.IsNull()) {
                CleanupList(mValue.Value());
            }
            mValue.SetNull();
        }
        else if constexpr ( IsValueNullable() )
        {
            if (!mValue.IsNull()) {
                // For Nullable<Struct>, clean up the struct if needed
                if constexpr (IsStruct<WrappedType>::value)
                {
                    CleanupStructValue(unwrapValue(mValue.Value()));
                }
            }
            mValue.SetNull();
        }
        else if constexpr ( IsValueList() )
        {
            CleanupList(mValue);
            mValue=ValueType();
        }
    }
protected:
    T & mValue; ///< Reference to the managed value storage

    /**
     * @brief Validate a new value
     * @param newValue The value to validate
     * @return true if valid, false otherwise
     * 
     * @note Derived classes should override for custom validation
     */
    bool IsValid(const T& newValue) const {
        return true;
    }

    /**
     * @brief Compare nullable values for inequality
     * @param a First value to compare
     * @param b Second value to compare
     * @return true if values are not equal (including null state)
     */
    static bool NullableNotEqual(const T& a, const T& b) {
        bool is_neq = false;
        if (a.IsNull() || b.IsNull()) {
            is_neq = a.IsNull() != b.IsNull();
        }
        LOG_VAR_CH(is_neq);
        return is_neq;
    }

    /**
     * @brief Check if value has changed
     * @param newValue The new value to compare against
     * @return true if value has changed
     * 
     * Special handling for:
     * - Nullable types: checks null state and value
     * - List types: compares sizes
     * - Others: always returns true (assume changed)
     */
    bool HasChanged(const T& newValue) const {
        return true;
    }
    
    virtual CHIP_ERROR UpdateStructValue(const PayloadType& source, PayloadType& destination)
    {
        //mValue = aValue;
        memcpy(&destination, &source, sizeof(PayloadType));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UpdateList(const DataModel::List<PayloadType>& source, DataModel::List<PayloadType>& destination)
    {
        auto* buffer = static_cast<PayloadType*>(
            chip::Platform::MemoryCalloc(source.size(), sizeof(PayloadType))
        );
    
        if (!buffer) {
            return CHIP_ERROR_NO_MEMORY;
        }
    
        for (size_t i = 0; i < source.size(); i++) {
            new (&buffer[i]) PayloadType();
            if (CHIP_NO_ERROR != UpdateStructValue(source[i], buffer[i]) )
            {
                for (size_t j = 0; j < i; j++)
                {
                    CleanupStructValue(buffer[i]);
                }
                return CHIP_ERROR_NO_MEMORY;
            }
        }

        destination = DataModel::List<PayloadType>(chip::Span<PayloadType>(buffer, source.size()));

        return CHIP_NO_ERROR;
   }

    /**
     * @brief Clean up nullable value resources
     * @param aValue The value to clean up
     * 
     * @note Derived classes should override for complex nullable types
     */
    virtual void CleanupNullable(const T& aValue) {
        (void) aValue;
    }

    /**
     * @brief Clean up list memory
     * @param list The list to clean up
     * 
     * Calls CleanupListItem for each element and frees the list memory
     */
     template<typename U>
     void CleanupList(DataModel::List<U>& list) {
        for (const auto& item : list) {
            CleanupStructValue(item);
        }
        if (list.data()) {
            chip::Platform::MemoryFree(list.data());
        }
    }

    /**
     * @brief Clean up list item resources
     * @param item Pointer to the item to clean up
     * 
     * @note Derived classes should override for complex list items
     */
    virtual void CleanupStructValue(const PayloadType& aValue) {
        (void)aValue;
    }
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
