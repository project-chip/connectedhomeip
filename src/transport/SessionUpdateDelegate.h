/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/ScopedNodeId.h>
#include <lib/support/DLLUtil.h>

namespace chip {

/**
 * @brief
 *   Delegate interface that will be notified by ReliableMessageMgr when an exchange
 *   fails to deliver the first message.
 */
class DLL_EXPORT SessionUpdateDelegate
{
public:
    virtual ~SessionUpdateDelegate() {}

    virtual void UpdatePeerAddress(ScopedNodeId peerId) = 0;
};

} // namespace chip
