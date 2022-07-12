/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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

#include "lib/core/DataModelTypes.h"
#include <type_traits>

#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/logging/CHIPLogging.h>
#include <map>
#include <app/util/af-types.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-storage.h>
#include <app/AttributeAccessInterface.h>
#include <lib/core/Optional.h>
#include <app/reporting/reporting.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/DefaultAclStorage.h>
#include <app/server/Dnssd.h>

#include <crypto/PersistentStorageOperationalKeystore.h>
#include <controller/python/ChipDeviceController-StorageDelegate.h>

#include "CommissionableDataProvider.h"

using namespace chip;
using namespace chip::app;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {

struct PyEmberAfAttributeMetadata
{
    uint32_t attributeId;
    uint8_t attributeType;
};

struct PyEmberAfCluster
{
    uint32_t clusterId;
    uint16_t attributeCount;
    PyEmberAfAttributeMetadata *attributes;
};

struct PyEmberAfEndpointType
{
    uint8_t clusterCount;
    PyEmberAfCluster *cluster;
};

using GetAttributeValueCb = void (*)(void *appContext, EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, char * value, uint16_t * size);

GetAttributeValueCb gGetValueCb;
void *gGetAttributeContext = nullptr;

}  // extern "C"

Controller::Python::CommissionableDataProvider gCommissionableDataProvider;
chip::app::DefaultAclStorage gAclStorage;
extern chip::Controller::Python::StorageAdapter * sStorageAdapter;

namespace chip {
namespace Controller {
namespace Python  {

class ClusterServerAdapter : public AttributeAccessInterface
{
public:
    ClusterServerAdapter(EndpointId endpointId, ClusterId clusterId)   :
        AttributeAccessInterface(MakeOptional(static_cast<EndpointId>(endpointId)), clusterId)
                                 {}
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    char mScratchBuf[1280];
};

CHIP_ERROR ClusterServerAdapter::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    uint16_t len;
    gGetValueCb(gGetAttributeContext, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId, mScratchBuf, &len);
    if (len == 0) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLV::TLVReader reader;
    uint8_t *tlvBuffer = reinterpret_cast<uint8_t *>(mScratchBuf);
    reader.Init(tlvBuffer, static_cast<uint32_t>(len));

    ReturnErrorOnFailure(reader.Next());

    return aEncoder.EncodeAttributeReportIB(reader);
}

}
}
}

extern "C" {

ChipError::StorageType pychip_Server_InitializeServer()
{
    chip::DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);
    return CHIP_NO_ERROR.AsInteger();
}

ChipError::StorageType pychip_Server_SetCommissioningParams(uint16_t discriminator)
{
    gCommissionableDataProvider.SetSetupDiscriminator(discriminator);
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kOnNetwork));
    app::DnssdServer::Instance().StartServer();

    return CHIP_NO_ERROR.AsInteger();
}

void pychip_Server_RegisterAttributeGetterCallback(void * appContext, GetAttributeValueCb getValueCb) {
    gGetAttributeContext = appContext;
    gGetValueCb = getValueCb;
}

void pychip_Server_RemoveEndpoint(uint16_t endpointIndex)
{
    emberAfClearDynamicEndpoint(endpointIndex);
}

uint32_t pychip_Server_RegisterEndpoint(EndpointId endpointId, uint16_t endpointIndex, void *buf, DeviceTypeId *deviceTypeList, uint16_t numDeviceTypes)
{
    //
    // TODO: We're just nakedly calling 'new' to allocate these metadata descriptors when we should perhaps be consolidating
    //       these into some kind of 'endpoint' object that consolidates ownership, and then stash that away in some kind of
    //       global list. That will help reclaim those objects if we end-up de-registering that endpoint.
    //
    PyEmberAfEndpointType *pyEndpointType = (PyEmberAfEndpointType *)(buf);
    EmberAfEndpointType *endpoint = new EmberAfEndpointType();

    endpoint->clusterCount = pyEndpointType->clusterCount;
    endpoint->cluster = new EmberAfCluster[endpoint->clusterCount];

    auto *pyCluster = pyEndpointType->cluster;
    EmberAfCluster *cluster = const_cast<EmberAfCluster *>(endpoint->cluster);

    for (int i = 0; i < pyEndpointType->clusterCount; i++) {
        cluster[i].clusterId = pyCluster[i].clusterId;

        cluster[i].attributeCount = pyCluster[i].attributeCount;
        cluster[i].mask = CLUSTER_MASK_SERVER;
        cluster[i].clusterSize = 0;
        cluster[i].functions = nullptr;
        cluster[i].generatedCommandList = nullptr;
        cluster[i].acceptedCommandList = nullptr;

        cluster[i].attributes = new EmberAfAttributeMetadata[cluster[i].attributeCount];
        auto *pyAttributeMetadata = pyCluster[i].attributes;
        EmberAfAttributeMetadata *metadata = const_cast<EmberAfAttributeMetadata *>(cluster[i].attributes);

        for (int j = 0; j < pyCluster[i].attributeCount; j++) {
            metadata[j].attributeId = pyAttributeMetadata[j].attributeId;
            metadata[j].attributeType = ZCL_INT8U_ATTRIBUTE_TYPE;
            metadata[j].mask = ATTRIBUTE_MASK_EXTERNAL_STORAGE;
            metadata[j].size = 0;
        }

        if (cluster[i].clusterId != Clusters::Descriptor::Id) {
            auto *clusterServerAdapter = new chip::Controller::Python::ClusterServerAdapter(endpointId, cluster[i].clusterId);
            registerAttributeAccessOverride(clusterServerAdapter);
        }
    }

    chip::DataVersion *dataVersionStorage = new chip::DataVersion[endpoint->clusterCount];
    chip::Span<chip::DataVersion> versionSpan = {dataVersionStorage, endpoint->clusterCount};

    EmberAfDeviceType *emberDeviceTypeList = new EmberAfDeviceType[numDeviceTypes];

    for (int i = 0; i < numDeviceTypes; i++) {
        emberDeviceTypeList[i].deviceId = static_cast<uint16_t>(deviceTypeList[i]);
        emberDeviceTypeList[i].deviceVersion = 0;
    }

    chip::Span<EmberAfDeviceType> deviceTypeSpan = {emberDeviceTypeList, numDeviceTypes};
    emberAfSetDynamicEndpoint(endpointIndex, endpointId, endpoint, versionSpan, deviceTypeSpan);

    return 0;
}

void pychip_Server_SetDirty(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId)
{
    MatterReportingAttributeChangeCallback(endpointId, clusterId, attributeId);
}

} // extern "C"
