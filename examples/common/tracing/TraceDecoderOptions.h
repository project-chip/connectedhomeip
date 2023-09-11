/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
