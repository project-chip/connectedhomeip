/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
