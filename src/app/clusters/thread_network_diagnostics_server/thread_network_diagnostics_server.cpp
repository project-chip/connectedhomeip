/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/AttributeDataElement.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPTLVTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConnectivityManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ThreadNetworkDiagnostics::Attributes;
using namespace chip::DeviceLayer;

namespace {

class ThreadDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the EthernetNetworkDiagnostics cluster on all endpoints.
    ThreadDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ThreadNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter, bool * aDataRead) override;

private:
    CHIP_ERROR ReadIfSupported(chip::AttributeId attributeId, TLV::TLVWriter * aWriter);
};

ThreadDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR ThreadDiagosticsAttrAccess::Read(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter, bool * aDataRead)
{
    if (aClusterInfo.mClusterId != ThreadNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *aDataRead = true;

    return ReadIfSupported(aClusterInfo.mFieldId, aWriter);
}

CHIP_ERROR ThreadDiagosticsAttrAccess::ReadIfSupported(chip::AttributeId attributeId, TLV::TLVWriter * aWriter)
{
    uint8_t * pData   = nullptr;
    uint16_t dataLen  = 0;
    TLVType valueType = kTLVType_NotSpecified;

    // GetThreadNetworkDiagnosticAttributeInfo will alloc memory for the data returned.
    CHIP_ERROR err = ConnectivityMgr().GetThreadNetworkDiagnosticAttributeInfo(attributeId, &pData, dataLen, valueType);

    if (err == CHIP_NO_ERROR)
    {
        switch (valueType)
        {
        case kTLVType_UnsignedInteger: {
            if (dataLen == sizeof(uint8_t))
            {
                aWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), Encoding::Get8(pData));
            }
            else if (dataLen == sizeof(uint16_t))
            {
                aWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::Encoding::LittleEndian::Get16(pData));
            }
            else if (dataLen == sizeof(uint32_t))
            {
                aWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::Encoding::LittleEndian::Get32(pData));
            }
            else if (dataLen == sizeof(uint64_t))
            {
                aWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), chip::Encoding::LittleEndian::Get64(pData));
            }
            else
            {
                err = CHIP_ERROR_INVALID_INTEGER_VALUE;
            }
        }
        break;

        case kTLVType_UTF8String: {
            aWriter->PutString(TLV::ContextTag(AttributeDataElement::kCsTag_Data), reinterpret_cast<char *>(pData), dataLen);
        }
        break;

        case kTLVType_ByteString: {
            aWriter->PutBytes(TLV::ContextTag(AttributeDataElement::kCsTag_Data), pData, dataLen);
        }
        break;

        default: {
            err = CHIP_ERROR_NOT_IMPLEMENTED;
        }
        break;
        }
    }

    if (pData != nullptr)
    {
        CHIPPlatformMemoryFree(pData);
    }
    return err;
}
} // anonymous namespace

bool emberAfThreadNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = ThreadNetworkDiagnostics::Attributes::SetOverrunCount(endpoint, 0);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to reset OverrunCount attribute");
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emberAfThreadNetworkDiagnosticsClusterServerInitCallback(EndpointId endpoint)
{
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAttrAccess);
        attrAccessRegistered = true;
    }
}
