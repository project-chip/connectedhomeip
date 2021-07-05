/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

namespace chip {

namespace Messaging {

class ExchangeContext;

/**
 * ExchangeHandle acts like a unique_ptr to an ExchangeContext, to help managing the life cycle of the ExchangeContext
 */
class ExchangeHandle
{
public:
    explicit ExchangeHandle() : mContext(nullptr) {}
    ~ExchangeHandle();

    ExchangeHandle(const ExchangeHandle & that);
    ExchangeHandle(ExchangeHandle && that) : mContext(that.mContext) { that.mContext = nullptr; }

    ExchangeHandle & operator=(const ExchangeHandle & that);
    ExchangeHandle & operator=(ExchangeHandle && that)
    {
        if (this == &that)
            return *this;
        mContext      = that.mContext;
        that.mContext = nullptr;
        return *this;
    }

    bool HasValue() const { return mContext != nullptr; }
    void Release();

    ExchangeContext * operator->() const { return mContext; }
    bool operator==(const ExchangeHandle & that) const { return mContext == that.mContext; }
    bool operator!=(const ExchangeHandle & that) const { return !(*this == that); }

private:
    friend class ExchangeContext;
    friend class ExchangeManager;
    explicit ExchangeHandle(ExchangeContext * context);
    ExchangeContext * mContext;
};

} // namespace Messaging
} // namespace chip
