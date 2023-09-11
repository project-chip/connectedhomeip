/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceConnectionBridge.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRError_Internal.h"

void MTRDeviceConnectionBridge::OnConnected(
    void * context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle)
{
    auto * object = static_cast<MTRDeviceConnectionBridge *>(context);
    object->mCompletionHandler(&exchangeMgr, chip::MakeOptional<chip::SessionHandle>(*sessionHandle->AsSecureSession()), nil);
    object->Release();
}

void MTRDeviceConnectionBridge::OnConnectionFailure(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error)
{
    auto * object = static_cast<MTRDeviceConnectionBridge *>(context);
    object->mCompletionHandler(nil, chip::NullOptional, [MTRError errorForCHIPErrorCode:error]);
    object->Release();
}
