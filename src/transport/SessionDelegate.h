/*
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

#pragma once

#include <transport/SessionHandle.h>

namespace chip {

class DLL_EXPORT SessionCreationDelegate
{
public:
    virtual ~SessionCreationDelegate() {}

    /**
     * @brief
     *   Called when a new session is being established
     *
     * @param session   The handle to the secure session
     */
    virtual void OnNewSession(SessionHandle session) = 0;
};

class DLL_EXPORT SessionReleaseDelegate
{
public:
    virtual ~SessionReleaseDelegate() {}

    /**
     * @brief
     *   Called when a session is releasing
     *
     * @param session   The handle to the secure session
     */
    virtual void OnSessionReleased(SessionHandle session) = 0;
};

} // namespace chip
