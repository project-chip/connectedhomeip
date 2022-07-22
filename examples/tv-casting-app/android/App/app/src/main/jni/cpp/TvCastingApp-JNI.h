/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <jni.h>

#include "Constants.h"
#include "MatterCallbackHandler-JNI.h"

class TvCastingAppJNI
{
public:
    MatterCallbackHandlerJNI & getCommissioningCompleteHandler() { return mCommissioningCompleteHandler; }
    MatterCallbackHandlerJNI & getMediaCommandResponseHandler(enum MediaCommandName name)
    {
        return mMediaCommandResponseHandler[name];
    }

private:
    friend TvCastingAppJNI & TvCastingAppJNIMgr();

    static TvCastingAppJNI sInstance;

    MatterCallbackHandlerJNI mCommissioningCompleteHandler;
    MatterCallbackHandlerJNI mMediaCommandResponseHandler[MEDIA_COMMAND_COUNT];
};

inline class TvCastingAppJNI & TvCastingAppJNIMgr()
{
    return TvCastingAppJNI::sInstance;
}
