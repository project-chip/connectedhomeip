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

#include "Decoder.h"
#include "../logging/Log.h"

#include <lib/support/BufferReader.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/PASESession.h>

namespace {
constexpr char kProtocolName[] = "Secure Channel";

constexpr char kUnknown[]            = "Unknown";
constexpr char kMsgCounterSyncReq[]  = "Message Counter Sync Request";
constexpr char kMsgCounterSyncResp[] = "Message Counter Sync Response";
constexpr char kStandaloneAck[]      = "Standalone Ack";
constexpr char kPBKDFParamRequest[]  = "Password-Based Key Derivation Parameters Request";
constexpr char kPBKDFParamResponse[] = "Password-Based Key Derivation Parameters Response";
constexpr char kPASE_Pake1[]         = "Password Authenticated Session Establishment '1'";
constexpr char kPASE_Pake2[]         = "Password Authenticated Session Establishment '2'";
constexpr char kPASE_Pake3[]         = "Password Authenticated Session Establishment '3'";
constexpr char kCASE_Sigma1[]        = "Certificate Authenticated Session Establishment Sigma '1'";
constexpr char kCASE_Sigma2[]        = "Certificate Authenticated Session Establishment Sigma '2'";
constexpr char kCASE_Sigma3[]        = "Certificate Authenticated Session Establishment Sigma '3'";
constexpr char kCASE_Sigma2Resume[]  = "Certificate Authenticated Session Establishment Sigma '2' Resume";
constexpr char kStatusReport[]       = "Status Report";
} // namespace

using MessageType = chip::Protocols::SecureChannel::MsgType;

namespace chip {
namespace trace {
namespace secure_channel {

using namespace logging;

CHIP_ERROR DecodeSEDParams(TLV::TLVReader & reader);
CHIP_ERROR DecodeMessageCounterSyncRequest(TLV::TLVReader & reader);
CHIP_ERROR DecodeMessageCounterSyncResponse(TLV::TLVReader & reader);
CHIP_ERROR DecodeStandaloneAck(TLV::TLVReader & reader);
CHIP_ERROR DecodePBDFKParamRequest(TLV::TLVReader & reader);
CHIP_ERROR DecodePBDFKParamResponse(TLV::TLVReader & reader);
CHIP_ERROR DecodePASEPake1(TLV::TLVReader & reader);
CHIP_ERROR DecodePASEPake2(TLV::TLVReader & reader);
CHIP_ERROR DecodePASEPake3(TLV::TLVReader & reader);
CHIP_ERROR DecodeCASESigma1(TLV::TLVReader & reader);
CHIP_ERROR DecodeCASESigma2(TLV::TLVReader & reader);
CHIP_ERROR DecodeCASESigma3(TLV::TLVReader & reader);
CHIP_ERROR DecodeCASESigma2Resume(TLV::TLVReader & reader);
CHIP_ERROR DecodeStatusReport(const uint8_t * data, size_t len); // TODO StatusReport does not use TLV Encoding

const char * ToProtocolName()
{
    return kProtocolName;
}

const char * ToProtocolMessageTypeName(uint8_t protocolCode)
{
    switch (protocolCode)
    {
    case to_underlying(MessageType::MsgCounterSyncReq):
        return kMsgCounterSyncReq;
    case to_underlying(MessageType::MsgCounterSyncRsp):
        return kMsgCounterSyncResp;
    case to_underlying(MessageType::StandaloneAck):
        return kStandaloneAck;
    case to_underlying(MessageType::PBKDFParamRequest):
        return kPBKDFParamRequest;
    case to_underlying(MessageType::PBKDFParamResponse):
        return kPBKDFParamResponse;
    case to_underlying(MessageType::PASE_Pake1):
        return kPASE_Pake1;
    case to_underlying(MessageType::PASE_Pake2):
        return kPASE_Pake2;
    case to_underlying(MessageType::PASE_Pake3):
        return kPASE_Pake3;
    case to_underlying(MessageType::CASE_Sigma1):
        return kCASE_Sigma1;
    case to_underlying(MessageType::CASE_Sigma2):
        return kCASE_Sigma2;
    case to_underlying(MessageType::CASE_Sigma3):
        return kCASE_Sigma3;
    case to_underlying(MessageType::CASE_Sigma2Resume):
        return kCASE_Sigma2Resume;
    case to_underlying(MessageType::StatusReport):
        return kStatusReport;
    default:
        return kUnknown;
    }
}

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len)
{
    TLV::TLVReader reader;
    reader.Init(data, len);

    switch (protocolCode)
    {
    case to_underlying(MessageType::MsgCounterSyncReq):
        return DecodeMessageCounterSyncRequest(reader);
    case to_underlying(MessageType::MsgCounterSyncRsp):
        return DecodeMessageCounterSyncResponse(reader);
    case to_underlying(MessageType::StandaloneAck):
        return DecodeStandaloneAck(reader);
    case to_underlying(MessageType::PBKDFParamRequest):
        return DecodePBDFKParamRequest(reader);
    case to_underlying(MessageType::PBKDFParamResponse):
        return DecodePBDFKParamResponse(reader);
    case to_underlying(MessageType::PASE_Pake1):
        return DecodePASEPake1(reader);
    case to_underlying(MessageType::PASE_Pake2):
        return DecodePASEPake2(reader);
    case to_underlying(MessageType::PASE_Pake3):
        return DecodePASEPake3(reader);
    case to_underlying(MessageType::CASE_Sigma1):
        return DecodeCASESigma1(reader);
    case to_underlying(MessageType::CASE_Sigma2):
        return DecodeCASESigma2(reader);
    case to_underlying(MessageType::CASE_Sigma3):
        return DecodeCASESigma3(reader);
    case to_underlying(MessageType::CASE_Sigma2Resume):
        return DecodeCASESigma2Resume(reader);
    case to_underlying(MessageType::StatusReport):
        return DecodeStatusReport(data, len);
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

CHIP_ERROR DecodeMessageCounterSyncRequest(TLV::TLVReader & reader)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DecodeMessageCounterSyncResponse(TLV::TLVReader & reader)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DecodeStandaloneAck(TLV::TLVReader & reader)
{
    // Standalone Ack does not contains any data. Something wrong is ongoing if this code is reached.
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR DecodePBDFKParamRequest(TLV::TLVReader & reader)
{
    constexpr uint8_t kInitiatorRandomTag    = 1;
    constexpr uint8_t kInitiatorSessionIdTag = 2;
    constexpr uint8_t kPasscodeIdTag         = 3;
    constexpr uint8_t kHasPBKDFParametersTag = 4;
    constexpr uint8_t kInitiatorSEDParamsTag = 5;

    ByteSpan initiatorRandom;
    uint16_t initiatorSessionId;
    PasscodeId passcodeId;
    bool hasPBKDFParameters;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kInitiatorRandomTag)));
    ReturnErrorOnFailure(reader.Get(initiatorRandom));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kInitiatorSessionIdTag)));
    ReturnErrorOnFailure(reader.Get(initiatorSessionId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kPasscodeIdTag)));
    ReturnErrorOnFailure(reader.Get(passcodeId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kHasPBKDFParametersTag)));
    ReturnErrorOnFailure(reader.Get(hasPBKDFParameters));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("Initiator Random", initiatorRandom);
    Log("Initiator Session Id", initiatorSessionId);
    Log("Passcode Id", passcodeId);
    Log("hasPBKDFParameters", hasPBKDFParameters ? "true" : "false");

    CHIP_ERROR err = reader.Next();
    if (err == CHIP_END_OF_TLV)
    {
        return reader.ExitContainer(containerType);
    }
    ReturnErrorOnFailure(err);

    VerifyOrReturnError(reader.GetTag() == TLV::ContextTag(kInitiatorSEDParamsTag), CHIP_ERROR_INVALID_TLV_TAG);
    ReturnErrorOnFailure(DecodeSEDParams(reader));

    return reader.ExitContainer(containerType);
}

CHIP_ERROR DecodePBDFKParamResponse(TLV::TLVReader & reader)
{
    constexpr uint8_t kInitiatorRandomTag    = 1;
    constexpr uint8_t kResponderRandomTag    = 2;
    constexpr uint8_t kResponderSessionIdTag = 3;
    constexpr uint8_t kPBKDFParametersTag    = 4;
    constexpr uint8_t kInitiatorSEDParamsTag = 5;

    ByteSpan initiatorRandom;
    ByteSpan responderRandom;
    uint16_t responderSessionId;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kInitiatorRandomTag)));
    ReturnErrorOnFailure(reader.Get(initiatorRandom));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResponderRandomTag)));
    ReturnErrorOnFailure(reader.Get(responderRandom));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResponderSessionIdTag)));
    ReturnErrorOnFailure(reader.Get(responderSessionId));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("Initiator Random", initiatorRandom);
    Log("Responder Random", responderRandom);
    Log("Responder Session Id", responderSessionId);

    CHIP_ERROR err = reader.Next();
    if (err == CHIP_END_OF_TLV)
    {
        return reader.ExitContainer(containerType);
    }
    ReturnErrorOnFailure(err);

    if (TLV::TagNumFromTag(reader.GetTag()) == kPBKDFParametersTag)
    {
        uint32_t iterationCount;
        ByteSpan salt;

        ReturnErrorOnFailure(reader.EnterContainer(containerType));
        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(reader.Get(iterationCount));
        Log("Iteration Count", iterationCount);
        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(reader.Get(salt));
        Log("Salt", salt);
        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        err = reader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        return reader.ExitContainer(containerType);
    }
    ReturnErrorOnFailure(err);

    VerifyOrReturnError(reader.GetTag() == TLV::ContextTag(kInitiatorSEDParamsTag), CHIP_ERROR_INVALID_TLV_TAG);
    ReturnErrorOnFailure(DecodeSEDParams(reader));

    return reader.ExitContainer(containerType);
}

CHIP_ERROR DecodePASEPake1(TLV::TLVReader & reader)
{
    ByteSpan pA;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Get(pA));
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("pA", pA);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodePASEPake2(TLV::TLVReader & reader)
{
    ByteSpan pB;
    ByteSpan cB;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Get(pB));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Get(cB));
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("pB", pB);
    Log("cB", cB);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodePASEPake3(TLV::TLVReader & reader)
{
    ByteSpan cA;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Get(cA));
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("cA", cA);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeCASESigma1(TLV::TLVReader & reader)
{
    constexpr uint8_t kInitiatorRandomTag    = 1;
    constexpr uint8_t kInitiatorSessionIdTag = 2;
    constexpr uint8_t kDestinationIdTag      = 3;
    constexpr uint8_t kInitiatorEphPubKeyTag = 4;
    constexpr uint8_t kInitiatorSEDParamsTag = 5;
    constexpr uint8_t kResumptionIDTag       = 6;
    constexpr uint8_t kResume1MICTag         = 7;

    ByteSpan initiatorRandom;
    uint16_t initiatorSessionId;
    ByteSpan destinationId;
    ByteSpan initiatorEphPubKey;
    ByteSpan resumptionId;
    ByteSpan initiatorResumeMIC;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kInitiatorRandomTag)));
    ReturnErrorOnFailure(reader.Get(initiatorRandom));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kInitiatorSessionIdTag)));
    ReturnErrorOnFailure(reader.Get(initiatorSessionId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kDestinationIdTag)));
    ReturnErrorOnFailure(reader.Get(destinationId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kInitiatorEphPubKeyTag)));
    ReturnErrorOnFailure(reader.Get(initiatorEphPubKey));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("InitiatorRandom", initiatorRandom);
    Log("InitiatorSessionId", initiatorSessionId);
    Log("DestinationId", destinationId);
    Log("InitiatorEphPubKey", initiatorEphPubKey);

    CHIP_ERROR err = reader.Next();
    if (err == CHIP_NO_ERROR && reader.GetTag() == TLV::ContextTag(kInitiatorSEDParamsTag))
    {
        ReturnErrorOnFailure(DecodeSEDParams(reader));
        err = reader.Next();
    }

    if (err == CHIP_NO_ERROR && reader.GetTag() == TLV::ContextTag(kResumptionIDTag))
    {
        ReturnErrorOnFailure(reader.Get(resumptionId));
        Log("ResumptionID", resumptionId);
        err = reader.Next();
    }

    if (err == CHIP_NO_ERROR && reader.GetTag() == TLV::ContextTag(kResume1MICTag))
    {
        ReturnErrorOnFailure(reader.Get(initiatorResumeMIC));
        Log("InitiatorResumeMIC", initiatorResumeMIC);
        err = reader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    return reader.ExitContainer(containerType);
}

CHIP_ERROR DecodeCASESigma2(TLV::TLVReader & reader)
{
    constexpr uint8_t kResponderRandomTag    = 1;
    constexpr uint8_t kResponderSessionIdTag = 2;
    constexpr uint8_t kResponderEphPubKeyTag = 3;
    constexpr uint8_t kEncrypted2Tag         = 4;
    constexpr uint8_t kResponderSEDParamsTag = 5;

    ByteSpan responderRandom;
    uint16_t responderSessionId;
    ByteSpan responderEphPubKey;
    ByteSpan encrypted2;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResponderRandomTag)));
    ReturnErrorOnFailure(reader.Get(responderRandom));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResponderSessionIdTag)));
    ReturnErrorOnFailure(reader.Get(responderSessionId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResponderEphPubKeyTag)));
    ReturnErrorOnFailure(reader.Get(responderEphPubKey));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kEncrypted2Tag)));
    ReturnErrorOnFailure(reader.Get(encrypted2));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("ResponderRandom", responderRandom);
    Log("ResponderSessionId", responderSessionId);
    Log("ResponderEphPubKey", responderEphPubKey);
    Log("Encrypted2", encrypted2);

    CHIP_ERROR err = reader.Next();
    if (err == CHIP_NO_ERROR && reader.GetTag() == TLV::ContextTag(kResponderSEDParamsTag))
    {
        ReturnErrorOnFailure(DecodeSEDParams(reader));
        err = reader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    return reader.ExitContainer(containerType);
}

CHIP_ERROR DecodeCASESigma3(TLV::TLVReader & reader)
{
    constexpr uint8_t kEncrypted3Tag = 1;

    ByteSpan encrypted3;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kEncrypted3Tag)));
    ReturnErrorOnFailure(reader.Get(encrypted3));
    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("Encrypted3", encrypted3);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeCASESigma2Resume(TLV::TLVReader & reader)
{
    constexpr uint8_t kResumptionIDTag       = 1;
    constexpr uint8_t kSigma2ResumeMICTag    = 2;
    constexpr uint8_t kResponderSessionIdTag = 3;
    constexpr uint8_t kResponderSEDParamsTag = 4;

    ByteSpan resumptionId;
    ByteSpan sigma2ResumeMIC;
    uint16_t responderSessionId;

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.Next(containerType, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResumptionIDTag)));
    ReturnErrorOnFailure(reader.Get(resumptionId));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kSigma2ResumeMICTag)));
    ReturnErrorOnFailure(reader.Get(sigma2ResumeMIC));
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kResponderSessionIdTag)));
    ReturnErrorOnFailure(reader.Get(responderSessionId));

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("ResumptionID", resumptionId);
    Log("Sigma2ResumeMIC", sigma2ResumeMIC);
    Log("ResponderSessionId", responderSessionId);

    CHIP_ERROR err = reader.Next();
    if (err == CHIP_NO_ERROR && reader.GetTag() == TLV::ContextTag(kResponderSEDParamsTag))
    {
        ReturnErrorOnFailure(DecodeSEDParams(reader));
        err = reader.Next();
    }

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    return reader.ExitContainer(containerType);
}

CHIP_ERROR DecodeStatusReport(const uint8_t * data, size_t len)
{
    uint16_t statusGeneralStatusCode;
    uint32_t statusProtocolId;
    uint16_t statusProtocolCode;

    Encoding::LittleEndian::Reader bufReader(data, len);
    ReturnErrorOnFailure(
        bufReader.Read16(&statusGeneralStatusCode).Read32(&statusProtocolId).Read16(&statusProtocolCode).StatusCode());

    auto scopedIndent = ScopedLogIndent("Parameters");
    Log("GeneralStatusCode", statusGeneralStatusCode);
    Log("ProtocolId", statusProtocolId);
    Log("ProtocolCode", statusProtocolCode);

    if (bufReader.Remaining())
    {
        ByteSpan statusProtocolData(data + bufReader.OctetsRead(), bufReader.Remaining());
        Log("Data", statusProtocolData);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeSEDParams(TLV::TLVReader & reader)
{
    constexpr uint8_t kIdleRetransmitTimeoutTag   = 1;
    constexpr uint8_t kActiveRetransmitTimeoutTag = 2;

    auto scopedIndent = ScopedLogIndent("SED");

    TLV::TLVType containerType = TLV::kTLVType_Structure;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next());

    if (TLV::TagNumFromTag(reader.GetTag()) == kIdleRetransmitTimeoutTag)
    {
        uint32_t idleRetransmitTimeout;
        ReturnErrorOnFailure(reader.Get(idleRetransmitTimeout));
        Log("Idle Retransmit Timeout", idleRetransmitTimeout);

        CHIP_ERROR err = reader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            return reader.ExitContainer(containerType);
        }
        ReturnErrorOnFailure(err);
    }

    VerifyOrReturnError(TLV::TagNumFromTag(reader.GetTag()) == kActiveRetransmitTimeoutTag, CHIP_ERROR_INVALID_TLV_TAG);

    uint32_t activeRetransmitTimeout;
    ReturnErrorOnFailure(reader.Get(activeRetransmitTimeout));
    Log("Active Retransmit Timeout", activeRetransmitTimeout);

    ReturnErrorOnFailure(reader.ExitContainer(containerType));

    return CHIP_NO_ERROR;
}

} // namespace secure_channel
} // namespace trace
} // namespace chip
