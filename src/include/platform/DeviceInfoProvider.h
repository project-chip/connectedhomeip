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
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/AttributeList.h>

namespace chip {
namespace DeviceLayer {

static constexpr size_t kMaxUserLabelListLength = 10;
static constexpr size_t kMaxLabelNameLength     = 16;
static constexpr size_t kMaxLabelValueLength    = 16;
static constexpr size_t kMaxActiveLocaleLength  = 35;

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
    using UserLabelType  = app::Clusters::UserLabel::Structs::LabelStruct::Type;
    using CalendarType   = app::Clusters::TimeFormatLocalization::CalendarTypeEnum;

    using FixedLabelIterator             = Iterator<FixedLabelType>;
    using UserLabelIterator              = Iterator<UserLabelType>;
    using SupportedLocalesIterator       = Iterator<CharSpan>;
    using SupportedCalendarTypesIterator = Iterator<CalendarType>;

    DeviceInfoProvider() = default;

    virtual ~DeviceInfoProvider() = default;

    // Not copyable
    DeviceInfoProvider(const DeviceInfoProvider &)             = delete;
    DeviceInfoProvider & operator=(const DeviceInfoProvider &) = delete;

    /**
     * @brief Set the storage implementation used for non-volatile storage of device information data.
     *
     * @param storage Pointer to storage instance to set. Cannot be nullptr, will assert.
     */
    void SetStorageDelegate(PersistentStorageDelegate * storage);

    CHIP_ERROR SetUserLabelList(EndpointId endpoint, const AttributeList<UserLabelType, kMaxUserLabelListLength> & labelList);
    CHIP_ERROR ClearUserLabelList(EndpointId endpoint);
    CHIP_ERROR AppendUserLabel(EndpointId endpoint, const UserLabelType & label);

    // Iterators
    /**
     *  Creates an iterator that may be used to obtain the list of labels associated with the given endpoint.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the label during the iteration is currently not supported, and may yield unexpected behaviour.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) = 0;
    virtual UserLabelIterator * IterateUserLabel(EndpointId endpoint)   = 0;

    /**
     *  Creates an iterator that may be used to obtain the list of supported locales of the device.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual SupportedLocalesIterator * IterateSupportedLocales() = 0;

    /**
     *  Creates an iterator that may be used to obtain the list of supported calendar types of the device.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() = 0;

protected:
    PersistentStorageDelegate * mStorage = nullptr;

    /**
     * @brief Set the UserLabel at the specified index of the UserLabelList on a given endpoint
     *
     * @param endpoint - id to UserLabelList on which to set the UserLabel.
     * @param index - index within the UserLabelList for which to set the UserLabel.
     * @param userLabel - user label to set.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_KEY_ID if index exceed the range (Total length - 1),
     *         or other CHIP_ERROR values from implementation on other errors.
     */
    virtual CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) = 0;

    /**
     * @brief Delete the UserLabel at the specified index of the UserLabelList on a given endpoint
     *
     * @param endpoint - id to UserLabelList on which to delete the UserLabel.
     * @param index - index within the UserLabelList for which to remove the UserLabel.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_KEY_ID if index exceed the range (Total length - 1),
     *         or other CHIP_ERROR values from implementation on other errors.
     */
    virtual CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) = 0;

    /**
     * @brief Set the total length of the UserLabelList on a given endpoint
     *
     * @param endpoint - id of the UserLabelList.
     * @param val - total count of the UserLabelList.
     * @return CHIP_NO_ERROR on success, other CHIP_ERROR values from implementation on other errors.
     */
    virtual CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) = 0;

    /**
     * @brief Get the total length of the UserLabelList on a given endpoint
     *
     * @param endpoint - id of the UserLabelList.
     * @param val - output of the total count of the UserLabelList.
     * @return CHIP_NO_ERROR on success, other CHIP_ERROR values from implementation on other errors.
     */
    virtual CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) = 0;
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
