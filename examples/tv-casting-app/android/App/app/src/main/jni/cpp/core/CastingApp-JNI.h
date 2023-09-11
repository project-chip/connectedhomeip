/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

namespace matter {
namespace casting {
namespace core {

class CastingAppJNI
{
public:
private:
    friend CastingAppJNI & CastingAppJNIMgr();
    static CastingAppJNI sInstance;
};

inline class CastingAppJNI & CastingAppJNIMgr()
{
    return CastingAppJNI::sInstance;
}
}; // namespace core
}; // namespace casting
}; // namespace matter
