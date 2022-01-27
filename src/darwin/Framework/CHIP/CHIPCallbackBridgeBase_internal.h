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

#import "CHIPError_Internal.h"
#import "zap-generated/CHIPClientCallbacks.h"
#import "zap-generated/CHIPClustersObjc.h"

#include <app/data-model/NullObject.h>
#include <platform/CHIPDeviceLayer.h>

typedef CHIP_ERROR (^CHIPActionBlock)(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure);
typedef void (*CHIPDefaultFailureCallbackType)(void *, CHIP_ERROR);

template <class T> class CHIPCallbackBridge {
public:
    CHIPCallbackBridge(dispatch_queue_t queue, ResponseHandler handler, CHIPActionBlock action, T OnSuccessFn, bool keepAlive)
        : mQueue(queue)
        , mHandler(handler)
        , mKeepAlive(keepAlive)
        , mSuccess(OnSuccessFn, this)
        , mFailure(OnFailureFn, this)
    {
        __block CHIP_ERROR err = CHIP_NO_ERROR;
        dispatch_sync(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
            err = action(mSuccess.Cancel(), mFailure.Cancel());
        });

        if (CHIP_NO_ERROR != err) {
            NSLog(@"Failure performing action. C++-mangled success callback type: '%s', error: %s", typeid(T).name(),
                chip::ErrorStr(err));

            // Take the normal async error-reporting codepath.  This will also
            // handle cleaning us up properly.
            DispatchFailure(this, [CHIPError errorForCHIPErrorCode:err]);
        }
    };

    virtual ~CHIPCallbackBridge() {};

    static void OnFailureFn(void * context, CHIP_ERROR error) { DispatchFailure(context, [CHIPError errorForCHIPErrorCode:error]); }

    static void DispatchSuccess(void * context, id value) { DispatchCallbackResult(context, nil, value); }

    static void DispatchFailure(void * context, NSError * error) { DispatchCallbackResult(context, error, nil); }

protected:
    dispatch_queue_t mQueue;

private:
    static void DispatchCallbackResult(void * context, NSError * error, id value)
    {
        CHIPCallbackBridge * callbackBridge = static_cast<CHIPCallbackBridge *>(context);
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
            callbackBridge->mHandler(value, error);

            if (!callbackBridge->mKeepAlive) {
                delete callbackBridge;
            }
        });
    }

    ResponseHandler mHandler;
    bool mKeepAlive;

    chip::Callback::Callback<T> mSuccess;
    chip::Callback::Callback<CHIPDefaultFailureCallbackType> mFailure;
};
