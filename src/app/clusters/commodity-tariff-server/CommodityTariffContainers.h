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

#include <cassert>
#include <pw_containers/algorithm.h>
#include <pw_containers/vector.h>

#include "lib/core/CHIPError.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>

#include "CommodityTariffConsts.h"

namespace chip {
namespace app {
namespace CommodityTariffContainers {

template <typename T, size_t kMaxSize>
class CTC_UnorderedSet : public pw::Vector<T, kMaxSize>
{
public:
    using Base          = pw::Vector<T, kMaxSize>;
    using ValueType     = T;
    using Iterator      = typename Base::iterator;
    using ConstIterator = typename Base::const_iterator;

    CTC_UnorderedSet() = default;

    // Modifiers
    bool insert(const T & value)
    {
        if (contains(value) || this->full())
        {
            return false;
        }

        this->push_back(value);
        return true;
    }

    bool insert(T && value)
    {
        if (contains(value) || this->full())
        {
            return false;
        }

        this->push_back(std::move(value));
        return true;
    }

    void remove(const T & value)
    {
        auto it = find(value);
        if (it == this->end())
        {
            return;
        }

        // Swap with last element and pop (O(1) removal)
        if (it != this->end() - 1)
        {
            *it = std::move(this->back());
        }
        this->pop_back();
    }

    void clear() { this->clear(); }

    // Lookup
    bool contains(const T & value) const { return find(value) != this->end(); }

    Iterator find(const T & value) { return pw::containers::Find(*this, value); }

    ConstIterator find(const T & value) const { return pw::containers::Find(*this, value); }

    // Merge operations
    template <size_t OtherMaxSize>
    void merge(const CTC_UnorderedSet<T, OtherMaxSize> & other)
    {
        for (const auto & item : other)
        {
            insert(item);
        }
    }

    template <typename InputIterator>
    void merge(InputIterator first, InputIterator last)
    {
        for (auto it = first; it != last; ++it)
        {
            insert(*it);
        }
    }
};

template <typename Key, typename Value, size_t kMaxSize>
class CTC_UnorderedMap : public CTC_UnorderedSet<std::pair<Key, Value>, kMaxSize>
{
public:
    using Base          = CTC_UnorderedSet<std::pair<Key, Value>, kMaxSize>;
    using PairType      = std::pair<Key, Value>;
    using Iterator      = typename Base::iterator;
    using ConstIterator = typename Base::const_iterator;

    CTC_UnorderedMap() = default;

    // Modifiers
    bool insert(const Key & key, const Value & value)
    {
        if (contains(key) || this->full())
        {
            return false;
        }
        return Base::insert(std::make_pair(key, value));
    }

    // Lookup
    bool contains(const Key & key) const { return find(key) != this->end(); }

    Iterator find(const Key & key)
    {
        return pw::containers::FindIf(*this, [&key](const PairType & pair) { return pair.first == key; });
    }

    ConstIterator find(const Key & key) const
    {
        return pw::containers::FindIf(*this, [&key](const PairType & pair) { return pair.first == key; });
    }

    // Access
    Value & operator[](const Key & key)
    {
        auto it = find(key);
        if (it != this->end())
        {
            return it->second;
        }

        // Key not found, must insert.
        if (this->full())
        {
            ChipLogError(AppServer, "Can't place new entry - the buffer is full");
            // This is a programming error. Using operator[] on a full map for a new key.
            assert(!this->full());
        }

        Base::insert({ key, Value{} });
        return this->back().second;
    }
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
     * @brief DayEntry IDs referenced by DayPattern and IndividualDays items
     * @details Collected separately for reference validation
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength>
        RefsToDayEntryIDsFromDays;

    /**
     * @brief DayEntry IDs referenced by TariffPeriod items
     * @details Collected separately for reference validation
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayEntriesAttrMaxLength>
        RefsToDayEntryIDsFromTariffPeriods;

    /// @}

    /// @name TariffComponent ID Tracking
    /// @{
    /**
     * @brief All tariff component identifiers are matched with the corresponding feature values.
     * @details Contains all TariffComponent IDs that exist in the tariff definition
     */
    CommodityTariffContainers::CTC_UnorderedMap<uint32_t, uint32_t, CommodityTariffConsts::kTariffComponentsAttrMaxLength>
        TariffComponentKeyIDsFeatureMap;

    /**
     * @brief TariffComponent IDs referenced by TariffPeriod items
     * @details Collected for validating period->component references
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kTariffComponentsAttrMaxLength>
        RefsToTariffComponentIDsFromTariffPeriods;
    /// @}

    /**
     * @brief DayPattern IDs referenced by CalendarPeriod items
     * @details Collected for validating calendar->pattern references
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kDayPatternsAttrMaxLength>
        RefsToDayPatternIDsFromCalendarPeriods;
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
