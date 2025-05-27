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

#include <cstring>
#include <type_traits>

namespace chip {

constexpr bool operator!=(const Span<const char>& lhs, const Span<const char>& rhs)
{
    if (lhs.size() != rhs.size()) 
        return true; // Different lengths → not equal
    
    if (lhs.data() == rhs.data()) 
        return false; // Same pointer (or both null) → equal
    
    // Compare byte-by-byte (safe for non-null-terminated data)
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

constexpr bool operator!=(const Span<const uint32_t>& lhs, const Span<const uint32_t>& rhs)
{
    if (lhs.size() != rhs.size()) 
        return true; // Different lengths → not equal
    
    if (lhs.data() == rhs.data()) 
        return false; // Same pointer (or both null) → equal
    
    // Compare byte-by-byte (safe for non-null-terminated data)
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

namespace app {
namespace Clusters {

constexpr bool operator!=(const Globals::Structs::CurrencyStruct::Type & lhs, const Globals::Structs::CurrencyStruct::Type & rhs)
{
    return ((lhs.currency != rhs.currency) || (lhs.decimalPoints != rhs.decimalPoints));
}

namespace CommodityTariff {

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

    /*static constexpr bool IsValueNullableList() {
        return IsNullable<T>::value && IsList<WrappedType>::value;
    }*/
    
    static constexpr bool IsValueNullable() {
        return IsNullable<T>::value; //&& !IsList<WrappedType>::value;
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
    explicit CTC_BaseDataClass(T& aValueStorage) : mValue(aValueStorage), mNewValue(mValue) {
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
     * @brief Indicates that the newValue has changed against current mValue
     * @return true if value has changed
     */
    bool HasChanged()
    {
        return is_changed;
    }

    bool IsValid()
    {
        return is_valid;
    }

    /**
     * @brief Performs a pre-validation of arguments value before assigning it as newValue
     * @param aValue New value for future update mValue
     */
     CHIP_ERROR UpdateBegin(const T & aValue, BitMask<Feature> aFeatureMap) {
        CHIP_ERROR err = CHIP_NO_ERROR;

        mFeatureMap = aFeatureMap;

        err = Validate(aValue);

        if (CHIP_NO_ERROR == err)
        {
            mNewValue = aValue;
        };

        return err;
    };

    void UpdateCommit()
    {
        if (!CompareValues())
        {
            return;
        }

        is_changed = true;

        if constexpr (IsValueNullable()) {
            if (!mNewValue.IsNull()) {
                WrappedType & tempValue = mNewValue.Value();
                CleanupValue();
                mValue.SetNonNull(tempValue);
            } else {
                mValue.SetNull();
            }
        }
        else if constexpr (IsValueList()) {
            CleanupValue();
            mValue = mNewValue;
        }
        else {
            InspectTypes();
            static_assert(false, "Unexpected type");
        }
    }

    void UpdateAbort()
    {
        is_valid = false;
        mNewValue = mValue;
    }

    /**
     * @brief Clean up the current value based on type
     */
    void CleanupValue()
    {
        if constexpr ( IsValueNullable() )
        {
            if (!mValue.IsNull()) {
                // For Nullable<Struct>, clean up the struct if needed
                if constexpr (IsList<WrappedType>::value)
                {
                    CleanupList(mValue.Value());
                }
                else if constexpr (IsStruct<WrappedType>::value)
                {
                    CleanupStructValue(mValue.Value());
                }
            }
            mValue.SetNull();
        }
        else if constexpr ( IsValueList() )
        {
            CleanupList(mValue);
        }
    }
protected:
    T & mValue; // Reference to the applied value storage
    T & mNewValue = mValue;  // Reference to a value for updating
    BitMask<Feature> mFeatureMap;
    bool is_valid = false;
    bool is_changed = false;

    /**
     * @brief Validate a new value
     * @param newValue The value to validate
     * @return true if valid, false otherwise
     * 
     * @note Derived classes should override for custom validation
     */
    virtual CHIP_ERROR Validate(const ValueType & aValue) const {
        return CHIP_NO_ERROR;
    }

    virtual bool CompareStructValue(const PayloadType& source, const PayloadType& destination) const 
    {
        return memcmp(&destination, &source, sizeof(PayloadType));
    }

    bool ListsNotEqual(const DataModel::List<PayloadType>& source,const DataModel::List<PayloadType>& destination)
    {
        if (source.size() != destination.size())
        {
            return true;
        }

        for (size_t i = 0; i < source.size(); i++) {
            if ( CompareStructValue(source[i], destination[i]) )
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
    bool NullableNotEqual(const T& a, const T& b)
    {
        bool is_neq = false;
        if (a.IsNull() || b.IsNull()) {
            is_neq = a.IsNull() != b.IsNull();
        }

        if (!is_neq)
        {
            if constexpr (IsList<WrappedType>::value)
            {
                is_neq = ListsNotEqual(a.Value(), b.Value());
            }
            else if constexpr (IsStruct<WrappedType>::value) {
                is_neq = CompareStructValue(a.Value(), b.Value());
            }
            else if constexpr (IsNumeric<WrappedType>::value || IsEnum<WrappedType>::value)
            {
                is_neq = ( a.Value() == b.Value());
            }
            else {
                static_assert(false, "Unexpected Nullable wrapped type");
            }
        }

        return is_neq;
    }

    bool CompareValues()
    {
        if constexpr (IsValueNullable()) {
            return NullableNotEqual(mNewValue, mValue);
        }
        else if constexpr (IsValueList()) {
            return ListsNotEqual(mNewValue, mValue);
        }
    }

    /**
     * @brief Clean up list memory
     * @param list The list to clean up
     * 
     * Calls CleanupListItem for each element and frees the list memory
     */
    void CleanupList(DataModel::List<PayloadType>& list) {
        for (auto& item : list) {
            CleanupStructValue(item);
        }
        if (list.data()) {
            chip::Platform::MemoryFree(list.data());
            list = DataModel::List<PayloadType>();
        }
    }

    /**
     * @brief Clean up struct item resources
     * @param item Pointer to the item to clean up
     * 
     * @note Derived classes should override for complex list items
     */
    virtual void CleanupStructValue(PayloadType& aValue) {
        (void)aValue;
    }
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
