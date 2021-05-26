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

#include <messaging/ExchangeHandle.h>

#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Messaging {

ExchangeHandle::ExchangeHandle(ExchangeContext * context) : mContext(context)
{
    if (mContext != nullptr)
        mContext->Retain();
}

ExchangeHandle::ExchangeHandle(const ExchangeHandle & that) : mContext(that.mContext)
{
    if (mContext != nullptr)
        mContext->Retain();
}

ExchangeHandle::~ExchangeHandle()
{
    if (mContext != nullptr)
        mContext->Release();
}

ExchangeHandle & ExchangeHandle::operator=(const ExchangeHandle & that)
{
    if (this == &that)
        return *this;
    mContext = that.mContext;
    if (mContext != nullptr)
        mContext->Retain();
    return *this;
}

void ExchangeHandle::Release()
{
    if (mContext != nullptr)
    {
        mContext->Release();
        mContext = nullptr;
    }
}

} // namespace Messaging
} // namespace chip
