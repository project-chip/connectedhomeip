/*
 *
 *    Copyright (c) 2025 Matter Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/tls-certificate-management-server/CertificateTableImpl.h>
#include <app/clusters/tls-certificate-management-server/IncrementingIdHelper.h>
#include <app/clusters/tls-client-management-server/TlsClientManagementCluster.h>
#include <app/storage/FabricTableImpl.ipp>
#include <clusters/TlsClientManagement/Commands.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CHIPMem.h>
#include <tls-client-management-instance.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace chip::app::Storage;
using namespace chip::app::Storage::Data;
using namespace chip::Platform;
using namespace chip::TLV;
using namespace Protocols::InteractionModel;

using EndpointSerializer = DefaultSerializer<TlsEndpointId, TlsClientManagementDelegate::EndpointStructType>;
using InnerIterator      = TableEntryDataConvertingIterator<TlsEndpointId, TlsClientManagementDelegate::EndpointStructType>;

namespace {
enum class TagEndpoint : uint8_t
{
    kTlsEndpointId,
    kEndpointPayload,
    kNextId,
    kStoredFabricIndex,
    kEndpointMapping,
};

static constexpr size_t kPersistentBufferNextIdBytes =
    EstimateStructOverhead(sizeof(uint16_t), // mNextId
                           EstimateStructOverhead(sizeof(EndpointId), sizeof(FabricIndex)) *
                               (kMaxProvisionedEndpoints * CHIP_CONFIG_MAX_FABRICS)); // mEndpointMapping

static constexpr size_t kTlsEndpointMaxBytes =
    EstimateStructOverhead(sizeof(chip::FabricIndex),                                    // Fabric ID
                           sizeof(uint16_t),                                             // endpointID
                           EstimateStructOverhead(sizeof(uint16_t),                      /* endpointID */
                                                  sizeof(uint8_t) * kSpecMaxHostname,    /* hostname */
                                                  sizeof(uint16_t),                      /* port */
                                                  sizeof(uint16_t),                      /* caid */
                                                  sizeof(DataModel::Nullable<uint16_t>), /* ccdid */
                                                  sizeof(uint8_t),                       /*  referenceCount */
                                                  sizeof(chip::FabricIndex) /* fabricIndex */));
struct BufferedEndpoint
{
    TlsClientManagementDelegate::EndpointStructType mEndpoint;
    PersistenceBuffer<kTlsEndpointMaxBytes> mBuffer;
};

class GlobalEndpointData : public PersistableData<kPersistentBufferNextIdBytes>
{
    IncrementingIdHelper<TlsEndpointId, kMaxProvisionedEndpoints * CHIP_CONFIG_MAX_FABRICS> mEndpoints;
    EndpointId mEndpointId = kInvalidEndpointId;

public:
    GlobalEndpointData(EndpointId endpoint) : mEndpointId(endpoint) {}
    ~GlobalEndpointData() {}

    void Clear() override { mEndpoints.Clear(); }

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kInvalidEndpointId != mEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
        key = DefaultStorageKeyAllocator::TlsClientEndpointGlobalDataKey(mEndpointId);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TagEndpoint::kNextId), mEndpoints.mNextId));

        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(
            writer.StartContainer(TLV::ContextTag(TagEndpoint::kEndpointMapping), TLV::kTLVType_Array, entryMapContainer));
        ReturnErrorOnFailure(mEndpoints.SerializeMapping(writer, TLV::ContextTag(TagEndpoint::kTlsEndpointId),
                                                         TLV::ContextTag(TagEndpoint::kStoredFabricIndex)));
        ReturnErrorOnFailure(writer.EndContainer(entryMapContainer));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagEndpoint::kNextId)));
        ReturnErrorOnFailure(reader.Get(mEndpoints.mNextId));

        TLV::TLVType entryMapContainer;
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(TagEndpoint::kEndpointMapping)));
        ReturnErrorOnFailure(reader.EnterContainer(entryMapContainer));
        ReturnErrorOnFailure(mEndpoints.DeserializeMapping(reader, TLV::ContextTag(TagEndpoint::kTlsEndpointId),
                                                           TLV::ContextTag(TagEndpoint::kStoredFabricIndex)));
        ReturnErrorOnFailure(reader.ExitContainer(entryMapContainer));

        return reader.ExitContainer(container);
    }

    CHIP_ERROR Load(PersistentStorageDelegate * storage) // NOLINT(bugprone-derived-method-shadowing-base-method)
    {
        CHIP_ERROR err = PersistableData::Load(storage);
        return err.NoErrorIf(CHIP_ERROR_NOT_FOUND); // NOT_FOUND is OK; DataAccessor::Load already called Clear()
    }

    CHIP_ERROR GetNextId(FabricIndex fabric, uint16_t & id)
    {
        ReturnErrorOnFailure(mEndpoints.GetNextId(UINT16_MAX));
        return mEndpoints.ConsumeId(fabric, id);
    }

    CHIP_ERROR Remove(PersistentStorageDelegate & storage, EndpointTable & table, FabricIndex fabric, uint16_t id)
    {
        return mEndpoints.Remove(table, fabric, TlsEndpointId(id), [&, this]() { return Save(&storage); });
    }

    CHIP_ERROR RemoveAll(PersistentStorageDelegate & storage, FabricIndex fabric)
    {
        return mEndpoints.RemoveAll(fabric, [&, this]() { return Save(&storage); });
    }
};
} // namespace

//
// EndpointTable storage template specialization
//

template <>
StorageKeyName EndpointSerializer::EndpointEntryCountKey(EndpointId endpoint_id)
{
    return DefaultStorageKeyAllocator::TlsClientEndpointCountKey(endpoint_id);
}

template <>
StorageKeyName EndpointSerializer::FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint)
{
    return DefaultStorageKeyAllocator::TlsClientEndpointFabricDataKey(fabric, endpoint);
}

template <>
StorageKeyName EndpointSerializer::FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx)
{
    return DefaultStorageKeyAllocator::TlsClientEndpointEntityKey(fabric, endpoint, idx);
}

template <>
constexpr size_t EndpointSerializer::kEntryMaxBytes()
{
    return kTlsEndpointMaxBytes;
}

template <>
constexpr uint16_t EndpointSerializer::kMaxPerFabric()
{
    return kMaxProvisionedEndpoints;
}

template <>
constexpr uint16_t EndpointSerializer::kMaxPerEndpoint()
{
    return UINT16_MAX;
}

template <>
CHIP_ERROR EndpointSerializer::SerializeId(TLV::TLVWriter & writer, const TlsEndpointId & id)
{
    return writer.Put(TLV::ContextTag(TagEndpoint::kTlsEndpointId), id.mEndpointId);
}

template <>
CHIP_ERROR EndpointSerializer::DeserializeId(TLV::TLVReader & reader, TlsEndpointId & id)
{
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TagEndpoint::kTlsEndpointId)));
    return reader.Get(id.mEndpointId);
}

template <>
CHIP_ERROR EndpointSerializer::SerializeData(TLV::TLVWriter & writer, const TlsClientManagementDelegate::EndpointStructType & data)
{
    return data.EncodeForRead(writer, TLV::ContextTag(TagEndpoint::kEndpointPayload), data.GetFabricIndex());
}

template <>
CHIP_ERROR EndpointSerializer::DeserializeData(TLV::TLVReader & reader, TlsClientManagementDelegate::EndpointStructType & data)
{
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(TagEndpoint::kEndpointPayload)));
    return data.Decode(reader);
}

template <>
void EndpointSerializer::Clear(TlsClientManagementDelegate::EndpointStructType & data)
{
    new (&data) TlsClientManagementDelegate::EndpointStructType();
}

template class chip::app::Storage::FabricTableImpl<TlsEndpointId, TlsClientManagementDelegate::EndpointStructType>;

CHIP_ERROR TlsClientManagementCommandDelegate::Init(PersistentStorageDelegate & storage)
{
    mStorage = &storage;
    mProvisioned.SetEndpoint(EndpointId(1));
    return mProvisioned.Init(storage);
}

CHIP_ERROR TlsClientManagementCommandDelegate::ForEachEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                                                               LoadedEndpointCallback callback)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_ERROR_INTERNAL);

    BufferedEndpoint endpoint;
    return mProvisioned.IterateEntries(fabric, endpoint.mBuffer, [&](auto & iterator) {
        InnerIterator innerIter(iterator);
        while (innerIter.Next(endpoint.mEndpoint))
        {
            ReturnErrorOnFailure(callback(endpoint.mEndpoint));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TlsClientManagementCommandDelegate::GetEndpointId(FabricIndex fabric, uint16_t & id)
{
    UniquePtr<GlobalEndpointData> globalData(New<GlobalEndpointData>(EndpointId(1)));
    VerifyOrReturnError(globalData, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(globalData->Load(mStorage));
    ReturnErrorOnFailure(globalData->GetNextId(fabric, id));

    return globalData->Save(mStorage);
}

ClusterStatusCode TlsClientManagementCommandDelegate::ProvisionEndpoint(
    EndpointId matterEndpoint, FabricIndex fabric,
    const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType & provisionReq, uint16_t & endpointID)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), ClusterStatusCode(Status::ConstraintError));
    VerifyOrReturnError(mStorage != nullptr, ClusterStatusCode(Status::ConstraintError));

    // Find existing value to update & check for port/name collisions
    uint16_t numInFabric = 0;

    ClusterStatusCode installedCheck(Status::Failure);
    BufferedEndpoint endpoint;
    CHIP_ERROR entryCheck = mProvisioned.IterateEntries(fabric, endpoint.mBuffer, [&](auto & iterator) {
        InnerIterator innerIter(iterator);
        while (innerIter.Next(endpoint.mEndpoint))
        {
            numInFabric++;
            auto & endpointStruct = endpoint.mEndpoint;
            // A host/port collision is detected if we are either:
            //  - provisioning a new endpoint (endpointID is null)
            //  - updating an existing endpoint, but the colliding endpoint is not the one being updated.
            if (endpointStruct.hostname.data_equal(provisionReq.hostname) && (endpointStruct.port == provisionReq.port) &&
                (provisionReq.endpointID.IsNull() || provisionReq.endpointID.Value() != endpointStruct.endpointID))
            {
                installedCheck = ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kEndpointAlreadyInstalled);
                return CHIP_ERROR_BAD_REQUEST;
            }
        }
        return CHIP_NO_ERROR;
    });
    VerifyOrReturnValue(entryCheck == CHIP_NO_ERROR, installedCheck);

    if (provisionReq.endpointID.IsNull())
    {
        VerifyOrReturnError(numInFabric < mTlsClientManagementCluster->GetMaxProvisioned(),
                            ClusterStatusCode(Status::ResourceExhausted));
        EndpointSerializer::Clear(endpoint.mEndpoint);
        auto & endpointStruct = endpoint.mEndpoint;

        uint16_t nextId;
        ReturnValueOnFailure(GetEndpointId(fabric, nextId), ClusterStatusCode(Status::ResourceExhausted));
        endpointStruct.endpointID = nextId;
        endpointID                = endpointStruct.endpointID;
    }
    // Updating existing value
    else
    {
        TlsEndpointId localId(provisionReq.endpointID.Value());
        ReturnValueOnFailure(mProvisioned.GetTableEntry(fabric, localId, endpoint.mEndpoint, endpoint.mBuffer),
                             ClusterStatusCode(Status::NotFound));
        endpointID = provisionReq.endpointID.Value();
    }

    auto & endpointStruct         = endpoint.mEndpoint;
    endpointStruct.hostname       = provisionReq.hostname;
    endpointStruct.port           = provisionReq.port;
    endpointStruct.caid           = provisionReq.caid;
    endpointStruct.ccdid          = provisionReq.ccdid;
    endpointStruct.referenceCount = 0;
    endpointStruct.SetFabricIndex(fabric);

    TlsEndpointId localId(endpointID);
    ReturnValueOnFailure(mProvisioned.SetTableEntry(fabric, localId, endpointStruct, endpoint.mBuffer),
                         ClusterStatusCode(Status::Failure));

    return ClusterStatusCode(Status::Success);
}

CHIP_ERROR TlsClientManagementCommandDelegate::FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                           uint16_t endpointID, LoadedEndpointCallback callback)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), CHIP_ERROR_INTERNAL);

    TlsEndpointId localId(endpointID);
    BufferedEndpoint endpoint;
    ReturnErrorOnFailure(mProvisioned.GetTableEntry(fabric, localId, endpoint.mEndpoint, endpoint.mBuffer));
    return callback(endpoint.mEndpoint);
}

Status TlsClientManagementCommandDelegate::RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                         uint16_t endpointID)
{
    VerifyOrReturnError(matterEndpoint == EndpointId(1), Status::ConstraintError);
    VerifyOrReturnError(mStorage != nullptr, Status::ConstraintError);

    BufferedEndpoint endpoint;
    TlsEndpointId localId(endpointID);
    CHIP_ERROR result = mProvisioned.GetTableEntry(fabric, localId, endpoint.mEndpoint, endpoint.mBuffer);
    VerifyOrReturnValue(result != CHIP_ERROR_NOT_FOUND, Status::NotFound);
    VerifyOrReturnValue(result == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnValue(endpoint.mEndpoint.referenceCount == 0, Status::InvalidInState);

    UniquePtr<GlobalEndpointData> globalData(New<GlobalEndpointData>(matterEndpoint));
    VerifyOrReturnError(globalData, Status::ResourceExhausted);
    ReturnValueOnFailure(globalData->Load(mStorage), Status::Failure);
    result = globalData->Remove(*mStorage, mProvisioned, fabric, endpointID);
    VerifyOrReturnValue(result != CHIP_ERROR_NOT_FOUND, Status::NotFound);
    VerifyOrReturnValue(result == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

CHIP_ERROR TlsClientManagementCommandDelegate::RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id)
{
    BufferedEndpoint endpoint;
    return mProvisioned.IterateEntries(fabric, endpoint.mBuffer, [&](auto & iterator) {
        InnerIterator innerIter(iterator);
        while (innerIter.Next(endpoint.mEndpoint))
        {
            VerifyOrReturnError(endpoint.mEndpoint.caid != id, CHIP_ERROR_BAD_REQUEST);
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TlsClientManagementCommandDelegate::ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      Tls::TLSCCDID id)
{
    BufferedEndpoint endpoint;
    return mProvisioned.IterateEntries(fabric, endpoint.mBuffer, [&](auto & iterator) {
        InnerIterator innerIter(iterator);
        while (innerIter.Next(endpoint.mEndpoint))
        {
            VerifyOrReturnError(endpoint.mEndpoint.ccdid != id, CHIP_ERROR_BAD_REQUEST);
        }
        return CHIP_NO_ERROR;
    });
}

void TlsClientManagementCommandDelegate::RemoveFabric(FabricIndex fabric)
{
    VerifyOrReturn(mStorage != nullptr);

    ReturnAndLogOnFailure(mProvisioned.RemoveFabric(fabric), Zcl, "Failure clearing TLS endpoints for fabric");

    UniquePtr<GlobalEndpointData> globalData(New<GlobalEndpointData>(EndpointId(1)));
    VerifyOrReturn(globalData);
    ReturnOnFailure(globalData->Load(mStorage));
    ReturnAndLogOnFailure(globalData->RemoveAll(*mStorage, fabric), Zcl, "Failure clearing TLS endpoint data for fabric");
}

CHIP_ERROR TlsClientManagementCommandDelegate::MutateEndpointReferenceCount(EndpointId matterEndpoint, FabricIndex fabric,
                                                                            uint16_t endpointID, int8_t delta)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INTERNAL);

    BufferedEndpoint endpoint;
    TlsEndpointId localId(endpointID);
    ReturnErrorOnFailure(mProvisioned.GetTableEntry(fabric, localId, endpoint.mEndpoint, endpoint.mBuffer));

    if ((0 - delta) <= endpoint.mEndpoint.referenceCount)
    {
        VerifyOrReturnError((int16_t(delta) + int16_t(endpoint.mEndpoint.referenceCount)) <= UINT8_MAX,
                            CHIP_ERROR_INVALID_ARGUMENT);
        endpoint.mEndpoint.referenceCount = (uint8_t) (endpoint.mEndpoint.referenceCount + delta);
    }
    else
    {
        endpoint.mEndpoint.referenceCount = 0;
    }

    return mProvisioned.SetTableEntry(fabric, localId, endpoint.mEndpoint, endpoint.mBuffer);
}

static CertificateTableImpl gCertificateTableInstance;
TlsClientManagementCommandDelegate TlsClientManagementCommandDelegate::instance;

static LazyRegisteredServerCluster<TlsClientManagementCluster> sTlsClientManagementClusterServer;

void emberAfTlsClientManagementClusterInitCallback(EndpointId matterEndpoint)
{
    TEMPORARY_RETURN_IGNORED gCertificateTableInstance.SetEndpoint(EndpointId(1));

    sTlsClientManagementClusterServer.Create(EndpointId(1), TlsClientManagementCommandDelegate::GetInstance(),
                                             gCertificateTableInstance, kMaxProvisionedEndpoints);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(sTlsClientManagementClusterServer.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to register TLS Client Management Cluster on endpoint %u: %" CHIP_ERROR_FORMAT, EndpointId(1),
                     err.Format());
    }
}

void emberAfTlsClientManagementClusterShutdownCallback(EndpointId matterEndpoint)
{
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&sTlsClientManagementClusterServer.Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "TLS Client Management Cluster unregister error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
