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
#include <transport/Session.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Bridge that allows invoking a given MTRActionBlock on the Matter queue, after
 * communication with the device in question has been established, as far as we
 * know.
 */
class MTRCallbackBridgeBase {
public:
    /**
     * Run the given MTRActionBlock on the Matter thread, after getting a CASE
     * session (possibly pre-existing) to the given node ID on the fabric
     * represented by the given MTRDeviceController.  On success, convert the
     * success value to whatever type it needs to be to call the callback type
     * we're templated over.  Once this function has been called, on a callback
     * bridge allocated with `new`, the bridge object must not be accessed by
     * the caller.  The action block will handle deleting the bridge.
     */
    void DispatchAction(chip::NodeId nodeID, MTRDeviceController * controller) && { ActionWithNodeID(nodeID, controller); }

    /**
     * Run the given MTRActionBlock on the Matter thread after getting a secure
     * session corresponding to the given MTRBaseDevice.  On success, convert
     * the success value to whatever type it needs to be to call the callback
     * type we're templated over.  Once this function has been called, on a callback
     * bridge allocated with `new`, the bridge object must not be accessed by
     * the caller.  The action block will handle deleting the bridge.
     */
    void DispatchAction(MTRBaseDevice * device) &&
    {
        if (device.isPASEDevice) {
            ActionWithPASEDevice(device);
        } else {
            ActionWithNodeID(device.nodeID, device.deviceController);
        }
    }

protected:
    MTRCallbackBridgeBase(dispatch_queue_t queue)
        : mQueue(queue)
    {
    }

    virtual ~MTRCallbackBridgeBase() {};

    virtual void MaybeDoAction(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
        const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error)
        = 0;
    virtual void LogRequestStart() = 0;

    void ActionWithPASEDevice(MTRBaseDevice * device)
    {
        LogRequestStart();

        [device.deviceController getSessionForCommissioneeDevice:device.nodeID
                                                      completion:^(chip::Messaging::ExchangeManager * exchangeManager,
                                                          const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error, NSNumber * _Nullable retryDelay) {
                                                          MaybeDoAction(exchangeManager, session, error);
                                                      }];
    }

    void ActionWithNodeID(chip::NodeId nodeID, MTRDeviceController * controller)
    {
        LogRequestStart();

        [controller getSessionForNode:nodeID
                           completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                               const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error, NSNumber * _Nullable retryDelay) {
                               MaybeDoAction(exchangeManager, session, error);
                           }];
    }

    // OnDone and KeepAliveOnCallback really only make sense for subscription
    // bridges, but we put them here to avoid many copies of this code in
    // generated bits.
    void OnDone()
    {
        if (!mQueue) {
            delete this;
            return;
        }

        // Delete ourselves async, so that any error/data reports we
        // queued up before getting OnDone have a chance to run.
        auto * self = this;
        dispatch_async(mQueue, ^{
            delete self;
        });
    }

    void KeepAliveOnCallback() { mKeepAlive = true; }

    dispatch_queue_t mQueue;
    bool mKeepAlive = false;
};

typedef void (^MTRResponseHandler)(id _Nullable value, NSError * _Nullable error);
typedef void (*MTRErrorCallback)(void * context, CHIP_ERROR error);

/**
 * The bridge will pass itself as the last argument to the action block.
 *
 * The action block must do one of three things:
 *
 * 1) Return an error.
 * 2) Call the "successCb" callback, with the bridge passed to the block as the
 *    context, possibly asynchronously.
 * 3) Call the "failureCb" callback, with the bridge passed to the block as the
 *    context, possibly asynchronously.
 *
 * For an MTRCallbackBridge that has keepAlive set to true, the success/failure
 * callbacks may be called multiple times.  If keepAlive is false, there must be
 * no calls after the first one.
 */
template <typename SuccessCallback>
using MTRActionBlockT = CHIP_ERROR (^)(chip::Messaging::ExchangeManager & exchangeManager, const chip::SessionHandle & session,
    SuccessCallback successCb, MTRErrorCallback failureCb, MTRCallbackBridgeBase * bridge);
template <typename SuccessCallback>
using MTRLocalActionBlockT = CHIP_ERROR (^)(SuccessCallback successCb, MTRErrorCallback failureCb);

template <class T>
class MTRCallbackBridge : public MTRCallbackBridgeBase {
public:
    using MTRActionBlock = MTRActionBlockT<T>;
    using MTRLocalActionBlock = MTRLocalActionBlockT<T>;
    using SuccessCallbackType = T;

    /**
     * Construct a callback bridge, which can then have DispatchAction() called
     * on it.
     */
    MTRCallbackBridge(dispatch_queue_t queue, MTRResponseHandler handler, MTRActionBlock _Nonnull action, T OnSuccessFn)
        : MTRCallbackBridgeBase(queue)
        , mHandler(handler)
        , mAction(action)
        , mSuccess(OnSuccessFn)
        , mFailure(OnFailureFn)
    {
    }

    void LogRequestStart() override
    {
        mRequestTime = [NSDate date];
        // Generate a unique cookie to track this operation
        mCookie = [NSString stringWithFormat:@"Response Time: %s+%u", typeid(T).name(), arc4random()];
        ChipLogDetail(Controller, "%s", mCookie.UTF8String);
    }

    void MaybeDoAction(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
        const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error) override
    {
        // Make sure we don't hold on to our action longer than we have to.
        auto action = mAction;
        mAction = nil;
        if (error != nil) {
            DispatchFailure(this, error);
            return;
        }

        CHIP_ERROR err = action(*exchangeManager, session.Value(), mSuccess, mFailure, this);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(Controller, "Failure performing action. C++-mangled success callback type: '%s', error: %s",
                typeid(T).name(), chip::ErrorStr(err));

            // Take the normal async error-reporting codepath.  This will also
            // handle cleaning us up properly.
            OnFailureFn(this, err);
        }
    }

    virtual ~MTRCallbackBridge() {};

protected:
    static void OnFailureFn(void * context, CHIP_ERROR error) { DispatchFailure(context, [MTRError errorForCHIPErrorCode:error]); }

    static void DispatchSuccess(void * context, id _Nullable value) { DispatchCallbackResult(context, nil, value); }

    static void DispatchFailure(void * context, NSError * error) { DispatchCallbackResult(context, error, nil); }

private:
    static void DispatchCallbackResult(void * context, NSError * _Nullable error, id _Nullable value)
    {
        MTRCallbackBridge * callbackBridge = static_cast<MTRCallbackBridge *>(context);
        if (!callbackBridge) {
            return;
        }

        if (!callbackBridge->mQueue) {
            ChipLogDetail(Controller, "%s %f seconds: can't dispatch response; no queue", callbackBridge->mCookie.UTF8String,
                -[callbackBridge->mRequestTime timeIntervalSinceNow]);
            if (!callbackBridge->mKeepAlive) {
                delete callbackBridge;
            }
            return;
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

    MTRResponseHandler mHandler;
    MTRActionBlock _Nullable mAction;
    // Keep our subclasses from accessing mKeepAlive directly, by putting this
    // "using" in our private section.
    using MTRCallbackBridgeBase::mKeepAlive;

    T mSuccess;
    MTRErrorCallback mFailure;

    // Measure the time it took for the callback to trigger
    NSDate * mRequestTime;
    NSString * mCookie;
};

NS_ASSUME_NONNULL_END
