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

#include "CHIPCallbackWrapper.h"

#include <zap-generated/CHIPClientCallbacks.h>

namespace chip {

/** A success callback that delegates to the Java DefaultClusterCallback.onSuccess(). */
class CHIPDefaultSuccessCallback : public chip::CHIPCallbackWrapper<DefaultSuccessCallback>
{
public:
    CHIPDefaultSuccessCallback(jobject javaCallback, CHIPActionBlock action, bool keepAlive = false) :
        chip::CHIPCallbackWrapper<DefaultSuccessCallback>(javaCallback, action, CallbackFn, keepAlive)
    {}

    static void CallbackFn(void * context);
};

} // namespace chip
