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

#include "CommodityTariffAttrsDataMgmt.h"
#include <algorithm>
#include <cstddef>
#include <utility>

namespace chip {
namespace app {
namespace CommodityTariffContainers {

template<typename T>
class CTC_ContainerClassBase
{
    static_assert(std::is_trivially_destructible<T>::value, "T must be trivially destructible");
protected:
    CTC_ContainerClassBase(size_t aCapacity, size_t aMaxLimit = 0) : mMaxLimit(aMaxLimit), mCapacity(aCapacity)
    {
        createBuffer();

        if (mMaxLimit == 0)
        {
            mMaxLimit = mCapacity;
        }
        VerifyOrDie(mMaxLimit >= mCapacity);
    }
    
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

    virtual bool insert(const T& item) = 0;
    virtual void remove(const T& item) = 0;
    virtual void clear()    { mCount = 0; };
    virtual void sort() = 0;
    virtual bool contains(const T& item) const = 0;
    virtual T* find(const T& item) = 0;
    virtual const T* find(const T& item) const = 0;

    size_t size() const { return mCount; }
    size_t capacity() const { return mCapacity; }
    bool empty() const { return (mCount == 0); }

    // Array-style access
    T& operator[](size_t index) { VerifyOrDie( index < mCount ); return mBuffer[index]; }
    const T& operator[](size_t index) const { VerifyOrDie( index < mCount ); return mBuffer[index]; }

    // Iterator support
    T* begin() { return mBuffer.Get(); }
    T* end() { return mBuffer.Get() + mCount; }
    const T* begin() const { return mBuffer.Get(); }
    const T* end() const { return mBuffer.Get() + mCount; }

protected:
    size_t mMaxLimit = 0;
    size_t mCapacity = 0;
    size_t mCount = 0;
    Platform::ScopedMemoryBuffer<T> mBuffer;

    RetCode createBuffer() {
        if (mCapacity == 0) {
            return RetCode::kNoInit;
        }

        if (!mBuffer.Calloc(mCapacity)) {
            return RetCode::kNoMem;
        }

        mCount = 0;
        return RetCode::kSuccess;
    }

    RetCode resize(size_t newCapacity) {
        if (newCapacity == mCapacity) {
            return RetCode::kSuccess;
        }
      
        if (newCapacity < mCount) {
            return RetCode::kInUse;
        }
      
        chip::Platform::ScopedMemoryBuffer<T> newBuffer;
        if (!newBuffer.Calloc(newCapacity)) {
            return RetCode::kNoMem;
        }
      
        // Use std::move for efficient transfer
        if (mBuffer.Get() && mCount > 0) {
            std::move(mBuffer.Get(), mBuffer.Get() + mCount, newBuffer.Get());
        }
      
        mBuffer = std::move(newBuffer);
        mCapacity = newCapacity;
        return RetCode::kSuccess;
    }

    RetCode ensureCapacity(size_t requiredCapacity) {
        if (requiredCapacity > mMaxLimit) 
        {
            return RetCode::kNoMem;
        }

        if (requiredCapacity <= mCapacity)
        {
            return RetCode::kSuccess;
        }
  
        size_t newCapacity = std::min(std::max(requiredCapacity, mCapacity * 2), mMaxLimit);
        return resize(newCapacity);
    }

    RetCode insertAtEnd(const T& item) {
        // Check if we need to resize
        RetCode ret = ensureCapacity(mCount + 1);
        if (ret != RetCode::kSuccess) {
            return ret;
        }
        
        // Append to the end
        mBuffer[mCount] = item;
        ++mCount;
        return RetCode::kSuccess;
    }
};

template<typename ItemType>
class CTC_UnorderedSet : public CTC_ContainerClassBase<ItemType>
{
public:
    using Base = CTC_ContainerClassBase<ItemType>;
    using Base::Base;

    CTC_UnorderedSet(size_t aCapacity, size_t aMaxLimit = 0) : Base(aCapacity, aMaxLimit) {}

    bool insert(const ItemType& item) override {
        // Check for duplicate using std::find
        if (this->find(item) != nullptr) {
            return false; //Duplicate
        }
  
        // Append to the end (unordered)
        return (this->insertAtEnd( item) == Base::RetCode::kSuccess);
    }

    void remove(const ItemType& item) override {
        auto* found = this->find(item);
        if (found) {
            // Swap with the last element for O(1) removal (after finding).
            auto* last = &this->mBuffer[this->mCount - 1];
            if (found != last) {
                *found = std::move(*last);
            }
            --this->mCount;
        }
    }

    void sort() override {
        if (this->mCount > 1) {
            std::sort(this->mBuffer.Get(), this->mBuffer.Get() + this->mCount);
        }
    }

    bool contains(const ItemType& item) const override {
        return this->find(item) != nullptr;
    }

    ItemType* find(const ItemType& item) override {
        // Linear search using std::find
        auto it = std::find(this->begin(), this->end(), item);
        return (it != this->end()) ? it : nullptr;
    }

    const ItemType* find(const ItemType& item) const override {
        // Const version of linear search
        auto it = std::find(this->begin(), this->end(), item);
        return (it != this->end()) ? it : nullptr;
    }
};

template<typename KeyType, typename ValueType>
class CTC_UnorderedMap
{
public:
    using PairType = std::pair<KeyType, ValueType>;

    static_assert(std::is_trivially_destructible<PairType>::value, "KeyType and ValueType must be trivially destructible");

private:
    // Custom set that only compares keys for uniqueness
    class PairSet : public CTC_UnorderedSet<PairType>
    {
    public:
        using Base = CTC_UnorderedSet<PairType>;
        using Base::Base;

        PairType* insertUnchecked(const PairType& pair) {
            if (this->insertAtEnd(pair) == Base::RetCode::kSuccess) {
                return &this->mBuffer[this->mCount - 1];
            }
            return nullptr;
        }

        // Override find to only compare keys
        PairType* find(const PairType& pair) override {
            return findByKey(pair.first);
        }

        const PairType* find(const PairType& pair) const override {
            return findByKey(pair.first);
        }

        PairType* findByKey(const KeyType& key) {
            auto it = std::find_if(this->begin(), this->end(),
                                  [&key](const PairType& item) {
                                      return item.first == key;
                                  });
            return (it != this->end()) ? it : nullptr;
        }

        const PairType* findByKey(const KeyType& key) const {
            auto it = std::find_if(this->begin(), this->end(),
                                  [&key](const PairType& item) {
                                      return item.first == key;
                                  });
            return (it != this->end()) ? it : nullptr;
        }

        void removeByKey(const KeyType& key) {
            if (auto* found = findByKey(key)) {
                // Swap with the last element for O(1) removal (after finding).
                auto* last = &this->mBuffer[this->mCount - 1];
                if (found != last) {
                    *found = std::move(*last);
                }
                --this->mCount;
            }
        }

        // Override contains to only check key
        bool contains(const PairType& pair) const override {
            return containsKey(pair.first);
        }

        bool containsKey(const KeyType& key) const {
            return findByKey(key) != nullptr;
        }
    };

    PairSet mPairStorage;

public:
    CTC_UnorderedMap(size_t aCapacity, size_t aMaxLimit = 0) : mPairStorage(aCapacity, aMaxLimit) {}

    bool insert(const KeyType& key, const ValueType& value) {
        return mPairStorage.insert(std::make_pair(key, value));
    }

    bool insert(const PairType& pair) {
        return mPairStorage.insert(pair);
    }

    void remove(const KeyType& key) {
        mPairStorage.removeByKey(key);
    }

    ValueType* getValue(const KeyType& key) {
        auto* pair = mPairStorage.findByKey(key);
        return pair ? &pair->second : nullptr;
    }

    const ValueType* getValue(const KeyType& key) const {
        auto* pair = mPairStorage.findByKey(key);
        return pair ? &pair->second : nullptr;
    }

    // In class CTC_UnorderedMap
    ValueType& operator[](const KeyType& key) {
        auto* pair = mPairStorage.findByKey(key);
        if (pair == nullptr) {
            // Use insertUnchecked since we already know the key is not present.
            pair = mPairStorage.insertUnchecked(std::make_pair(key, ValueType{}));
            VerifyOrDie(pair != nullptr);
        }
        return pair->second;
    }

    bool contains(const KeyType& key) const {
        return mPairStorage.containsKey(key);
    }

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
} // namespace app
} // namespace chip