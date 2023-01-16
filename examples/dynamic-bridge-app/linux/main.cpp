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

#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app-common/zap-generated/af-structs.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/chip-zcl-zpro-codec.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "ActionCluster.h"
#include "Backend.h"
#include "CommissionableInit.h"
#include "Device.h"
#include "main.h"
#include <app/server/Server.h>

#include "AppMain.h"

#include "bridge/BridgeClustersImpl.h"

#ifdef PW_RPC_ENABLED
#include "Rpc.h"
#include "bridge_service.h"
#include "pw_rpc_system_server/rpc_server.h"
static chip::rpc::Bridge bridge_service;
#endif

#include <cassert>
#include <iostream>
#include <vector>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
Room gRooms[kMaxRooms];

struct CommonAttributeAccessInterface : public chip::app::AttributeAccessInterface
{
    using chip::app::AttributeAccessInterface::AttributeAccessInterface;

    // Find a cluster given a specific endpoint/cluster. Returns nullptr if no such
    // cluster exists at that path.
    static CommonCluster * FindCluster(const chip::app::ConcreteClusterPath & path);

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, TLV::TLVWriter & writer);

    void OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override;
    void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override;
};

CommonCluster * CommonAttributeAccessInterface::FindCluster(const chip::app::ConcreteClusterPath & path)
{
    Device * dev = FindDeviceEndpoint(path.mEndpointId);
    if (dev)
    {
        for (auto c : dev->clusters())
        {
            if (c->GetClusterId() == path.mClusterId)
            {
                return static_cast<CommonCluster *>(c);
            }
        }
    }
    return nullptr;
}

CHIP_ERROR CommonAttributeAccessInterface::Read(const chip::app::ConcreteReadAttributePath & aPath,
                                                chip::app::AttributeValueEncoder & aEncoder)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
    if (!a)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return a->Read(aPath, aEncoder);
}

CHIP_ERROR CommonAttributeAccessInterface::Write(const chip::app::ConcreteDataAttributePath & aPath,
                                                 chip::app::AttributeValueDecoder & aDecoder)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return c->ForwardWriteToBridge(aPath, aDecoder);
}

CHIP_ERROR CommonAttributeAccessInterface::Read(const chip::app::ConcreteReadAttributePath & aPath, chip::TLV::TLVWriter & writer)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
    if (!a)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return a->Read(aPath, writer);
}

void CommonAttributeAccessInterface::OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath)
{
    CommonCluster * c = FindCluster(aPath);
    if (c)
    {
        AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
        if (a)
        {
            a->ListWriteBegin(aPath);
        }
    }
}

void CommonAttributeAccessInterface::OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful)
{
    CommonCluster * c = FindCluster(aPath);
    if (c)
    {
        AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
        if (a)
        {
            a->ListWriteEnd(aPath, aWriteWasSuccessful);
        }
    }
}

chip::Optional<chip::ClusterId> LookupClusterByName(const char * name)
{
    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (!strcmp(name, cluster.name))
        {
            return chip::Optional<chip::ClusterId>(cluster.id);
        }
    }
    return chip::Optional<chip::ClusterId>();
}

std::unique_ptr<GeneratedCluster> CreateCluster(const char * name)
{
    auto id = LookupClusterByName(name);
    return id.HasValue() ? CreateCluster(id.Value()) : nullptr;
}

std::unique_ptr<GeneratedCluster> CreateCluster(chip::ClusterId id)
{
    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (id == cluster.id)
        {
            return std::unique_ptr<GeneratedCluster>(cluster.ctor(::operator new(cluster.size)));
        }
    }
    return nullptr;
}

CHIP_ERROR TLVWriteValue(chip::TLV::TLVWriter & wr, const Span<const char> & v)
{
    return wr.PutString(chip::TLV::AnonymousTag(), v);
}

CHIP_ERROR TLVWriteValue(chip::TLV::TLVWriter & wr, const bool & v)
{
    return wr.PutBoolean(chip::TLV::AnonymousTag(), v);
}

template <typename T>
CHIP_ERROR TLVWriteValue(chip::TLV::TLVWriter & wr, const T & v)
{
    return wr.Put(chip::TLV::AnonymousTag(), v);
}

CHIP_ERROR WriteValueToBuffer(const bool & value, chip::Span<uint8_t> buffer)
{
    if (buffer.size() != 1)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    *(buffer.data()) = value ? 1 : 0;
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR WriteValueToBuffer(const T & value, chip::Span<uint8_t> buffer)
{
    size_t value_size = sizeof(value);
    if (buffer.size() != value_size)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    memcpy(buffer.data(), &value, value_size);
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR WriteValueToBuffer(chip::TLV::TLVReader & reader, chip::Span<uint8_t> buffer)
{
    T v;
    ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, v));
    return WriteValueToBuffer(v, buffer);
}

// Describes a conversion direction between:
// - A binary buffer (passed from ember internals)
// - A TLV data buffer (used by TLVWriter and TLVReader)
enum ConversionDirection
{
    BUFFER_TO_TLV,
    TLV_TO_BUFFER
};

template <typename T>
CHIP_ERROR PerformTLVBufferConversion(std::vector<uint8_t> * tlvData, chip::Span<uint8_t> buffer,
                                      ConversionDirection convert_direction)
{
    CHIP_ERROR err;
    if (convert_direction == BUFFER_TO_TLV)
    {
        // buffer.size() is ignored here, because it was called from the external write ember callback,
        // which does not provide a buffer size
        chip::TLV::TLVWriter wr;
        wr.Init(tlvData->data(), tlvData->size());
        T value;
        memcpy(&value, buffer.data(), sizeof(T));
        err = TLVWriteValue(wr, value);
        wr.Finalize();
        tlvData->resize(wr.GetLengthWritten());
    }
    else
    {
        chip::TLV::TLVReader rd;
        rd.Init(tlvData->data(), tlvData->size());
        rd.Next();
        err = WriteValueToBuffer<T>(rd, buffer);
    }
    return err;
}

CHIP_ERROR PerformTLVBufferConversionForType(std::vector<uint8_t> * tlvData, chip::Span<uint8_t> buffer, EmberAfAttributeType type,
                                             ConversionDirection convert_direction)
{
    switch (type)
    {
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<Span<const char>>(tlvData, buffer, convert_direction);
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<ByteSpan>(tlvData, buffer, convert_direction);
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // structs not supported yet
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<float>(tlvData, buffer, convert_direction);
    case ZCL_DOUBLE_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<double>(tlvData, buffer, convert_direction);
    case ZCL_INT8S_ATTRIBUTE_TYPE:
    case ZCL_INT16S_ATTRIBUTE_TYPE:
    case ZCL_INT24S_ATTRIBUTE_TYPE:
    case ZCL_INT32S_ATTRIBUTE_TYPE:
    case ZCL_INT40S_ATTRIBUTE_TYPE:
    case ZCL_INT48S_ATTRIBUTE_TYPE:
    case ZCL_INT56S_ATTRIBUTE_TYPE:
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<int64_t>(tlvData, buffer, convert_direction);
    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_INT16U_ATTRIBUTE_TYPE:
    case ZCL_INT24U_ATTRIBUTE_TYPE:
    case ZCL_INT32U_ATTRIBUTE_TYPE:
    case ZCL_INT40U_ATTRIBUTE_TYPE:
    case ZCL_INT48U_ATTRIBUTE_TYPE:
    case ZCL_INT56U_ATTRIBUTE_TYPE:
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<uint64_t>(tlvData, buffer, convert_direction);
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<bool>(tlvData, buffer, convert_direction);
    case ZCL_BITMAP32_ATTRIBUTE_TYPE:
        return PerformTLVBufferConversion<uint32_t>(tlvData, buffer, convert_direction);
    default:
        // Assume integer
        return PerformTLVBufferConversion<int64_t>(tlvData, buffer, convert_direction);
    }
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Actions::Commands::InstantAction::DecodableType & commandData)
{
    // No actions are implemented, just return status NotFound.
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if ((endpointIndex >= CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) || (gDevices[endpointIndex] == nullptr))
    {
        ChipLogError(DeviceLayer, "Could not find dynamic endpoint: %d", endpoint);
        return EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT;
    }

    chip::app::AttributeAccessInterface * accessInterface = chip::app::GetAttributeAccessOverride(endpoint, clusterId);

    if (accessInterface == nullptr)
    {
        ChipLogError(DeviceLayer, "Cluster %d has no attribute access override", clusterId);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // adding 64 bytes as padding to include the staging buffer used by
    // TLVReader and TLVWriter, which is 17 bytes
    std::vector<uint8_t> tlvData(attributeMetadata->size + 64);

    // read the attribute and write it to `data`
    chip::TLV::TLVWriter writer;
    writer.Init(tlvData.data(), tlvData.size());

    // this cast is safe because all the registered attribute accessors are of type `CommonAttributeAccessInterface`. See `main()`
    CHIP_ERROR err = static_cast<CommonAttributeAccessInterface *>(accessInterface)
                         ->Read(chip::app::ConcreteDataAttributePath(endpoint, clusterId, attributeMetadata->attributeId), writer);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "%" CHIP_ERROR_FORMAT, err.Format());
        ChipLogError(DeviceLayer, "Attribute access interface failed to read attribute %d, for endpoint %d cluster %d",
                     attributeMetadata->attributeId, endpoint, clusterId);
        return EMBER_ZCL_STATUS_FAILURE;
    }
    writer.Finalize();
    tlvData.resize(writer.GetLengthWritten());

    // read from `data` and write to `buffer`

    // maxReadLength here is the maximum number of bytes to read from the attribute value and to write into the buffer.
    err = PerformTLVBufferConversionForType(&tlvData, chip::Span<uint8_t>(buffer, maxReadLength), attributeMetadata->attributeType,
                                            TLV_TO_BUFFER);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "%" CHIP_ERROR_FORMAT, err.Format());
        ChipLogError(DeviceLayer, "Failed to write attribute to buffer. Endpoint %d, Cluster %d, Attribute %d", endpoint, clusterId,
                     attributeMetadata->attributeId);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if ((endpointIndex >= CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) || (gDevices[endpointIndex] == nullptr))
    {
        ChipLogError(DeviceLayer, "Could not find dynamic endpoint: %d", endpoint);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    chip::app::AttributeAccessInterface * accessInterface = chip::app::GetAttributeAccessOverride(endpoint, clusterId);

    if (accessInterface == nullptr)
    {
        ChipLogError(DeviceLayer, "Cluster %d has no attribute access override", clusterId);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // adding 64 bytes as padding to include the staging buffer used by
    // TLVReader and TLVWriter, which is 17 bytes
    std::vector<uint8_t> tlvData(attributeMetadata->size + 64);

    // read from `buffer` and write to `data`

    // buffer size will not be used in this code path, so we set it to 0. See `PerformTLVBufferConversion`
    CHIP_ERROR err = PerformTLVBufferConversionForType(&tlvData, chip::Span<uint8_t>(buffer, 0), attributeMetadata->attributeType,
                                                       BUFFER_TO_TLV);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "%" CHIP_ERROR_FORMAT, err.Format());
        ChipLogError(DeviceLayer, "Failed to read value from buffer: Endpoint %d, Cluster %d, Attribute %d", endpoint, clusterId,
                     attributeMetadata->attributeId);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    // read from `data` and write to attribute
    chip::TLV::TLVReader reader;
    reader.Init(tlvData.data(), tlvData.size());
    reader.Next();
    chip::app::AttributeValueDecoder decoder(reader, chip::Access::SubjectDescriptor());
    err =
        accessInterface->Write(chip::app::ConcreteReadAttributePath(endpoint, clusterId, attributeMetadata->attributeId), decoder);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "%" CHIP_ERROR_FORMAT, err.Format());
        ChipLogError(DeviceLayer,
                     "Attribute access interface failed to write attribute value. Endpoint %d, Cluster %d, Attribute %d", endpoint,
                     clusterId, attributeMetadata->attributeId);
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

Device * FindDeviceEndpoint(chip::EndpointId id)
{
    for (auto dev : gDevices)
    {
        if (dev && dev->GetEndpointId() == id)
        {
            return dev;
        }
    }
    return nullptr;
}

int AddDeviceEndpoint(Device * dev)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (nullptr == gDevices[index])
        {
            gDevices[index] = dev;
            EmberAfStatus ret;
            while (true)
            {
                // Todo: Update this to schedule the work rather than use this lock
                dev->SetEndpointId(gCurrentEndpointId);
                ret =
                    emberAfSetDynamicEndpoint(index, gCurrentEndpointId, dev->endpointType(), dev->versions(), dev->deviceTypes());
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    gCurrentEndpointId, index);
                    return index;
                }
                if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: %d!", ret);
                    gDevices[index] = nullptr;
                    return -1;
                }
                // Handle wrap condition
                if (++gCurrentEndpointId < gFirstDynamicEndpointId)
                {
                    gCurrentEndpointId = gFirstDynamicEndpointId;
                }
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int RemoveDeviceEndpoint(Device * dev)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (gDevices[index] == dev)
        {
            // Todo: Update this to schedule the work rather than use this lock
            DeviceLayer::StackLock lock;
            EndpointId ep   = emberAfClearDynamicEndpoint(index);
            gDevices[index] = nullptr;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return index;
        }
        index++;
    }
    return -1;
}

Room * FindRoom(const std::string & name)
{
    for (auto & room : gRooms)
    {
        if (room.GetName() == name)
        {
            return &room;
        }
    }
    return nullptr;
}

chip::Span<Action *> GetActionListInfo(chip::EndpointId parentId)
{
    return chip::Span<Action *>();
}

void ApplicationInit()
{
#ifdef PW_RPC_ENABLED
    chip::rpc::Init();

    pw::rpc::system_server::Server().RegisterService(bridge_service);
#endif

    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;
    StartUserInput();
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    std::vector<CommonAttributeAccessInterface> clusterAccess;
    clusterAccess.reserve(std::extent<decltype(clusters::kKnownClusters)>::value);
    for (auto & entry : clusters::kKnownClusters)
    {
        // Desciptor clusters should not be overridden.
        if (entry.id != chip::app::Clusters::Descriptor::Id)
        {
            clusterAccess.emplace_back(chip::Optional<EndpointId>(), entry.id);
            registerAttributeAccessOverride(&clusterAccess.back());
        }
    }

    ChipLinuxAppMainLoop();
    return 0;
}
