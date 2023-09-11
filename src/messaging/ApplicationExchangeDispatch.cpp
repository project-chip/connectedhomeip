/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file provides implementation of Application Channel class.
 */

#include <messaging/ApplicationExchangeDispatch.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Messaging {

bool ApplicationExchangeDispatch::MessagePermitted(Protocols::Id protocol, uint8_t type)
{
    // TODO: Change this check to only include the protocol and message types that are allowed
    if (protocol == Protocols::SecureChannel::Id)
    {
        switch (type)
        {
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PBKDFParamRequest):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PBKDFParamResponse):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Pake1):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Pake2):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Pake3):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma1):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma2):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma3):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma2Resume):
            return false;

        default:
            break;
        }
    }

    return true;
}

} // namespace Messaging
} // namespace chip
