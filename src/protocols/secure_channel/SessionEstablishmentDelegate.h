/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      This file defines the Delegate class that contains callbacks to
 *      establish a secure session and update status of the session establishment process.
 *
 */

#pragma once

#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

class DLL_EXPORT SessionEstablishmentDelegate
{
public:
    /**
     * @brief
     *   Called when session establishment fails with an error
     *
     * @param error error code
     *
     * TODO: Rename function as per issue: https://github.com/project-chip/connectedhomeip/issues/4468
     */
    virtual void OnSessionEstablishmentError(CHIP_ERROR error) {}

    /**
     * @brief
     *   Called when the new secure session has been established
     *
     * TODO: Rename function as per issue: https://github.com/project-chip/connectedhomeip/issues/4468
     */
    virtual void OnSessionEstablished() {}

    virtual ~SessionEstablishmentDelegate() {}
};

} // namespace chip
