/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceController.h"

#import <Foundation/Foundation.h>

#include <controller/CHIPDeviceController.h>
#include <lib/core/ReferenceCounted.h>
#include <messaging/ExchangeMgr.h>
#include <transport/Session.h>

NS_ASSUME_NONNULL_BEGIN

// Either exchangeManager will be non-nil and session will have a value, or
// error will be non-nil.
typedef void (^MTRInternalDeviceConnectionCallback)(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
    const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error);

/**
 * Helper to establish or look up a CASE session and hand its session
 * information back to the consumer.
 */
class MTRDeviceConnectionBridge : public chip::ReferenceCounted<MTRDeviceConnectionBridge> {
public:
    MTRDeviceConnectionBridge(MTRInternalDeviceConnectionCallback completionHandler)
        : mCompletionHandler(completionHandler)
        , mOnConnected(OnConnected, this)
        , mOnConnectFailed(OnConnectionFailure, this)
    {
    }

    ~MTRDeviceConnectionBridge()
    {
        mOnConnected.Cancel();
        mOnConnectFailed.Cancel();
    }

    /**
     * connect must be called on the Matter queue, and will invoke the
     * completionHandler on the Matter queue as well.
     */
    CHIP_ERROR connect(chip::Controller::DeviceController * controller, chip::NodeId deviceID)
    {
        return controller->GetConnectedDevice(deviceID, &mOnConnected, &mOnConnectFailed);
    }

private:
    MTRInternalDeviceConnectionCallback mCompletionHandler;
    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnected;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectFailed;

    static void OnConnected(
        void * context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle);
    static void OnConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);
};

NS_ASSUME_NONNULL_END
