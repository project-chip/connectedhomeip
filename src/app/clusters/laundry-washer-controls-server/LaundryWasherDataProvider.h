/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>

#include <lib/support/Span.h>

using namespace chip;
using namespace chip::app::Clusters::LaundryWasherControls;

namespace chip {

constexpr size_t kSpinSpeedMaxSize               = 32u;
constexpr size_t kLaundryWasherMaxSerializedSize = 512u;

/**
 * A class that can create a link list for spin speed list of laundry washer controls cluster
 */
struct SpinSpeedListCharSpan
{
    char SpinSpeed_c[kSpinSpeedMaxSize];
    CharSpan spinSpeed;
    SpinSpeedListCharSpan * Next;
};

/**
 * A class that can create a link list for supported rinses list of laundry washer controls cluster
 */
struct SupportedRinsesListSpan
{
    NumberOfRinsesEnum numberOfRinses;
    SupportedRinsesListSpan * Next;
};

/**
 * Interface to help manage the phase list and operation state list of the Operational State Cluster.
 */
class LaundryWasherDataProvider
{
public:
    using SpinSpeedList       = chip::app::DataModel::List<const chip::CharSpan>;
    using SupportedRinsesList = chip::app::DataModel::List<const NumberOfRinsesEnum>;

    ~LaundryWasherDataProvider() {}

    /**
     * Init the operational state data provider.
     * @param persistentStorage The refence of pesistent storage object.
     * @return void
     */
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

    /**
     * Store spin speed list to storage.
     * @param endpointId The endpoint for which to save the list[string].
     * @param clusterId The cluster for which to save the list[string].
     * @param spinSpeedList The spin speed list for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR StoreSpinSpeedList(EndpointId endpoint, ClusterId clusterId, const SpinSpeedList & spinSpeedList);

    /**
     * Load spin speed list from storage.
     * @param endpointId The endpoint for which to load the list[string].
     * @param clusterId The cluster for which to load the list[string].
     * @param pSpinSpeedList The pointer to load phase list.
     * @param size The number of phase list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR LoadSpinSpeedList(EndpointId endpoint, ClusterId clusterId, SpinSpeedListCharSpan ** pSpinSpeedList, size_t & size);

    /**
     * Rlease spinSpeedListCharSpan
     * @param spinSpeedList The pointer for which to clear the SpinSpeedListCharSpan.
     * @return void
     */
    void ReleaseSpinSpeedList(SpinSpeedListCharSpan * spinSpeedList);

    /**
     * Clear spin speed list from storage.
     * @param endpointId The endpoint for which to clear the list[string].
     * @param clusterId The cluster for which to clear the list[string].
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearSpinSpeedList(EndpointId endpoint, ClusterId clusterId);

    /**
     * Store supported rinses list to storage.
     * @param endpointId The endpoint for which to save the list[string].
     * @param clusterId The cluster for which to save the list[string].
     * @param supportedRinsesList The supported rinses list for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR StoreSupportedRinsesList(EndpointId endpoint, ClusterId clusterId, const SupportedRinsesList & supportedRinsesList);

    /**
     * Load supported rinses list from storage.
     * @param endpointId The endpoint for which to load the list[string].
     * @param clusterId The cluster for which to load the list[string].
     * @param pSupportedRinsesList The pointer to load phase list.
     * @param size The number of phase list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR LoadSupportedRinsesList(EndpointId endpoint, ClusterId clusterId, SupportedRinsesListSpan ** pSupportedRinsesList,
                                       size_t & size);

    /**
     * Rlease SupportedRinsesListSpan
     * @param supportedRinsesList The pointer for which to clear the SupportedRinsesListSpan.
     * @return void
     */
    void ReleaseSupportedRinsesList(SupportedRinsesListSpan * supportedRinsesList);

    /**
     * Clear supported rinses list from storage.
     * @param endpointId The endpoint for which to clear the list[string].
     * @param clusterId The cluster for which to clear the list[string].
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearSupportedRinsesList(EndpointId endpoint, ClusterId clusterId);

private:
    /**
     * Load content by key from storage.
     * @param key  key to save the content.
     * @param buffer buffer to save the content.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR Load(const char * key, MutableByteSpan & buffer);
    PersistentStorageDelegate * mPersistentStorage = nullptr;
};

} // namespace chip
