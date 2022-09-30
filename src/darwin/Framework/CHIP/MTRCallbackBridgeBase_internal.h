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

#import "MTRBaseDevice_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRError_Internal.h"
#import "zap-generated/MTRBaseClusters.h"

#include <app/data-model/NullObject.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/SessionHandle.h>

/**
 * Bridge that allows invoking a given MTRActionBlock on the Matter queue, after
 * communication with the device in question has been established, as far as we
 * know.
 */

// TODO: ADD NS_ASSUME_NONNULL_BEGIN to this header.  When that happens, note
// that in MTRActionBlock the two callback pointers are nonnull and the two
// arguments of ResponseHandler are both nullable.

typedef void (^ResponseHandler)(id value, NSError * error);
typedef CHIP_ERROR (^MTRActionBlock)(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
    chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure);
typedef CHIP_ERROR (^MTRLocalActionBlock)(chip::Callback::Cancelable * success, chip::Callback::Cancelable * failure);
typedef void (*DefaultFailureCallbackType)(void *, CHIP_ERROR);

template <class T> class MTRCallbackBridge {
public:
    /**
     * Run the given MTRLocalActionBlock on the Matter thread, then handle
     * converting the value produced by the success callback to the right type
     * so it can be passed to a callback of the type we're templated over.
     *
     * Does not attempt to establish any sessions to devices.  Must not be used
     * with any action blocks that need a session.
     */
    MTRCallbackBridge(dispatch_queue_t queue, ResponseHandler handler, MTRLocalActionBlock action, T OnSuccessFn, bool keepAlive)
        : mQueue(queue)
        , mHandler(handler)
        , mKeepAlive(keepAlive)
        , mSuccess(OnSuccessFn, this)
        , mFailure(OnFailureFn, this)
    {
        LogRequestStart();

        // For now keep sync dispatch here.
        dispatch_sync(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
            CHIP_ERROR err = action(mSuccess.Cancel(), mFailure.Cancel());
            if (err != CHIP_NO_ERROR) {
                NSLog(@"Failure performing action. C++-mangled success callback type: '%s', error: %s", typeid(T).name(),
                    chip::ErrorStr(err));

                // Take the normal async error-reporting codepath.  This will also
                // handle cleaning us up properly.
                OnFailureFn(this, err);
            }
        });
    }

    /**
     * Run the given MTRActionBlock on the Matter thread, after getting a CASE
     * session (possibly pre-existing) to the given node ID on the fabric
     * represented by the given MTRDeviceController.  On success, convert the
     * success value to whatever type it needs to be to call the callback type
     * we're templated over.
     */
    MTRCallbackBridge(dispatch_queue_t queue, chip::NodeId nodeID, MTRDeviceController * controller, ResponseHandler handler,
        MTRActionBlock action, T OnSuccessFn, bool keepAlive)
        : mQueue(queue)
        , mHandler(handler)
        , mAction(action)
        , mKeepAlive(keepAlive)
        , mSuccess(OnSuccessFn, this)
        , mFailure(OnFailureFn, this)
    {
        ActionWithNodeID(nodeID, controller);
    }

    /**
     * Run the given MTRActionBlock on the Matter thread after getting a secure
     * session corresponding to the given MTRBaseDevice.  On success, convert
     * the success value to whatever type it needs to be to call the callback
     * type we're templated over.
     */
    MTRCallbackBridge(dispatch_queue_t queue, MTRBaseDevice * device, ResponseHandler handler, MTRActionBlock action, T OnSuccessFn,
        bool keepAlive)
        : mQueue(queue)
        , mHandler(handler)
        , mAction(action)
        , mKeepAlive(keepAlive)
        , mSuccess(OnSuccessFn, this)
        , mFailure(OnFailureFn, this)
    {
        if (device.isPASEDevice) {
            ActionWithPASEDevice(device);
        } else {
            ActionWithNodeID(device.nodeID, device.deviceController);
        }
    };

    void ActionWithPASEDevice(MTRBaseDevice * device)
    {
        LogRequestStart();

        BOOL ok = [device.deviceController
            getSessionForCommissioneeDevice:device.nodeID
                                 completion:^(chip::Messaging::ExchangeManager * exchangeManager,
                                     const chip::Optional<chip::SessionHandle> & session, NSError * error) {
                                     MaybeDoAction(exchangeManager, session, error);
                                 }];

        if (ok == NO) {
            OnFailureFn(this, CHIP_ERROR_INCORRECT_STATE);
        }
    }

    void ActionWithNodeID(chip::NodeId nodeID, MTRDeviceController * controller)
    {
        LogRequestStart();

        BOOL ok = [controller getSessionForNode:nodeID
                                     completion:^(chip::Messaging::ExchangeManager * exchangeManager,
                                         const chip::Optional<chip::SessionHandle> & session, NSError * error) {
                                         MaybeDoAction(exchangeManager, session, error);
                                     }];

        if (ok == NO) {
            OnFailureFn(this, CHIP_ERROR_INCORRECT_STATE);
        }
    }

    void LogRequestStart()
    {
        mRequestTime = [NSDate date];
        // Generate a unique cookie to track this operation
        mCookie = [NSString stringWithFormat:@"Response Time: %s+%u", typeid(T).name(), arc4random()];
        ChipLogDetail(Controller, "%s", mCookie.UTF8String);
    }

    void MaybeDoAction(
        chip::Messaging::ExchangeManager * exchangeManager, const chip::Optional<chip::SessionHandle> & session, NSError * error)
    {
        // Make sure we don't hold on to our action longer than we have to.
        auto action = mAction;
        mAction = nil;
        if (error != nil) {
            DispatchFailure(this, error);
            return;
        }

        CHIP_ERROR err = action(*exchangeManager, session.Value(), mSuccess.Cancel(), mFailure.Cancel());
        if (err != CHIP_NO_ERROR) {
            NSLog(@"Failure performing action. C++-mangled success callback type: '%s', error: %s", typeid(T).name(),
                chip::ErrorStr(err));

            // Take the normal async error-reporting codepath.  This will also
            // handle cleaning us up properly.
            OnFailureFn(this, err);
        }
    }

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
    MTRActionBlock mAction;
    bool mKeepAlive;

    chip::Callback::Callback<T> mSuccess;
    chip::Callback::Callback<DefaultFailureCallbackType> mFailure;

    // Measure the time it took for the callback to trigger
    NSDate * mRequestTime;
    NSString * mCookie;
};
