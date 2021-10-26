/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <cstring>
#include <app/clusters/mode-select-server/supported-modes-manager.h>
#include <map>
#include <vector>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelectCluster {

/**
 * This implementation statically defines the options.
 */

class HeapBasedSupportedModesManager : public chip::app::Clusters::ModeSelectCluster::SupportedModesManager
{
    using ModeOptionStructType = Structs::ModeOptionStruct::Type;
    using storage_value_type        = const ModeOptionStructType*;

    static const ModeOptionStructType blackOption;
    static const ModeOptionStructType cappuccinoOption;
    static const ModeOptionStructType espressoOption;

    static const ModeOptionStructType* coffeeOptions[];
    static const Span<storage_value_type> coffeeOptionsSpan;
    static const std::map<EndpointId, Span<storage_value_type>> optionsByEndpoints;



public:

    static const HeapBasedSupportedModesManager instance;

    struct Iterator: public chip::app::Clusters::ModeSelectCluster::SupportedModesManager::ModeOptionStructIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using pointer           = storage_value_type*;
        using base_iterator_type= chip::app::Clusters::ModeSelectCluster::SupportedModesManager::ModeOptionStructIterator;

        Iterator(const pointer aPtr):mPtr(aPtr) {}
        ~Iterator() = default;

        const ModeOptionStructType& operator*() const override { return **mPtr; }
        const ModeOptionStructType* operator->() override { return *mPtr; }
        const ModeOptionStructType* operator->() const override { return *mPtr; }

        // Prefix increment
        base_iterator_type& operator++() override { ++mPtr; return *this; }

        bool operator== (const base_iterator_type& other) const override
        {
            // Warning: we are not doing type check
            // TODO: use of typeid requires -frtti
            // if (typeid(other) != typeid(*this))
            // {
            //     return false;
            // }
            return this->operator->() == other.operator->();
        }
        bool operator!= (const base_iterator_type& other) const override { return !((*this) == other); }

        private:
            pointer mPtr;

    };

    struct IteratorFactory: public chip::app::Clusters::ModeSelectCluster::SupportedModesManager::ModeOptionStructIteratorFactory
    {
        using pointer = Iterator*;
        using const_pointer = const pointer;

        IteratorFactory(const Span<storage_value_type>& aSupportedOptions): _begin(Iterator(aSupportedOptions.data())), _end(Iterator(aSupportedOptions.data() + aSupportedOptions.size())) { }
        ~IteratorFactory() = default;

        const Iterator* begin() const override { return &_begin; }
        const Iterator* end() const override { return &_end; }

        private:
            const Iterator _begin;
            const Iterator _end;

    };

    const IteratorFactory* getIteratorFactory(EndpointId endpointId) const override;

    EmberAfStatus getModeOptionByMode(EndpointId endpointId, uint8_t mode, const ModeOptionStructType *& dataPtr) const override;

    ~HeapBasedSupportedModesManager() {};

    HeapBasedSupportedModesManager(): HeapBasedSupportedModesManager(&optionsByEndpoints) {}

    static inline const HeapBasedSupportedModesManager& getHeapBasedSupportedModesManagerInstance() { return instance; }

private:
    HeapBasedSupportedModesManager(const std::map<EndpointId, Span<storage_value_type>>* const supportedModes): _iteratorFactoriesByEndpoints(std::map<EndpointId, IteratorFactory>()) {
        for(auto& entry: *supportedModes)
        {
            _iteratorFactoriesByEndpoints.insert(std::pair<EndpointId, IteratorFactory>(entry.first, IteratorFactory(entry.second)));
        }
    }
    // TODO: Implement move constructor?

    std::map<EndpointId, IteratorFactory> _iteratorFactoriesByEndpoints;
};


} // namespace ModeSelectCluster
} // namespace Clusters
} // namespace app
} // namespace chip
