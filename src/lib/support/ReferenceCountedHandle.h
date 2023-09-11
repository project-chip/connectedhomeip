/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    // Ideally we would suppress this from within Optional.h, where this false positive is coming from. That said suppressing
    // here is okay since no other cases could create instance of ReferenceCountedHandle without going through explicit
    // contstructor.
    //
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage): Only in a false positive is mTarget uninitialized.
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
