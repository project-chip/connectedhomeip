/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <lib/core/DataModelTypes.h>
#include <transport/Session.h>

namespace chip {
namespace app {

// First Interaction Model revision where SuppressResponse and TimedRequest are optional in Write Request and Invoke Request.
inline constexpr InteractionModelRevision kFirstRevisionWithOptionalRequestFlags = 14;

/**
 * Returns true if requests sent on `session` may omit SuppressResponse and TimedRequest when they are FALSE.
 *
 * The answer comes from the InteractionModelRevision the peer reported in the session parameters during session
 * establishment. A peer that did not report one is treated as older than the first revision with optional flags.
 * Group sessions have no remote session parameters and never carry the fields, so they always return true.
 */
inline bool PeerAcceptsOmittedRequestFlags(const SessionHandle & session)
{
    if (session->IsGroupSession())
    {
        return true;
    }
    const Optional<uint16_t> & peerRevision = session->GetRemoteSessionParameters().GetInteractionModelRevision();
    return peerRevision.HasValue() && peerRevision.Value() >= kFirstRevisionWithOptionalRequestFlags;
}

} // namespace app
} // namespace chip
