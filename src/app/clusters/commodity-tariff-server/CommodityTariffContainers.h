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

#include <pw_containers/flat_map.h>
#include <pw_containers/vector.h>
#include <pw_containers/algorithm.h>
#include "lib/core/CHIPError.h"

namespace chip {
namespace app {
namespace CommodityTariffContainers {

template<typename T, size_t kMaxSize>
class CTC_UnorderedSet {
public:
    using ValueType = T;
    using Iterator = typename pw::Vector<T, kMaxSize>::iterator;
    using ConstIterator = typename pw::Vector<T, kMaxSize>::const_iterator;

    PwUnorderedSet() = default;

    // Capacity
    constexpr size_t capacity() const { return kMaxSize; }
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    bool full() const { return data_.size() >= kMaxSize; }

    // Modifiers
    CHIP_ERROR insert(const T& value) {
        if (contains(value)) {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
        if (full()) {
            return CHIP_ERROR_NO_MEMORY;
        }
        return data_.push_back(value) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR insert(T&& value) {
        if (contains(value)) {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
        if (full()) {
            return CHIP_ERROR_NO_MEMORY;
        }
        return data_.push_back(std::move(value)) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR remove(const T& value) {
        auto it = find(value);
        if (it == end()) {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        
        // Swap with last element and pop (O(1) removal)
        if (it != data_.end() - 1) {
            *it = std::move(data_.back());
        }
        data_.pop_back();
        return CHIP_NO_ERROR;
    }

    void clear() {
        data_.clear();
    }

    // Lookup
    bool contains(const T& value) const {
        return find(value) != end();
    }

    Iterator find(const T& value) {
        return pw::containers::Find(data_, value);
    }

    ConstIterator find(const T& value) const {
        return pw::containers::Find(data_, value);
    }

    // Iterators
    Iterator begin() { return data_.begin(); }
    Iterator end() { return data_.end(); }
    ConstIterator begin() const { return data_.begin(); }
    ConstIterator end() const { return data_.end(); }
    ConstIterator cbegin() const { return data_.cbegin(); }
    ConstIterator cend() const { return data_.cend(); }

    // Access
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }

    T* data() { return data_.data(); }
    const T* data() const { return data_.data(); }

    // Merge operations
    template<size_t OtherMaxSize>
    CHIP_ERROR merge(const PwUnorderedSet<T, OtherMaxSize>& other) {
        for (const auto& item : other) {
            CHIP_ERROR err = insert(item);
            if (err != CHIP_NO_ERROR && err != CHIP_ERROR_DUPLICATE_KEY_ID) {
                return err; // Return on memory error, continue on duplicate
            }
        }
        return CHIP_NO_ERROR;
    }

    template<typename InputIterator>
    CHIP_ERROR merge(InputIterator first, InputIterator last) {
        for (auto it = first; it != last; ++it) {
            CHIP_ERROR err = insert(*it);
            if (err != CHIP_NO_ERROR && err != CHIP_ERROR_DUPLICATE_KEY_ID) {
                return err;
            }
        }
        return CHIP_NO_ERROR;
    }
private:
    pw::Vector<T, kMaxSize> data_;
}

template<typename Key, typename Value, size_t kMaxSize>
class CTC_UnorderedMap {
public:
    using PairType = std::pair<Key, Value>;
    using Iterator = typename pw::Vector<PairType, kMaxSize>::iterator;
    using ConstIterator = typename pw::Vector<PairType, kMaxSize>::const_iterator;

    PwUnorderedMap() = default;

    // Capacity
    constexpr size_t capacity() const { return kMaxSize; }
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    bool full() const { return data_.size() >= kMaxSize; }

    // Modifiers
    CHIP_ERROR insert(const Key& key, const Value& value) {
        return insert(std::make_pair(key, value));
    }

    CHIP_ERROR insert(const PairType& pair) {
        if (contains(pair.first)) {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
        if (full()) {
            return CHIP_ERROR_NO_MEMORY;
        }
        return data_.push_back(pair) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR insert(PairType&& pair) {
        if (contains(pair.first)) {
            return CHIP_ERROR_DUPLICATE_KEY_ID;
        }
        if (full()) {
            return CHIP_ERROR_NO_MEMORY;
        }
        return data_.push_back(std::move(pair)) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR remove(const Key& key) {
        auto it = find(key);
        if (it == end()) {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        
        // Swap with last element and pop
        if (it != data_.end() - 1) {
            *it = std::move(data_.back());
        }
        data_.pop_back();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR update(const Key& key, const Value& value) {
        auto it = find(key);
        if (it == end()) {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }
        it->second = value;
        return CHIP_NO_ERROR;
    }

    void clear() {
        data_.clear();
    }

    // Lookup
    bool contains(const Key& key) const {
        return find(key) != end();
    }

    Value* get_value(const Key& key) {
        auto it = find(key);
        return it != end() ? &it->second : nullptr;
    }

    const Value* get_value(const Key& key) const {
        auto it = find(key);
        return it != end() ? &it->second : nullptr;
    }

    Iterator find(const Key& key) {
        return pw::containers::FindIf(data_, 
            [&key](const PairType& pair) { return pair.first == key; });
    }

    ConstIterator find(const Key& key) const {
        return pw::containers::FindIf(data_, 
            [&key](const PairType& pair) { return pair.first == key; });
    }

    // Access
    Value& operator[](const Key& key) {
        auto* value = get_value(key);
        if (!value) {
            // Insert default value if key doesn't exist
            insert(key, Value{});
            value = get_value(key);
        }
        return *value;
    }

    // Iterators
    Iterator begin() { return data_.begin(); }
    Iterator end() { return data_.end(); }
    ConstIterator begin() const { return data_.begin(); }
    ConstIterator end() const { return data_.end(); }
    ConstIterator cbegin() const { return data_.cbegin(); }
    ConstIterator cend() const { return data_.cend(); }

    pw::Vector<Key, kMaxSize> get_keys() const {
        pw::Vector<Key, kMaxSize> keys;
        for (const auto& pair : data_) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    pw::Vector<Value, kMaxSize> get_values() const {
        pw::Vector<Value, kMaxSize> values;
        for (const auto& pair : data_) {
            values.push_back(pair.second);
        }
        return values;
    }

    // Merge operations
    template<size_t OtherMaxSize>
    CHIP_ERROR merge(const PwUnorderedMap<Key, Value, OtherMaxSize>& other, bool overwrite = false) {
        for (const auto& pair : other) {
            if (!contains(pair.first)) {
                // New key
                CHIP_ERROR err = insert(pair);
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
            } else if (overwrite) {
                // Existing key, overwrite
                CHIP_ERROR err = update(pair.first, pair.second);
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
            }
            // else: keep existing (do nothing)
        }
        return CHIP_NO_ERROR;
    }

    template<typename InputIterator>
    CHIP_ERROR merge(InputIterator first, InputIterator last, bool overwrite = false) {
        for (auto it = first; it != last; ++it) {
            if (!contains(it->first)) {
                CHIP_ERROR err = insert(*it);
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
            } else if (overwrite) {
                CHIP_ERROR err = update(it->first, it->second);
                if (err != CHIP_NO_ERROR) {
                    return err;
                }
            }
        }
        return CHIP_NO_ERROR;
    }

private:
    pw::Vector<PairType, kMaxSize> data_;
};

} // namespace CommodityTariffContainers

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
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength> DayEntryKeyIDs;

    /**
     * @brief DayEntry IDs referenced by DayPattern items
     * @details Collected separately for reference validation
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength> DayPatternsDayEntryIDs;

    /**
     * @brief DayEntry IDs referenced by IndividualDays items
     * @details Collected separately for reference validation
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength>
        IndividualDaysDayEntryIDs;

    /**
     * @brief DayEntry IDs referenced by TariffPeriod items
     * @details Collected separately for reference validation
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength> TariffPeriodsDayEntryIDs;

    /// @}

    /// @name TariffComponent ID Tracking
    /// @{
    /**
     * @brief Master set of all valid TariffComponent IDs
     * @details Contains all TariffComponent IDs that exist in the tariff definition
     */
    CommodityTariffContainers::CTC_UnorderedMap<uint32_t, uint32_t, CommodityTariffConsts::kTariffComponentsAttrMaxLength>
        TariffComponentKeyIDsFeatureMap;

    /**
     * @brief TariffComponent IDs referenced by TariffPeriod items
     * @details Collected for validating period->component references
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kTariffComponentsAttrMaxLength>
        TariffPeriodsTariffComponentIDs;
    /// @}

    /// @name DayPattern ID Tracking
    /// @{
    /**
     * @brief Master set of all valid DayPattern IDs
     * @details Contains all DayPattern IDs that exist in the tariff definition
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayPatternsAttrMaxLength> DayPatternKeyIDs;

    /**
     * @brief DayPattern IDs referenced by CalendarPeriod items
     * @details Collected for validating calendar->pattern references
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayPatternsAttrMaxLength>
        CalendarPeriodsDayPatternIDs;
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
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
