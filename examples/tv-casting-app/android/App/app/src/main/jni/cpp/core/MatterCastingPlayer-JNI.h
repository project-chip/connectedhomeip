/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#pragma once

#include "../support/MatterCallback-JNI.h"
#include "core/CastingPlayer.h" // from tv-casting-common

#include <jni.h>

namespace matter {
namespace casting {
namespace core {

class MatterCastingPlayerJNI
{
public:
    MatterCastingPlayerJNI() : mConnectionSuccessHandler([](void *) { return nullptr; }) {}
    support::MatterCallbackJNI<void *> mConnectionSuccessHandler;
    support::MatterFailureCallbackJNI mConnectionFailureHandler;

private:
    friend MatterCastingPlayerJNI & MatterCastingPlayerJNIMgr();
    static MatterCastingPlayerJNI sInstance;
};

inline class MatterCastingPlayerJNI & MatterCastingPlayerJNIMgr()
{
    return MatterCastingPlayerJNI::sInstance;
}
}; // namespace core
}; // namespace casting
}; // namespace matter
