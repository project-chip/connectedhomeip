/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <protocols/Protocols.h>

namespace chip {
namespace trace {

struct TraceDecoderOptions
{
    // Protocol
    bool mEnableProtocolSecureChannel             = true;
    bool mEnableProtocolInteractionModel          = true;
    bool mEnableProtocolBDX                       = true;
    bool mEnableProtocolUserDirectedCommissioning = true;
    bool mEnableProtocolEcho                      = true;

    // Disable the decoding of interaction model responses
    bool mEnableProtocolInteractionModelResponse = true;

    // Message
    bool mEnableMessageInitiator = true;
    bool mEnableMessageResponder = true;

    // Data
    bool mEnableDataEncryptedPayload = true;

    bool IsProtocolEnabled(chip::Protocols::Id id)
    {
        switch (id.ToFullyQualifiedSpecForm())
        {
        case chip::Protocols::SecureChannel::Id.ToFullyQualifiedSpecForm():
            return mEnableProtocolSecureChannel;
        case chip::Protocols::InteractionModel::Id.ToFullyQualifiedSpecForm():
            return mEnableProtocolInteractionModel;
        case chip::Protocols::BDX::Id.ToFullyQualifiedSpecForm():
            return mEnableProtocolBDX;
        case chip::Protocols::UserDirectedCommissioning::Id.ToFullyQualifiedSpecForm():
            return mEnableProtocolUserDirectedCommissioning;
        case chip::Protocols::Echo::Id.ToFullyQualifiedSpecForm():
            return mEnableProtocolEcho;
        default:
            return false;
        }
    }
};

} // namespace trace
} // namespace chip
