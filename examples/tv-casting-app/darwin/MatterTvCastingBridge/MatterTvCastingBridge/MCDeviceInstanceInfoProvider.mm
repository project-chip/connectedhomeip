/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MCDeviceInstanceInfoProvider.h"

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#import <Foundation/Foundation.h>

namespace matter {
namespace casting {
    namespace support {

        CHIP_ERROR MCDeviceInstanceInfoProvider::Initialize(MCDeviceInstanceInfo * _Nonnull deviceInstanceInfo)
        {
            VerifyOrReturnError(deviceInstanceInfo != nil, CHIP_ERROR_INVALID_ARGUMENT);

            if (deviceInstanceInfo.vendorName != nil) {
                chip::Platform::CopyString(mVendorName, [deviceInstanceInfo.vendorName UTF8String]);
                mHasVendorName = true;
            }

            if (deviceInstanceInfo.productName != nil) {
                chip::Platform::CopyString(mProductName, [deviceInstanceInfo.productName UTF8String]);
                mHasProductName = true;
            }

            if (deviceInstanceInfo.serialNumber != nil) {
                chip::Platform::CopyString(mSerialNumber, [deviceInstanceInfo.serialNumber UTF8String]);
                mHasSerialNumber = true;
            }

            if (deviceInstanceInfo.hardwareVersionString != nil) {
                chip::Platform::CopyString(mHardwareVersionString, [deviceInstanceInfo.hardwareVersionString UTF8String]);
                mHasHardwareVersionString = true;
            }

            if (deviceInstanceInfo.vendorId != nil) {
                mVendorId.SetValue([deviceInstanceInfo.vendorId unsignedShortValue]);
            }

            if (deviceInstanceInfo.productId != nil) {
                mProductId.SetValue([deviceInstanceInfo.productId unsignedShortValue]);
            }

            if (deviceInstanceInfo.hardwareVersion != nil) {
                mHardwareVersion.SetValue([deviceInstanceInfo.hardwareVersion unsignedShortValue]);
            }

            return CHIP_NO_ERROR;
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetVendorName(char * buf, size_t bufSize)
        {
            if (mHasVendorName) {
                VerifyOrReturnError(bufSize >= strlen(mVendorName) + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
                chip::Platform::CopyString(buf, bufSize, mVendorName);
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetVendorName(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetProductName(char * buf, size_t bufSize)
        {
            if (mHasProductName) {
                VerifyOrReturnError(bufSize >= strlen(mProductName) + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
                chip::Platform::CopyString(buf, bufSize, mProductName);
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetProductName(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetVendorId(uint16_t & vendorId)
        {
            if (mVendorId.HasValue()) {
                vendorId = mVendorId.Value();
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetVendorId(vendorId);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetProductId(uint16_t & productId)
        {
            if (mProductId.HasValue()) {
                productId = mProductId.Value();
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetProductId(productId);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetSerialNumber(char * buf, size_t bufSize)
        {
            if (mHasSerialNumber) {
                VerifyOrReturnError(bufSize >= strlen(mSerialNumber) + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
                chip::Platform::CopyString(buf, bufSize, mSerialNumber);
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetSerialNumber(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetHardwareVersion(uint16_t & hardwareVersion)
        {
            if (mHardwareVersion.HasValue()) {
                hardwareVersion = mHardwareVersion.Value();
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetHardwareVersion(hardwareVersion);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetHardwareVersionString(char * buf, size_t bufSize)
        {
            if (mHasHardwareVersionString) {
                VerifyOrReturnError(bufSize >= strlen(mHardwareVersionString) + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
                chip::Platform::CopyString(buf, bufSize, mHardwareVersionString);
                return CHIP_NO_ERROR;
            }
            return mDefaultProvider.GetHardwareVersionString(buf, bufSize);
        }

        // Delegated methods — always forward to default provider

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetPartNumber(char * buf, size_t bufSize)
        {
            return mDefaultProvider.GetPartNumber(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetProductURL(char * buf, size_t bufSize)
        {
            return mDefaultProvider.GetProductURL(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetProductLabel(char * buf, size_t bufSize)
        {
            return mDefaultProvider.GetProductLabel(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
        {
            return mDefaultProvider.GetManufacturingDate(year, month, day);
        }

        CHIP_ERROR MCDeviceInstanceInfoProvider::GetRotatingDeviceIdUniqueId(chip::MutableByteSpan & uniqueIdSpan)
        {
            return mDefaultProvider.GetRotatingDeviceIdUniqueId(uniqueIdSpan);
        }

    }; // namespace support
}; // namespace casting
}; // namespace matter
