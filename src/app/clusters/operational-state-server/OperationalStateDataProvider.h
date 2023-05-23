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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace chip {

constexpr size_t kOperationalStateLabelMaxSize = 64u;
constexpr size_t kPhaseMaxSize = 32u;
constexpr size_t kOperationalStateMaxSerializedSize = 512u;

/**
 * A class that can create a link list for operational state list of operational state cluster
 */
struct OperationalStateStructDynamicList : public app::Clusters::OperationalState::Structs::OperationalStateStruct::Type
{
    char OperationalStateLabel[kOperationalStateLabelMaxSize];
    OperationalStateStructDynamicList *Next;
};

/**
 * A class that can create a link list for phase list of operational state cluster
 */
struct PhaseListCharSpan
{
    char Phase[kPhaseMaxSize];
    CharSpan phase;
    PhaseListCharSpan *Next;
};

/**
 * Interface to help manage the phase list and operation state list of the Operational State Cluster.
 */
class OperationalStateDataProvider
{

public:
    template <typename T>
    using DataModelListTemplate = app::DataModel::List<T>;
    using OperationalStateStructType = app::Clusters::OperationalState::Structs::OperationalStateStruct::Type;
    using OperationalStateStructTypeList = app::DataModel::List<OperationalStateStructType>;
    using PhaseListType = chip::CharSpan;
    using PhaseList        = chip::app::DataModel::List<const chip::CharSpan>;

    ~OperationalStateDataProvider() {}

    /**
     * Init the operational state data provider.
     * @param persistentStorage The refence of pesistent storage object.
     * @return void
     */
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

    /**
     * Store operational state list to storage.
     * @param endpointId The endpoint for which to save the list[OperationalStateStruct].
     * @param clusterId The cluster for which to save the list[OperationalStateStruct].
     * @param operationalStateList The operational state list for which to save.
     *  Template for save operational state alias cluster
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    template <typename T>
    CHIP_ERROR StoreOperationalStateList(EndpointId endpoint, ClusterId clusterId, const DataModelListTemplate<T> & operationalStateList);

    /**
     * Load operational state list from storage.
     * @param endpointId The endpoint for which to load the list[OperationalStateStruct].
     * @param clusterId The cluster for which to load the list[OperationalStateStruct].
     * @param operationalStateList The pointer to save operational state list.
     * @param size The number of operational state list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR LoadOperationalStateList(EndpointId endpoint, ClusterId clusterId, OperationalStateStructDynamicList** operationalStateList, size_t & size);

     /**
     * Clear operational state list from storage.
     * @param endpointId The endpoint for which to clear the list[OperationalStateStruct].
     * @param clusterId The cluster for which to clear the list[OperationalStateStruct].
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearOperationalStateList(EndpointId endpoint, ClusterId clusterId);

     /**
     * Rlease OperationalStateStructDynamicList
     * @param operationalStateList The pointer for which to clear the OperationalStateStructDynamicList.
     * @return void
     */
    void ReleaseOperationalStateList(OperationalStateStructDynamicList * operationalStateList);

     /**
     * use operational state id to get OperationalStateStruct.
     * @param endpointId The endpoint for which to get the OperationalStateStruct.
     * @param clusterId The cluster for which to get the OperationalStateStruct.
     * @param targetOp The target operational state struct.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR UseOpStateIDGetOpStateStruct(EndpointId endpoint, ClusterId clusterId, OperationalStateStructDynamicList& targetOp);

    /**
     * Store phase list to storage.
     * @param endpointId The endpoint for which to save the list[string].
     * @param clusterId The cluster for which to save the list[string].
     * @param phaseList The phase list for which to save.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR StorePhaseList(EndpointId endpoint, ClusterId clusterId, const PhaseList & phaseList);

    /**
     * Load phase list from storage.
     * @param endpointId The endpoint for which to load the list[string].
     * @param clusterId The cluster for which to load the list[string].
     * @param phaseList The pointer to load phase list.
     * @param size The number of phase list's item.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR LoadPhaseList(EndpointId endpoint, ClusterId clusterId, PhaseListCharSpan **phaseList, size_t & size);

     /**
     * Rlease PhaseListCharSpan
     * @param phaseList The pointer for which to clear the PhaseListCharSpan.
     * @return void
     */
    void ReleasePhaseList(PhaseListCharSpan * phaseList);

     /**
     * Clear phase list from storage.
     * @param endpointId The endpoint for which to clear the list[string].
     * @param clusterId The cluster for which to clear the list[string].
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR ClearPhaseStateList(EndpointId endpoint, ClusterId clusterId);

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

template <typename T>
CHIP_ERROR OperationalStateDataProvider::StoreOperationalStateList(EndpointId endpoint, ClusterId clusterId, const DataModelListTemplate<T> & operationalStateList)
{
    uint8_t buffer[kOperationalStateMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto const & operationalState : operationalStateList)
    {
        ReturnErrorOnFailure(operationalState.Encode(writer, TLV::AnonymousTag()));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OperationalStateOpStateList(endpoint, clusterId).KeyName(), buffer,
                                               static_cast<uint16_t>(writer.GetLengthWritten()));

}


} // namespace chip
