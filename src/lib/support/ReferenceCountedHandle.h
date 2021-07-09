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

/** ReferenceCountedHandle acts like a unique_ptr to an object derived from ReferenceCounted.  */
template <typename Target>
class ReferenceCountedHandle
{
public:
    explicit ReferenceCountedHandle() : mTarget(nullptr) {}

    explicit ReferenceCountedHandle(Target * context) : mTarget(context)
    {
        if (mTarget != nullptr)
            mTarget->Retain();
    }

    ~ReferenceCountedHandle()
    {
        if (mTarget != nullptr)
            mTarget->Release();
    }

    ReferenceCountedHandle(const ReferenceCountedHandle & that) : mTarget(that.mTarget)
    {
        if (mTarget != nullptr)
            mTarget->Retain();
    }

    ReferenceCountedHandle & operator=(const ReferenceCountedHandle & that)
    {
        if (this == &that)
            return *this;
        Release();
        mTarget = that.mTarget;
        if (mTarget != nullptr)
            mTarget->Retain();
        return *this;
    }

    ReferenceCountedHandle(ReferenceCountedHandle && that) : mTarget(that.mTarget) { that.mTarget = nullptr; }

    ReferenceCountedHandle & operator=(ReferenceCountedHandle && that)
    {
        if (this == &that)
            return *this;
        Release();
        mTarget      = that.mTarget;
        that.mTarget = nullptr;
        return *this;
    }

    bool HasValue() const { return mTarget != nullptr; }

    void Release()
    {
        if (mTarget != nullptr)
        {
            mTarget->Release();
            mTarget = nullptr;
        }
    }

    Target * Get() const { return mTarget; }
    Target * operator->() const { return mTarget; }
    bool operator==(const ReferenceCountedHandle & that) const { return mTarget == that.mTarget; }
    bool operator!=(const ReferenceCountedHandle & that) const { return !(*this == that); }

private:
    Target * mTarget;
};

} // namespace chip
