/*
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

/**
 *  @file
 *    This file contains definitions for Callback objects for registering with
 *     Clusters and the Device
 */

#pragma once

#include "CHIPCallbackManager.h"
#include <stddef.h>
#include <stdint.h>

using namespace chip;
using namespace Callback;

void CallbackManager::AddPostAttributeChangeHandler(Callback::PostAttributeChangeCallback<> * onResponse, EndpointId endpointId,
                                                    ClusterId clusterId, AttributeId attributeId)
{
    // Since a cb is binded to endpoint and other args, should we have the init take those arguments to
    // make that relationship explicit. Or should we bind somewhere else?
    onResponse->mAttributeID = attributeId;
    onResponse->mEndpointID  = endpointId;
    onResponse->mClusterID   = clusterId;

    Callback::Cancelable * cancelable = onResponse->Cancel();
    mCallbacks.Enqueue(cancelable);
}

bool CallbackManager::ProcessPostAttributeChangeCallbacks(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                                                          uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                          uint8_t * value)
{
    Cancelable * ca = &mCallbacks;
    while (ca->mNext != &mCallbacks)
    {
        Callback::Callback<> * cb = Callback::Callback<>::FromCancelable(ca->mNext);
        if (cb != nullptr && cb->mCallbackType == kPostAttributeChangeCallbackType && cb->mEndpointID == endpoint &&
            cb->mClusterID == clusterId)
        {
            Callback::PostAttributeChangeCallback<PostAttributeChangeFn> * postAttributeCb =
                Callback::PostAttributeChangeCallback<PostAttributeChangeFn>::FromCancelable(ca->mNext);
            postAttributeCb->mCall(cb->mContext, endpoint, clusterId, attributeId, mask, manufacturerCode, type, size, value);
            return true;
        }
        ca = ca->mNext;
    }

    return false;
}
