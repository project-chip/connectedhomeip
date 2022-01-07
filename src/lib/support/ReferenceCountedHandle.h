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

/** ReferenceCountedHandle acts like a shared_ptr to an object derived from ReferenceCounted. In contrast to shared_ptr, the handle
 * will always hold a valid target */
template <typename Target>
class ReferenceCountedHandle
{
public:
    explicit ReferenceCountedHandle(Target & target) : mTarget(target) { mTarget.Retain(); }
    ~ReferenceCountedHandle() { mTarget.Release(); }

    ReferenceCountedHandle(const ReferenceCountedHandle & that) : mTarget(that.mTarget) { mTarget.Retain(); }

    ReferenceCountedHandle(ReferenceCountedHandle && that) : mTarget(that.mTarget) { mTarget.Retain(); }

    ReferenceCountedHandle & operator=(const ReferenceCountedHandle & that) = delete;
    ReferenceCountedHandle & operator=(ReferenceCountedHandle && that) = delete;

    bool operator==(const ReferenceCountedHandle & that) const { return &mTarget == &that.mTarget; }
    bool operator!=(const ReferenceCountedHandle & that) const { return !(*this == that); }

    Target * operator->() const { return &mTarget; }
    Target & Get() const { return mTarget; }

private:
    Target & mTarget;
};

} // namespace chip
