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

#include <algorithm>
#include <cstddef>
#include <utility>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>

#include "CommodityTariffConsts.h"

namespace chip {
namespace app {
namespace CommodityTariffContainers {

template <typename T, size_t Capacity>
class CTC_ContainerClassBase
{
    static_assert(std::is_trivially_destructible<T>::value, "T must be trivially destructible");

protected:
    CTC_ContainerClassBase() = default;

public:
    virtual ~CTC_ContainerClassBase() = default;

    enum class RetCode : uint8_t
    {
        kSuccess,
        kNoInit,
        kInUse,
        kNoMem,
        kGenericFail,
        kNotFound,
        kDuplicate,
    };

    virtual bool insert(const T & item) = 0;
    virtual void remove(const T & item) = 0;
    virtual void clear() { mCount = 0; };
    virtual bool contains(const T & item) const  = 0;
    virtual T * find(const T & item)             = 0;
    virtual const T * find(const T & item) const = 0;

    size_t size() const { return mCount; }
    constexpr size_t capacity() const { return Capacity; }
    bool empty() const { return (mCount == 0); }

    // Array-style access
    T & operator[](size_t index)
    {
        VerifyOrDie(index < mCount);
        return mBuffer[index];
    }

    const T & operator[](size_t index) const
    {
        VerifyOrDie(index < mCount);
        return mBuffer[index];
    }

    // Iterator support
    T * begin() { return mBuffer; }
    T * end() { return mBuffer + mCount; }
    const T * begin() const { return mBuffer; }
    const T * end() const { return mBuffer + mCount; }

protected:
    T mBuffer[Capacity];
    size_t mCount = 0;

    RetCode insertAtEnd(const T & item)
    {
        if (mCount >= Capacity)
        {
            return RetCode::kNoMem;
        }

        // Append to the end
        mBuffer[mCount] = item;
        ++mCount;
        return RetCode::kSuccess;
    }
};

template <typename ItemType, size_t Capacity>
class CTC_UnorderedSet : public CTC_ContainerClassBase<ItemType, Capacity>
{
public:
    using Base = CTC_ContainerClassBase<ItemType, Capacity>;
    using Base::Base;

    CTC_UnorderedSet(CTC_UnorderedSet && other) noexcept             = default;
    CTC_UnorderedSet & operator=(CTC_UnorderedSet && other) noexcept = default;

    bool insert(const ItemType & item) override
    {
        // Check for duplicate using std::find
        if (this->find(item) != nullptr)
        {
            return false; // Duplicate
        }

        // Append to the end (unordered)
        return (this->insertAtEnd(item) == Base::RetCode::kSuccess);
    }

    void remove(const ItemType & item) override
    {
        auto * found = this->find(item);
        if (found)
        {
            // Swap with the last element for O(1) removal (after finding).
            auto * last = &this->mBuffer[this->mCount - 1];
            if (found != last)
            {
                *found = std::move(*last);
            }
            --this->mCount;
        }
    }

    bool contains(const ItemType & item) const override { return this->find(item) != nullptr; }

    ItemType * find(const ItemType & item) override
    {
        // Linear search using std::find
        auto it = std::find(this->begin(), this->end(), item);
        return (it != this->end()) ? it : nullptr;
    }

    const ItemType * find(const ItemType & item) const override
    {
        // Const version of linear search
        auto it = std::find(this->begin(), this->end(), item);
        return (it != this->end()) ? it : nullptr;
    }

    /**
     * @brief Merge another set into this set with automatic resizing
     * @param other The set to merge into this one
     * @return RetCode indicating success or failure
     *
     * @note Automatically resizes if needed (if ensureCapacity is implemented)
     * @note Duplicate items from 'other' are skipped
     */
    template <typename InputIterator>
    void mergeIt(InputIterator first, InputIterator last)
    {
        for (auto it = first; it != last; ++it)
        {
            if (!this->contains(*it))
            {
                auto ret = this->insertAtEnd(*it);
                VerifyOrDie(ret == Base::RetCode::kSuccess);
            }
        }
    }

    /**
     * @brief Merge from any container (std::vector, std::array, etc.)
     */
    template <typename Container>
    void merge(const Container & container)
    {
        auto ret = Base::RetCode::kSuccess;
        // Calculate required capacity including only non-duplicates
        size_t nonDuplicateCount = 0;
        for (const auto & item : container)
        {
            if (!this->contains(item))
            {
                nonDuplicateCount++;
            }
        }

        size_t requiredCapacity = this->mCount + nonDuplicateCount;
        if (requiredCapacity > Capacity)
        {
            ret = Base::RetCode::kNoMem;
        }

        VerifyOrDie(ret == Base::RetCode::kSuccess);

        mergeIt(container.begin(), container.end());
    }
};

template <typename KeyType, typename ValueType, size_t Capacity>
class CTC_UnorderedMap
{
public:
    using PairType = std::pair<KeyType, ValueType>;

    static_assert(std::is_trivially_destructible<PairType>::value, "KeyType and ValueType must be trivially destructible");

private:
    // Custom set that only compares keys for uniqueness
    class PairSet : public CTC_UnorderedSet<PairType, Capacity>
    {
    public:
        using Base = CTC_UnorderedSet<PairType, Capacity>;
        using Base::Base;

        PairType * insertUnchecked(const PairType & pair)
        {
            if (this->insertAtEnd(pair) == Base::RetCode::kSuccess)
            {
                return &this->mBuffer[this->mCount - 1];
            }
            return nullptr;
        }

        // Override find to only compare keys
        PairType * find(const PairType & pair) override { return findByKey(pair.first); }

        const PairType * find(const PairType & pair) const override { return findByKey(pair.first); }

        PairType * findByKey(const KeyType & key)
        {
            auto it = std::find_if(this->begin(), this->end(), [&key](const PairType & item) { return item.first == key; });
            return (it != this->end()) ? it : nullptr;
        }

        const PairType * findByKey(const KeyType & key) const
        {
            auto it = std::find_if(this->begin(), this->end(), [&key](const PairType & item) { return item.first == key; });
            return (it != this->end()) ? it : nullptr;
        }

        void removeByKey(const KeyType & key)
        {
            if (auto * found = findByKey(key))
            {
                // Swap with the last element for O(1) removal (after finding).
                auto * last = &this->mBuffer[this->mCount - 1];
                if (found != last)
                {
                    *found = std::move(*last);
                }
                --this->mCount;
            }
        }

        // Override contains to only check key
        bool contains(const PairType & pair) const override { return containsKey(pair.first); }

        bool containsKey(const KeyType & key) const { return findByKey(key) != nullptr; }
    };

    PairSet mPairStorage;

public:
    CTC_UnorderedMap() = default;

    bool insert(const KeyType & key, const ValueType & value) { return mPairStorage.insert(std::make_pair(key, value)); }

    bool insert(const PairType & pair) { return mPairStorage.insert(pair); }

    void remove(const KeyType & key) { mPairStorage.removeByKey(key); }

    ValueType * find(const KeyType & key)
    {
        auto * pair = mPairStorage.findByKey(key);
        return pair ? &pair->second : nullptr;
    }

    const ValueType * find(const KeyType & key) const
    {
        auto * pair = mPairStorage.findByKey(key);
        return pair ? &pair->second : nullptr;
    }

    // In class CTC_UnorderedMap
    ValueType & operator[](const KeyType & key)
    {
        auto * pair = mPairStorage.findByKey(key);
        if (pair == nullptr)
        {
            // Use insertUnchecked since we already know the key is not present.
            pair = mPairStorage.insertUnchecked(std::make_pair(key, ValueType{}));
            VerifyOrDie(pair != nullptr);
        }
        return pair->second;
    }

    bool contains(const KeyType & key) const { return mPairStorage.containsKey(key); }

    size_t size() const { return mPairStorage.size(); }
    size_t capacity() const { return mPairStorage.capacity(); }
    bool empty() const { return mPairStorage.empty(); }

    // Iterator support
    auto begin() { return mPairStorage.begin(); }
    auto end() { return mPairStorage.end(); }
    auto begin() const { return mPairStorage.begin(); }
    auto end() const { return mPairStorage.end(); }
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
    CommodityTariffContainers::CTC_UnorderedMap<uint32_t, uint32_t, CommodityTariffConsts::kTariffComponentMaxLabelLength>
        TariffComponentKeyIDsFeatureMap;

    /**
     * @brief TariffComponent IDs referenced by TariffPeriod items
     * @details Collected for validating period->component references
     */
    CommodityTariffContainers::CTC_UnorderedSet<uint32_t, CommodityTariffConsts::kTariffComponentMaxLabelLength>
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
