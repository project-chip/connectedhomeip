/*
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

#import <Foundation/Foundation.h>

#import "MTRError_Internal.h"
#import "zap-generated/MTRClustersObjc.h"

#include <app/data-model/NullObject.h>
#include <platform/CHIPDeviceLayer.h>

typedef CHIP_ERROR (^MTRActionBlock)(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure);
typedef void (*DefaultFailureCallbackType)(void *, CHIP_ERROR);

template <class T> class MTRCallbackBridge {
public:
    MTRCallbackBridge(dispatch_queue_t queue, ResponseHandler handler, MTRActionBlock action, T OnSuccessFn, bool keepAlive)
        : mQueue(queue)
        , mHandler(handler)
        , mKeepAlive(keepAlive)
        , mSuccess(OnSuccessFn, this)
        , mFailure(OnFailureFn, this)
    {
        mRequestTime = [NSDate date];
        // Generate a unique cookie to track this operation
        mCookie = [NSString stringWithFormat:@"Response Time: %s+%u", typeid(T).name(), arc4random()];
        ChipLogDetail(Controller, "%s", mCookie.UTF8String);
        __block CHIP_ERROR err = CHIP_NO_ERROR;
        dispatch_sync(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
            err = action(mSuccess.Cancel(), mFailure.Cancel());
        });

        if (CHIP_NO_ERROR != err) {
            NSLog(@"Failure performing action. C++-mangled success callback type: '%s', error: %s", typeid(T).name(),
                chip::ErrorStr(err));

            // Take the normal async error-reporting codepath.  This will also
            // handle cleaning us up properly.
            DispatchFailure(this, [MTRError errorForCHIPErrorCode:err]);
        }
    };

    virtual ~MTRCallbackBridge() {};

    static void OnFailureFn(void * context, CHIP_ERROR error) { DispatchFailure(context, [MTRError errorForCHIPErrorCode:error]); }

    static void DispatchSuccess(void * context, id value) { DispatchCallbackResult(context, nil, value); }

    static void DispatchFailure(void * context, NSError * error) { DispatchCallbackResult(context, error, nil); }

protected:
    dispatch_queue_t mQueue;

private:
    static void DispatchCallbackResult(void * context, NSError * error, id value)
    {
        MTRCallbackBridge * callbackBridge = static_cast<MTRCallbackBridge *>(context);
        if (!callbackBridge) {
            return;
        }

        if (!callbackBridge->mQueue) {
            delete callbackBridge;
            return;
        }

        if (error) {
            // We should delete ourselves; there will be no more callbacks.
            callbackBridge->mKeepAlive = false;
        }

        dispatch_async(callbackBridge->mQueue, ^{
            ChipLogDetail(Controller, "%s %f seconds", callbackBridge->mCookie.UTF8String,
                -[callbackBridge->mRequestTime timeIntervalSinceNow]);
            callbackBridge->mHandler(value, error);

            if (!callbackBridge->mKeepAlive) {
                delete callbackBridge;
            }
        });
    }

    ResponseHandler mHandler;
    bool mKeepAlive;

    chip::Callback::Callback<T> mSuccess;
    chip::Callback::Callback<DefaultFailureCallbackType> mFailure;

    // Measure the time it took for the callback to trigger
    NSDate * mRequestTime;
    NSString * mCookie;
};
