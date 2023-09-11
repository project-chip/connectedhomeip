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

#include <lib/core/CHIPError.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>

namespace chip {

using namespace Messaging;

bool SessionEstablishmentExchangeDispatch::MessagePermitted(Protocols::Id protocol, uint8_t type)
{
    if (protocol == Protocols::SecureChannel::Id)
    {
        switch (type)
        {
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::StandaloneAck):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PBKDFParamRequest):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PBKDFParamResponse):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Pake1):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Pake2):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Pake3):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma1):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma2):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma3):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_Sigma2Resume):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::StatusReport):
            return true;

        default:
            break;
        }
    }

    return false;
}

} // namespace chip
