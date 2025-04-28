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

//#include <json/json.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

static constexpr size_t kDefaultStringValuesMaxBufLength = 128u;

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
public:
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
        if constexpr (is_nullable<T>::value) {
            mValue.SetNull();
        } else if constexpr (is_list<T>::value) {
            mValue = T();
        } 
    }

    /// @brief Virtual destructor for proper cleanup
    virtual ~CTC_BaseDataClass() = default;

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

    /**
     * @brief Update the stored value if it has changed
     * @param aValue New value to store
     * @return true if value was updated, false if no change needed
     * 
     * Checks HasChanged() before applying the update
     */
    bool Update(const T& aValue) {
        if ( HasChanged(aValue) && (UpdateValue(aValue) == CHIP_NO_ERROR) )
        {
            return true;
        }
        return false;
    };

    /**
     * @brief Clean up the current value based on type
     */
     void CleanupValue() {
        if constexpr (is_nullable<T>::value) {
            CleanupNullable(mValue);
            mValue.SetNull();
        } else if constexpr (is_list<T>::value) {
            CleanupList(mValue);
            mValue = T();
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
        if constexpr (is_nullable<T>::value) {
            return NullableNotEqual(mValue, newValue);
        } else if constexpr (is_list<T>::value) {
            return mValue.size() != newValue.size();
        }
        return true;
    }
    
    /**
     * @brief Update the stored value
     * @param aValue New value to store
     * @return CHIP_NO_ERROR on success, error code otherwise
     * 
     * @note Derived classes can override for custom update logic
     */
    virtual CHIP_ERROR UpdateValue(const T& aValue) {
        mValue = aValue;
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
     void CleanupList(const DataModel::List<T>& list) {
        for (const auto& item : list) {
            CleanupListItem(&item);
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
    virtual void CleanupListItem(const void * item) {
        (void) item;
    }

private:
    /// @brief Type trait for nullable types
    template <typename U>
    struct is_nullable : std::false_type {};

    template <typename U>
    struct is_nullable<DataModel::Nullable<U>> : std::true_type {};

    /// @brief Type trait for list types
    template <typename U>
    struct is_list : std::false_type {};

    template <typename U>
    struct is_list<DataModel::List<U>> : std::true_type {};
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
