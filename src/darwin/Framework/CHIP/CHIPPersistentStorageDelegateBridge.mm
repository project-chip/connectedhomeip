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

CHIPPersistentStorageDelegateBridge::CHIPPersistentStorageDelegateBridge(void)
    : mDelegate(nil)
{
    mDefaultPersistentStorage = [[NSUserDefaults alloc] init];
    mWorkQueue = dispatch_queue_create("com.zigbee.chip.framework.storage.workqueue", DISPATCH_QUEUE_SERIAL);
}

CHIPPersistentStorageDelegateBridge::~CHIPPersistentStorageDelegateBridge(void) {}

void CHIPPersistentStorageDelegateBridge::setFrameworkDelegate(
    _Nullable id<CHIPPersistentStorageDelegate> delegate, _Nullable dispatch_queue_t queue)
{
    dispatch_async(mWorkQueue, ^{
        if (delegate && queue) {
            mDelegate = delegate;
            mQueue = queue;
        } else {
            mDelegate = nil;
            mQueue = nil;
        }
    });
}

void CHIPPersistentStorageDelegateBridge::SetStorageDelegate(chip::PersistentStorageResultDelegate * delegate)
{
    dispatch_async(mWorkQueue, ^{
        if (delegate) {
            mCallback = delegate;

            mSetStatusHandler = ^(NSString * key, NSError * status) {
                chip::PersistentStorageResultDelegate * callback = mCallback;
                if (callback) {
                    dispatch_async(mWorkQueue, ^{
                        callback->OnPersistentStorageStatus([key UTF8String],
                            chip::PersistentStorageResultDelegate::Operation::kSET, [CHIPError errorToCHIPErrorCode:status]);
                    });
                }
            };

            mDeleteStatusHandler = ^(NSString * key, NSError * status) {
                chip::PersistentStorageResultDelegate * callback = mCallback;
                if (callback) {
                    dispatch_async(mWorkQueue, ^{
                        callback->OnPersistentStorageStatus([key UTF8String],
                            chip::PersistentStorageResultDelegate::Operation::kDELETE, [CHIPError errorToCHIPErrorCode:status]);
                    });
                }
            };
        } else {
            mCallback = nil;
            mSetStatusHandler = nil;
            mDeleteStatusHandler = nil;
        }
    });
}

CHIP_ERROR CHIPPersistentStorageDelegateBridge::SyncGetKeyValue(const char * key, char * value, uint16_t & size)
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
            if (([valueString lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1) > UINT16_MAX) {
                error = CHIP_ERROR_BUFFER_TOO_SMALL;
            } else {
                if (value != nullptr) {
                    size = (uint16_t) strlcpy(value, [valueString UTF8String], size);
                    if (size < [valueString lengthOfBytesUsingEncoding:NSUTF8StringEncoding]) {
                        error = CHIP_ERROR_NO_MEMORY;
                    }
                } else {
                    size = (uint16_t) [valueString lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
                    error = CHIP_ERROR_NO_MEMORY;
                }
                // Increment size to account for null termination
                size += 1;
            }
        } else {
            error = CHIP_ERROR_KEY_NOT_FOUND;
        }
    });
    return error;
}

void CHIPPersistentStorageDelegateBridge::AsyncSetKeyValue(const char * key, const char * value)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    NSString * valueString = [NSString stringWithUTF8String:value];
    dispatch_async(mWorkQueue, ^{
        NSLog(@"PersistentStorageDelegate Set Key %@, Value %@", keyString, valueString);

        id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                [strongDelegate CHIPSetKeyValue:keyString value:valueString handler:mSetStatusHandler];
            });
        } else {
            [mDefaultPersistentStorage setObject:valueString forKey:keyString];
            if (mSetStatusHandler) {
                mSetStatusHandler(keyString, [CHIPError errorForCHIPErrorCode:0]);
            }
        }
    });
}

void CHIPPersistentStorageDelegateBridge::AsyncDeleteKeyValue(const char * key)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    dispatch_async(mWorkQueue, ^{
        NSLog(@"PersistentStorageDelegate Delete Key: %@", keyString);

        id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                [strongDelegate CHIPDeleteKeyValue:keyString handler:mDeleteStatusHandler];
            });
        } else {
            [mDefaultPersistentStorage removeObjectForKey:keyString];
            if (mDeleteStatusHandler) {
                mDeleteStatusHandler(keyString, [CHIPError errorForCHIPErrorCode:0]);
            }
        }
    });
}
