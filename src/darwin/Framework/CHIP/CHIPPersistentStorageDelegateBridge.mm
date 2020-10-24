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
}

CHIPPersistentStorageDelegateBridge::~CHIPPersistentStorageDelegateBridge(void) {}

void CHIPPersistentStorageDelegateBridge::setFrameworkDelegate(id<CHIPPersistentStorageDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mDelegate = nil;
        mQueue = nil;
    }
}

void CHIPPersistentStorageDelegateBridge::SetDelegate(chip::DeviceController::PersistentStorageResultDelegate * delegate)
{
    if (delegate) {
        mCallback = delegate;

        mCompletionHandler = ^(NSString * key, NSString * value) {
            mCallback->OnValue([key UTF8String], [value UTF8String]);
        };

        mStatusHandler = ^(NSString * key, Operation operation, NSError * status) {
            chip::DeviceController::PersistentStorageResultDelegate::Operation op
                = chip::DeviceController::PersistentStorageResultDelegate::Operation::kGET;
            switch (operation) {
            case kGet:
                op = chip::DeviceController::PersistentStorageResultDelegate::Operation::kGET;
                break;
            case kSet:
                op = chip::DeviceController::PersistentStorageResultDelegate::Operation::kSET;
                break;
            case kDelete:
                op = chip::DeviceController::PersistentStorageResultDelegate::Operation::kDELETE;
                break;
            }
            mCallback->OnStatus([key UTF8String], op, [CHIPError errorToCHIPErrorCode:status]);
        };
    } else {
        mCallback = nil;
        mCompletionHandler = nil;
        mStatusHandler = nil;
    }
}

void CHIPPersistentStorageDelegateBridge::GetKeyValue(const char * key)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    NSLog(@"PersistentStorageDelegate Get Value for Key: %@", keyString);

    id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        dispatch_async(mQueue, ^{
            [strongDelegate GetKeyValue:keyString handler:mCompletionHandler];
        });
    }
}

void CHIPPersistentStorageDelegateBridge::SetKeyValue(const char * key, const char * value)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    NSString * valueString = [NSString stringWithUTF8String:value];
    NSLog(@"PersistentStorageDelegate Set Key %@", keyString);

    id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        dispatch_async(mQueue, ^{
            [strongDelegate SetKeyValue:keyString value:valueString handler:mStatusHandler];
        });
    }
}

void CHIPPersistentStorageDelegateBridge::DeleteKeyValue(const char * key)
{
    NSString * keyString = [NSString stringWithUTF8String:key];
    NSLog(@"PersistentStorageDelegate Delete Key: %@", keyString);

    id<CHIPPersistentStorageDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        dispatch_async(mQueue, ^{
            [strongDelegate DeleteKeyValue:keyString handler:mStatusHandler];
        });
    }
}
