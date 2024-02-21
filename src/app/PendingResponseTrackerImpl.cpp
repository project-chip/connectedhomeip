/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/PendingResponseTrackerImpl.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

CHIP_ERROR PendingResponseTrackerImpl::Add(uint16_t aCommandRef)
{
    VerifyOrReturnError(!IsTracked(aCommandRef), CHIP_ERROR_INVALID_ARGUMENT);
    mCommandReferenceSet.insert(aCommandRef);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PendingResponseTrackerImpl::Remove(uint16_t aCommandRef)
{
    VerifyOrReturnError(IsTracked(aCommandRef), CHIP_ERROR_KEY_NOT_FOUND);
    mCommandReferenceSet.erase(aCommandRef);
    return CHIP_NO_ERROR;
}

bool PendingResponseTrackerImpl::IsTracked(uint16_t aCommandRef)
{
    return mCommandReferenceSet.find(aCommandRef) != mCommandReferenceSet.end();
}

size_t PendingResponseTrackerImpl::Count()
{
    return mCommandReferenceSet.size();
}

Optional<uint16_t> PendingResponseTrackerImpl::PopPendingResponse()
{
    if (Count() == 0)
    {
        return NullOptional;
    }
    uint16_t commandRef = *mCommandReferenceSet.begin();
    mCommandReferenceSet.erase(mCommandReferenceSet.begin());
    return MakeOptional(commandRef);
}

} // namespace app
} // namespace chip
