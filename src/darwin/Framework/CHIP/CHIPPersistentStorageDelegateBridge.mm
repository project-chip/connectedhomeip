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
#import "CHIPError.h"
#import <Foundation/Foundation.h>

CHIPPersistentStorageDelegateBridge::CHIPPersistentStorageDelegateBridge(void)
    : mDelegate(nil)
{
    mDefaultPersistentStorage = [[NSUserDefaults alloc] init];
    mWorkQueue = dispatch_queue_create("com.zigbee.chip.framework.storage.workqueue", DISPATCH_QUEUE_SERIAL);
}

CHIPPersistentStorageDelegateBridge::~CHIPPersistentStorageDelegateBridge(void) {}

void CHIPPersistentStorageDelegateBridge::setFrameworkDelegate(id<CHIPPersistentStorageDelegate> delegate, dispatch_queue_t queue)
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

void CHIPPersistentStorageDelegateBridge::SetDelegate(chip::Controller::PersistentStorageResultDelegate * delegate)
{
    dispatch_async(mWorkQueue, ^{
        if (delegate) {
            mCallback = delegate;

            mCompletionHandler = ^(NSString * key, NSString * value) {
                chip::Controller::PersistentStorageResultDelegate * callback = mCallback;
                if (callback) {
                    dispatch_async(mWorkQueue, ^{
                        callback->OnValue([key UTF8String], [value UTF8String]);
                    });
                }
            };

            mSetStatusHandler = ^(NSString * key, NSError * status) {
                chip::Controller::PersistentStorageResultDelegate * callback = mCallback;
                if (callback) {
                    dispatch_async(mWorkQueue, ^{
                        callback->OnStatus([key UTF8String], chip::Controller::PersistentStorageResultDelegate::Operation::kSET,
                            [CHIPError errorToCHIPErrorCode:status]);
                    });
                }
            };

            mDeleteStatusHandler = ^(NSString * key, NSError * status) {
                chip::Controller::PersistentStorageResultDelegate * callback = mCallback;
                if (callback) {
                    dispatch_async(mWorkQueue, ^{
                        callback->OnStatus([key UTF8String], chip::Controller::PersistentStorageResultDelegate::Operation::kDELETE,
                            [CHIPError errorToCHIPErrorCode:status]);
                    });
                }
            };
        } else {
            mCallback = nil;
            mCompletionHandler = nil;
            mSetStatusHandler = nil;
            mDeleteStatusHandler = nil;
        }
    });
}

void CHIPPersistentStorageDelegateBridge::GetKeyValue(const char * key)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    dispatch_async(mWorkQueue, ^{
        NSLog(@"PersistentStorageDelegate Get Value for Key: %@", keyString);

        id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                [strongDelegate CHIPGetKeyValue:keyString handler:mCompletionHandler];
            });
        } else {
            NSString * value = [mDefaultPersistentStorage objectForKey:keyString];
            NSLog(@"PersistentStorageDelegate Get Value for Key: %@, value %@", keyString, value);
            mCompletionHandler(keyString, value);
        }
    });
}

CHIP_ERROR CHIPPersistentStorageDelegateBridge::GetKeyValue(const char * key, char * value, uint16_t & size)
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
            if (value != nullptr) {
                size = strlcpy(value, [valueString UTF8String], size);
            } else {
                size = [valueString length];
            }
            // Increment size to account for null termination
            size += 1;
        } else {
            error = CHIP_ERROR_INVALID_ARGUMENT;
        }
    });
    return error;
}

void CHIPPersistentStorageDelegateBridge::SetKeyValue(const char * key, const char * value)
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
            mSetStatusHandler(keyString, [CHIPError errorForCHIPErrorCode:0]);
        }
    });
}

void CHIPPersistentStorageDelegateBridge::DeleteKeyValue(const char * key)
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
            mDeleteStatusHandler(keyString, [CHIPError errorForCHIPErrorCode:0]);
        }
    });
}
