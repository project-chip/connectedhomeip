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

#include <stddef.h>
#include <stdint.h>

#include <app/util/basic-types.h>
#include <core/CHIPCallback.h>
#include <core/CHIPCore.h>
#include <core/CHIPError.h>

namespace chip {

// namespace Callback {

class CallbackManager
{
public:
    CallbackManager() {}
    ~CallbackManager() {}

    void AddPostAttributeChangeHandler(Callback::PostAttributeChangeCallback<> * onResponse, EndpointId endpointId,
                                       ClusterId clusterId, AttributeId attributeId);
    bool ProcessPostAttributeChangeCallbacks(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                             uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value);

private:
    /* Track all outstanding response callbacks for this manager. */
    Callback::CallbackDeque mCallbacks;
};

// } // namespace Callback
} // namespace chip
