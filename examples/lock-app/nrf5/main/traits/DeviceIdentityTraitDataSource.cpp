/*
 *
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *      A trait data source implementation for the Weave DeviceIdentityTrait.
 *
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ConfigurationManager.h>
#include <traits/include/DeviceIdentityTraitDataSource.h>
#include <schema/include/DeviceIdentityTrait.h>

using namespace ::nl::Weave::Profiles::DataManagement_Current;
using namespace ::nl::Weave::TLV;
using namespace ::nl::Weave::DeviceLayer;
using namespace ::Schema::Weave::Trait::Description;

DeviceIdentityTraitDataSource::DeviceIdentityTraitDataSource(void) : TraitDataSource(&DeviceIdentityTrait::TraitSchema) { }

WEAVE_ERROR DeviceIdentityTraitDataSource::GetLeafData(PropertyPathHandle aLeafHandle, uint64_t aTagToWrite, TLVWriter & aWriter)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    switch (aLeafHandle)
    {
        case DeviceIdentityTrait::kPropertyHandle_VendorId:
        {
            uint16_t vendorId;
            err = ConfigurationMgr().GetVendorId(vendorId);
            SuccessOrExit(err);
            err = aWriter.Put(aTagToWrite, vendorId);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_VendorProductId:
        {
            uint16_t productId;
            err = ConfigurationMgr().GetProductId(productId);
            SuccessOrExit(err);
            err = aWriter.Put(aTagToWrite, productId);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_ProductRevision:
        {
            uint16_t productRev;
            err = ConfigurationMgr().GetProductRevision(productRev);
            SuccessOrExit(err);
            err = aWriter.Put(aTagToWrite, productRev);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_SerialNumber:
        {
            char serialNum[ConfigurationManager::kMaxSerialNumberLength + 1];
            size_t serialNumLen;
            err = ConfigurationMgr().GetSerialNumber(serialNum, sizeof(serialNum), serialNumLen);
            VerifyOrExit(err != WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND, err = WEAVE_NO_ERROR);
            SuccessOrExit(err);
            err = aWriter.PutString(aTagToWrite, serialNum, (uint32_t) serialNumLen);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_SoftwareVersion:
        {
            char firmwareRev[ConfigurationManager::kMaxFirmwareRevisionLength + 1];
            size_t firmwareRevLen;
            err = ConfigurationMgr().GetFirmwareRevision(firmwareRev, sizeof(firmwareRev), firmwareRevLen);
            VerifyOrExit(err != WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND, err = WEAVE_NO_ERROR);
            SuccessOrExit(err);
            err = aWriter.PutString(aTagToWrite, firmwareRev, (uint32_t) firmwareRevLen);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_ManufacturingDate:
        {
            enum
            {
                kDateStrLen = 10 // YYYY-MM-DD
            };
            char mfgDateStr[kDateStrLen + 1];
            uint16_t year;
            uint8_t month, dayOfMonth;
            err = ConfigurationMgr().GetManufacturingDate(year, month, dayOfMonth);
            VerifyOrExit(err != WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND, err = WEAVE_NO_ERROR);
            SuccessOrExit(err);
            snprintf(mfgDateStr, sizeof(mfgDateStr), "%04" PRIu16 "-%02" PRIu8 "-%02" PRIu8, year, month, dayOfMonth);
            err = aWriter.PutString(aTagToWrite, mfgDateStr, kDateStrLen);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_DeviceId:
        {
            err = aWriter.Put(aTagToWrite, ::nl::Weave::DeviceLayer::FabricState.LocalNodeId);
            SuccessOrExit(err);
            break;
        }

        case DeviceIdentityTrait::kPropertyHandle_FabricId:
        {
            if (ConfigurationMgr().IsMemberOfFabric())
            {
                err = aWriter.Put(aTagToWrite, ::nl::Weave::DeviceLayer::FabricState.FabricId);
                SuccessOrExit(err);
            }
            break;
        }

        default:
            break;
    }

exit:
    return err;
}
