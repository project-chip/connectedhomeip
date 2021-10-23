/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "CHIPPersistentStorageDelegateBridge.h"

#include <algorithm>
#include <lib/support/Base64.h>
#include <memory>
#include <string>

namespace {

std::string StringToBase64(const std::string & value)
{
    std::unique_ptr<char[]> buffer(new char[BASE64_ENCODED_LEN(value.length())]);

    uint32_t len = chip::Base64Encode32(
        reinterpret_cast<const uint8_t *>(value.data()), static_cast<uint32_t>(value.length()), buffer.get());
    if (len == UINT32_MAX) {
        return "";
    }

    return std::string(buffer.get(), len);
}

std::string Base64ToString(const std::string & b64Value)
{
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[BASE64_MAX_DECODED_LEN(b64Value.length())]);

    uint32_t len = chip::Base64Decode32(b64Value.data(), static_cast<uint32_t>(b64Value.length()), buffer.get());
    if (len == UINT32_MAX) {
        return "";
    }

    return std::string(reinterpret_cast<const char *>(buffer.get()), len);
}

} // namespace

CHIPPersistentStorageDelegateBridge::CHIPPersistentStorageDelegateBridge(void)
    : mDelegate(nil)
{
    mDefaultPersistentStorage = [[NSUserDefaults alloc] init];
    mWorkQueue = dispatch_queue_create("com.zigbee.chip.framework.storage.workqueue", DISPATCH_QUEUE_SERIAL);
}

CHIPPersistentStorageDelegateBridge::~CHIPPersistentStorageDelegateBridge(void) {}

void CHIPPersistentStorageDelegateBridge::setFrameworkDelegate(_Nullable id<CHIPPersistentStorageDelegate> delegate)
{
    dispatch_async(mWorkQueue, ^{
        if (delegate) {
            mDelegate = delegate;
        } else {
            mDelegate = nil;
        }
    });
}

CHIP_ERROR CHIPPersistentStorageDelegateBridge::SyncGetKeyValue(const char * key, void * buffer, uint16_t & size)
{
    __block CHIP_ERROR error = CHIP_NO_ERROR;
    NSString * keyString = [NSString stringWithUTF8String:key];

    dispatch_sync(mWorkQueue, ^{
        NSLog(@"PersistentStorageDelegate Sync Get Value for Key: %@", keyString);

        NSString * valueString = nil;

        id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
        if (strongDelegate) {
            valueString = [strongDelegate CHIPGetKeyValue:keyString];
        } else {
            valueString = [mDefaultPersistentStorage objectForKey:keyString];
        }

        if (valueString != nil) {
            std::string decoded = Base64ToString([valueString UTF8String]);

            if (decoded.length() > UINT16_MAX) {
                error = CHIP_ERROR_BUFFER_TOO_SMALL;
            } else {
                if (buffer != nullptr) {
                    memcpy(buffer, decoded.data(), std::min<size_t>(decoded.length(), size));
                    if (size < decoded.length()) {
                        error = CHIP_ERROR_NO_MEMORY;
                    }
                } else {
                    error = CHIP_ERROR_NO_MEMORY;
                }
                size = static_cast<uint16_t>(decoded.length());
            }
        } else {
            error = CHIP_ERROR_KEY_NOT_FOUND;
        }
    });
    return error;
}

CHIP_ERROR CHIPPersistentStorageDelegateBridge::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    std::string base64Value = StringToBase64(std::string(static_cast<const char *>(value), size));

    NSString * keyString = [NSString stringWithUTF8String:key];
    NSString * valueString = [NSString stringWithUTF8String:base64Value.c_str()];

    dispatch_sync(mWorkQueue, ^{
        NSLog(@"PersistentStorageDelegate Set Key %@", keyString);

        id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
        if (strongDelegate) {
            [strongDelegate CHIPSetKeyValue:keyString value:valueString];
        } else {
            [mDefaultPersistentStorage setObject:valueString forKey:keyString];
        }
    });

    // TODO: ideally the error from the dispatch should be returned
    // however we expect to replace the storage delegate with KVS so for now
    // we return no error (return used to be void due to async dispatch anyway)

    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPPersistentStorageDelegateBridge::SyncDeleteKeyValue(const char * key)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    dispatch_sync(mWorkQueue, ^{
        NSLog(@"PersistentStorageDelegate Delete Key: %@", keyString);

        id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
        if (strongDelegate) {
            [strongDelegate CHIPDeleteKeyValue:keyString];
        } else {
            [mDefaultPersistentStorage removeObjectForKey:keyString];
        }
    });

    // TODO: ideally the error from the dispatch should be returned
    // however we expect to replace the storage delegate with KVS so for now
    // we return no error (return used to be void due to async dispatch anyway)

    return CHIP_NO_ERROR;
}
