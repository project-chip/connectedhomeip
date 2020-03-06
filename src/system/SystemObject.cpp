/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *    @file
 *      This file contains definitions of member functions for class
 *      nl::Weave::System::Object.
 */

// Include module header
#include <SystemLayer/SystemObject.h>

// Include common private header
#include "SystemLayerPrivate.h"

// Include local headers
#include <SystemLayer/SystemLayer.h>
#include <Weave/Support/CodeUtils.h>

// Include local headers
#include <stddef.h>
#include <string.h>

namespace nl {
namespace Weave {
namespace System {

/**
 *  @brief
 *      Decrements the reference count for the Weave System Layer object. Recycles the object back into the pool if the reference
 *      count is decremented to zero. No destructor is invoked.
 */
NL_DLL_EXPORT void Object::Release(void)
{
    unsigned int oldCount = __sync_fetch_and_sub(&this->mRefCount, 1);

    if (oldCount == 1)
    {
        this->mSystemLayer = NULL;
        __sync_synchronize();
    }
    else if (oldCount == 0)
    {
        WeaveDie();
    }
}

NL_DLL_EXPORT bool Object::TryCreate(Layer& aLayer, size_t aOctets)
{
    bool lReturn = false;

    if (__sync_bool_compare_and_swap(&this->mSystemLayer, NULL, &aLayer))
    {
        this->mRefCount = 0;
        this->AppState = NULL;
        memset(reinterpret_cast<char*>(this) + sizeof(*this), 0, aOctets - sizeof(*this));

        this->Retain();
        lReturn = true;
    }

    return lReturn;
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
void Object::DeferredRelease(Object::ReleaseDeferralErrorTactic aTactic)
{
    Layer& lSystemLayer = *this->mSystemLayer;
    Error lError = lSystemLayer.PostEvent(*this, Weave::System::kEvent_ReleaseObj, 0);

    if (lError != WEAVE_SYSTEM_NO_ERROR)
    {
        switch (aTactic)
        {
        case kReleaseDeferralErrorTactic_Ignore:
            break;

        case kReleaseDeferralErrorTactic_Release:
            this->Release();
            break;

        case kReleaseDeferralErrorTactic_Die:
            VerifyOrDieWithMsg(false, WeaveSystemLayer, "Object::DeferredRelease %p->PostEvent failed err(%d)", &lSystemLayer,
                static_cast<int>(lError));
            break;
        }
    }
}
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace Weave
} // namespace nl
