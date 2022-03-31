/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <stdint.h>
#include <sys/types.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <platform/AttributeList.h>

namespace chip {
namespace DeviceLayer {

static constexpr size_t kMaxLabelNameLength  = 16;
static constexpr size_t kMaxLabelValueLength = 16;

class DeviceInfoProvider
{
public:
    /**
     * Template used to iterate the stored group data
     */
    template <typename T>
    class Iterator
    {
    public:
        virtual ~Iterator() = default;
        /**
         *  @retval The number of entries in total that will be iterated.
         */
        virtual size_t Count() = 0;
        /**
         *  @param[out] item  Value associated with the next element in the iteration.
         *  @retval true if the next entry is successfully retrieved.
         *  @retval false if no more entries can be found.
         */
        virtual bool Next(T & item) = 0;
        /**
         * Release the memory allocated by this iterator.
         * Must be called before the pointer goes out of scope.
         */
        virtual void Release() = 0;

    protected:
        Iterator() = default;
    };

    using FixedLabelType = app::Clusters::FixedLabel::Structs::LabelStruct::Type;

    using FixedLabelIterator = Iterator<FixedLabelType>;

    DeviceInfoProvider() = default;

    virtual ~DeviceInfoProvider() = default;

    // Not copyable
    DeviceInfoProvider(const DeviceInfoProvider &) = delete;
    DeviceInfoProvider & operator=(const DeviceInfoProvider &) = delete;

    // Iterators
    /**
     *  Creates an iterator that may be used to obtain the list of user labels associated with the given endpoint.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the user label during the iteration is currently not supported, and may yield unexpected behaviour.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) = 0;
};

/**
 * Instance getter for the global DeviceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Device Info Provider. Assume never null.
 */
DeviceInfoProvider * GetDeviceInfoProvider();

/**
 * Instance setter for the global DeviceInfoProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the Device Info Provider
 */
void SetDeviceInfoProvider(DeviceInfoProvider * provider);

} // namespace DeviceLayer
} // namespace chip
