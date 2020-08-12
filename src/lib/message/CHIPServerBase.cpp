/*
 *
 *    Copyright (c) 2020 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements a common, non-instantiatable base object
 *      for implementing CHIP profile unsolicited responders
 *      (i.e. servers) that encapsulates validating authenticated
 *      requests and sending status reports and also provides common
 *      data member storage for fabric state and an exchange manager.
 *
 */

#include "CHIPServerBase.h"
#include <Profiles/common/CommonProfile.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <profiles/CHIPProfiles.h>
#include <support/CodeUtils.h>

namespace chip {

using namespace chip::Profiles;
using namespace chip::Encoding;
using namespace chip::TLV;

/**
 * Determine whether an incoming request message to a CHIP server should be accepted or discarded.
 *
 * This method is intended to be used by CHIP server implementations to implement extensible access control
 * policy for incoming request messages.  Server implementations that rely on delegate objects should call
 * this method early in message processing to determine whether message processing should continue.
 *
 * This method calls the virtual ChipServerDelegateBase::EnforceAccessControl() method on the supplied delegate to evaluate access
 * control policy for the message.  CHIP server delegate classes, and application-specific delegates derived
 * from the standard server classes, should override the virtual method to enforce specific access control
 * policies.
 *
 * @param[in] ec            The ExchangeContext over which the message was received.
 * @param[in] msgProfileId  The profile id of the received message.
 * @param[in] msgType       The message type of the received message.
 * @param[in] msgInfo       A ChipMessageInfo structure containing information about the received message.
 * @param[in] delegate      The delegate object supplied by the application that can be used to override
 *                          the default message access control policy.
 *
 * @retval true             If the message should be accepted and processed as normal.
 * @retval false            If message processing should stop and the message the should be discarded.
 */
bool ChipServerBase::EnforceAccessControl(ExchangeContext * ec, uint32_t msgProfileId, uint8_t msgType,
                                          const ChipMessageInfo * msgInfo, ChipServerDelegateBase * delegate)
{
    // Reject all messages if the application hasn't specified a delegate object.
    if (delegate == NULL)
    {
        ChipServerBase::SendStatusReport(ec, kChipProfile_Common, Common::kStatus_InternalError, CHIP_NO_ERROR);
        return false;
    }

    // Invoke the virtual method to evaluate the message and decide whether or not to accept it.
    ChipServerDelegateBase::AccessControlResult res = ChipServerDelegateBase::kAccessControlResult_NotDetermined;
    delegate->EnforceAccessControl(ec, msgProfileId, msgType, msgInfo, res);

    // If the final determination was that the message should be accepted AND the delegate called all the way up to
    // the base method, then tell the caller that the message should be accepted.
    if (res == ChipServerDelegateBase::kAccessControlResult_FinalAccepted)
    {
        return true;
    }

    // Otherwise the message should not be accepted...
    else
    {
        // Clear the 'Final' bit so that the following checks ignore it.
        res &= ~ChipServerDelegateBase::kAccessControlResult_IsFinal;

        // Send a standard response to the requester unless the delegate has already sent a response, or determined
        // that no response should be sent.
        if (res != ChipServerDelegateBase::kAccessControlResult_Rejected_RespSent &&
            res != ChipServerDelegateBase::kAccessControlResult_Rejected_Silent)
        {
            uint16_t statusCode = (msgInfo->PeerAuthMode == kChipAuthMode_None) ? Common::kStatus_AuthenticationRequired
                                                                                : Common::kStatus_AccessDenied;
            ChipServerBase::SendStatusReport(ec, kChipProfile_Common, statusCode, CHIP_NO_ERROR);
        }

        // Tell the caller the message should NOT be accepted.
        return false;
    }
}

/**
 *  Send a CHIP status report with default message flags to the
 *  initiator on the specified exchange containing the status code in
 *  the specified profile and system error.
 *
 *  @param[in]    ec               A pointer to the exchange context to send
 *                                 the status report on.
 *
 *  @param[in]    statusProfileId  The profile for the specified status code.
 *
 *  @param[in]    statusCode       The status code to send.
 *
 *  @param[in]    sysError         The system error associated or correlated
 *                                 with the status code.
 *
 */
CHIP_ERROR ChipServerBase::SendStatusReport(ExchangeContext * ec, uint32_t statusProfileId, uint16_t statusCode,
                                            CHIP_ERROR sysError)
{
    const uint16_t sendFlags = 0;

    return SendStatusReport(ec, statusProfileId, statusCode, sysError, sendFlags);
}

/**
 *  Send a CHIP status report with the provided message flags to the
 *  initiator on the specified exchange containing the status code in
 *  the specified profile and system error.
 *
 *  @param[in]    ec               A pointer to the exchange context to send
 *                                 the status report on.
 *
 *  @param[in]    statusProfileId  The profile for the specified status code.
 *
 *  @param[in]    statusCode       The status code to send.
 *
 *  @param[in]    sysError         The system error associated or correlated
 *                                 with the status code.
 *
 *  @param[in]    sendFlags        Flags set by the application for the CHIP
 *                                 status report being sent.
 *
 */
CHIP_ERROR ChipServerBase::SendStatusReport(ExchangeContext * ec, uint32_t statusProfileId, uint16_t statusCode,
                                            CHIP_ERROR sysError, uint16_t sendFlags)
{
    CHIP_ERROR err;
    PacketBuffer * respBuf;
    uint8_t * p;
    uint8_t respLen = 18; // sizeof(statusProfileId) + sizeof(statusCode) + StartContainer(1) + kTag_SystemErrorCode TLV Len (10),
                          // EndContainer (1)

    respBuf = PacketBuffer::NewWithAvailableSize(respLen);
    VerifyOrExit(respBuf != NULL, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrDie(ec != NULL);

    p = respBuf->Start();
    LittleEndian::Write32(p, statusProfileId);
    LittleEndian::Write16(p, statusCode);
    respBuf->SetDataLength(6);

    if (sysError != CHIP_NO_ERROR)
    {
        TLVWriter statusWriter;
        TLVType outerContainerType;

        statusWriter.Init(respBuf);

        err = statusWriter.StartContainer(AnonymousTag, kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);

        err = statusWriter.Put(ProfileTag(kChipProfile_Common, Common::kTag_SystemErrorCode), (uint32_t) sysError);
        SuccessOrExit(err);

        err = statusWriter.EndContainer(outerContainerType);
        SuccessOrExit(err);

        err = statusWriter.Finalize();
        SuccessOrExit(err);
    }

    err     = ec->SendMessage(kChipProfile_Common, Common::kMsgType_StatusReport, respBuf, sendFlags);
    respBuf = NULL;

exit:
    if (respBuf != NULL)
        PacketBuffer::Free(respBuf);
    return err;
}

/**
 * Virtual method for determining message-level access control policy for incoming server request messages.
 *
 * This method is called by the CHIP server infrastructure to determine whether an incoming request message
 * should be accepted and processed normally, or rejected.  Delegate classes associated with CHIP server
 * implementations must override this method to implement an appropriate access control policies for their
 * protocols.  Applications may further override this method to support custom policies beyond those provide
 * by the standard server implementations.
 *
 * Implementations of this method are expected to return a result value of Accepted or Rejected based on
 * the outcome of access control policy evaluation.  Returning a result of Rejected causes a StatusReport
 * to be sent to the requester containing the status Common/AccessDenied.  Alternatively, method implementations
 * can choose to send their own responses, which can be a StatusReport or any other type of message.  In this
 * case, the method should return a result of Reject_RespSent to signal that a response has already been sent.
 * Finally, implementations can return Reject_Silent to indicate that the request should be rejected without
 * sending a response to the requester.
 *
 * Classes that override the EnforceAccessControl() method are required in call cases to call the like-named
 * method on their immediate parent class, be that the ChipServerDelegateBase class, or a class derived from
 * that class. Overriding methods should first update the result value with their determination of the access
 * control policy, and then call on their base class to make its determination.
 *
 * @param[in] ec            The ExchangeContext over which the message was received.
 * @param[in] msgProfileId  The profile id of the received message.
 * @param[in] msgType       The message type of the received message.
 * @param[in] msgInfo       A ChipMessageInfo structure containing information about the received message.
 * @param[inout] result     An enumerated value describing the result of access control policy evaluation for
 *                          the received message. Upon entry to the method, the value represents the tentative
 *                          result at the current point in the evaluation process.  Upon return, the result
 *                          is expected to represent the final assessment of access control policy for the
 *                          message.
 */
void ChipServerDelegateBase::EnforceAccessControl(ExchangeContext * ec, uint32_t msgProfileId, uint8_t msgType,
                                                  const ChipMessageInfo * msgInfo, AccessControlResult & result)
{
    // Mark the result as 'Final', confirming that the subclass properly called up to the base class.
    result |= kAccessControlResult_IsFinal;
}

} // namespace chip
