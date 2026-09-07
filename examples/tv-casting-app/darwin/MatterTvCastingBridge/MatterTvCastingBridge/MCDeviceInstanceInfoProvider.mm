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

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetVendorName(char * buf, size_t bufSize)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(vendorName)]) {
                NSString * value = [delegate vendorName];
                if (value != nil) {
                    chip::Platform::CopyString(buf, bufSize, [value UTF8String]);
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetVendorName(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetVendorId(uint16_t & vendorId)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(vendorId)]) {
                NSNumber * value = [delegate vendorId];
                if (value != nil) {
                    vendorId = [value unsignedShortValue];
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetVendorId(vendorId);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetProductName(char * buf, size_t bufSize)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(productName)]) {
                NSString * value = [delegate productName];
                if (value != nil) {
                    chip::Platform::CopyString(buf, bufSize, [value UTF8String]);
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetProductName(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetProductId(uint16_t & productId)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(productId)]) {
                NSNumber * value = [delegate productId];
                if (value != nil) {
                    productId = [value unsignedShortValue];
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetProductId(productId);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetSerialNumber(char * buf, size_t bufSize)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(serialNumber)]) {
                NSString * value = [delegate serialNumber];
                if (value != nil) {
                    chip::Platform::CopyString(buf, bufSize, [value UTF8String]);
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetSerialNumber(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetHardwareVersion(uint16_t & hardwareVersion)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(hardwareVersion)]) {
                NSNumber * value = [delegate hardwareVersion];
                if (value != nil) {
                    hardwareVersion = [value unsignedShortValue];
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetHardwareVersion(hardwareVersion);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetHardwareVersionString(char * buf, size_t bufSize)
        {
            id<MCDeviceInstanceInfoProvider> delegate = mDelegate;
            if (delegate != nil && [delegate respondsToSelector:@selector(hardwareVersionString)]) {
                NSString * value = [delegate hardwareVersionString];
                if (value != nil) {
                    chip::Platform::CopyString(buf, bufSize, [value UTF8String]);
                    return CHIP_NO_ERROR;
                }
            }
            return mDefaultProvider.GetHardwareVersionString(buf, bufSize);
        }

        // Delegated methods — always forward to default provider

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetPartNumber(char * buf, size_t bufSize)
        {
            return mDefaultProvider.GetPartNumber(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetProductURL(char * buf, size_t bufSize)
        {
            return mDefaultProvider.GetProductURL(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetProductLabel(char * buf, size_t bufSize)
        {
            return mDefaultProvider.GetProductLabel(buf, bufSize);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
        {
            return mDefaultProvider.GetManufacturingDate(year, month, day);
        }

        CHIP_ERROR MCDeviceInstanceInfoProviderBridge::GetRotatingDeviceIdUniqueId(chip::MutableByteSpan & uniqueIdSpan)
        {
            return mDefaultProvider.GetRotatingDeviceIdUniqueId(uniqueIdSpan);
        }

    }; // namespace support
}; // namespace casting
}; // namespace matter
