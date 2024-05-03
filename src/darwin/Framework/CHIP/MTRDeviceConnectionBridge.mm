/**
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

#import "MTRDeviceConnectionBridge.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRError_Internal.h"

void MTRDeviceConnectionBridge::OnConnected(
    void * context, chip::Messaging::ExchangeManager & exchangeMgr, const chip::SessionHandle & sessionHandle)
{
    auto * object = static_cast<MTRDeviceConnectionBridge *>(context);
    object->mCompletionHandler(&exchangeMgr, chip::MakeOptional<chip::SessionHandle>(*sessionHandle->AsSecureSession()), nil, nil);
    object->Release();
}

void MTRDeviceConnectionBridge::OnConnectionFailure(void * context, const chip::OperationalSessionSetup::ConnnectionFailureInfo & failureInfo)
{
    NSNumber * retryDelay;
    if (failureInfo.requestedBusyDelay.HasValue()) {
        retryDelay = @(static_cast<double>(failureInfo.requestedBusyDelay.Value().count()) / MSEC_PER_SEC);
    }
    auto * object = static_cast<MTRDeviceConnectionBridge *>(context);
    object->mCompletionHandler(nil, chip::NullOptional, [MTRError errorForCHIPErrorCode:failureInfo.error], retryDelay);
    object->Release();
}
