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

#include <map>
#include <vector>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ModeSelect {

/**
 * Interface to help manage the supported modes of the Mode Select Cluster.
 */
class SupportedModesManager
{

    using ModeOptionStructType = Structs::ModeOptionStruct::Type;

public:
    struct ModeOptionStructIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = const ModeOptionStructType;
        using pointer           = value_type *;
        using reference         = value_type &;

        virtual reference operator*() const = 0;
        virtual pointer operator->()        = 0;
        virtual pointer operator->() const  = 0;

        // Prefix increment
        virtual ModeOptionStructIterator & operator++() = 0;

        virtual bool operator==(const ModeOptionStructIterator & other) const = 0;
        virtual bool operator!=(const ModeOptionStructIterator & other) const = 0;

        virtual ~ModeOptionStructIterator() {}
    };

    /**
     * A factory that can return the ModeOptionStructIterators for a specific endpoint.
     */
    struct ModeOptionStructIteratorFactory
    {
        using const_pointer = const ModeOptionStructIterator *;

        /**
         * Returns the ModeOptionStructIterator to the first option.
         */
        virtual const_pointer begin() const = 0;

        /**
         * Returns the ModeOptionStructIterator to an element after the last option.
         */
        virtual const_pointer end() const = 0;

        virtual ~ModeOptionStructIteratorFactory() {}
    };

    /**
     * Given the endpointId, returns all its supported modes options.
     * @param endpointId
     * @return The iterator facotry for the endpoint, or nullptr if the endpoint doesn't support ModeSelectCluster.
     */
    virtual const ModeOptionStructIteratorFactory * getIteratorFactory(EndpointId endpointId) const = 0;

    /**
     * Given the endpointId and a mode value, find the ModeOptionStruct that matches the mode.
     * @param endpointId The endpoint for which to search the ModeOptionStruct.
     * @param mode The mode for which to search the ModeOptionStruct.
     * @param dataPtr The pointer to receive the ModeOptionStruct, if one is found.
     * @return EMBER_ZCL_STATUS_SUCCESS if sucessfully found the option. Otherwise, returns appropriate status code (found in
     * <app/util/af-enums.h>)
     */
    virtual EmberAfStatus getModeOptionByMode(EndpointId endpointId, uint8_t mode, const ModeOptionStructType ** dataPtr) const = 0;

    virtual ~SupportedModesManager() {}
};

} // namespace ModeSelect
} // namespace Clusters
} // namespace app
} // namespace chip
