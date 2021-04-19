/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <support/CHIPMem.h>

#include "system/TLVPacketBufferBackingStore.h"
#include "transport/DeviceAttestation.h"
#include "transport/SecureSessionMgr.h"

#include "dac_chain.h"

#ifdef CONFIG_COMMISSIONER_ENABLED
extern "C" {
#include <certifier/base64.h>
#include <certifier/certifier_api_easy.h>
#include <certifier/certifier_internal.h>
#include <certifier/http.h>
#include <certifier/parson.h>
#include <certifier/util.h>
}
} // the include seems to consume one openning brace
#endif // CONFIG_COMMISSIONER_ENABLED

/**
 * autonumber
participant "Commissioner" as cr
participant "Commissionee" as ce #33FCFF

+ Establish a PASE session
+ cr --> ce : nonce, begin attestation flow
+ ce -> ce: Create signature of nonce using\n private key associated with DAC
+ ce -> cr : DAC, signed nonce (from previous step)
+ cr -> cr: DAC is valid and chains up to PAI and PAA
+ cr -> cr: The signed nonce is validated with\nDAC public key
+ cr -> cr: Device attestation is successful
+ cr -> ce: CSR request
+ ce -> cr: CSR, nodeAddress (or OperID), CD and FM
+ cr -> certifier: requests Oper. Credentials
+ certifier -> cr: chain of certificates
+ cr -> cr: extract certificates from PKCS7 -> DER -> CHIP format. (TODO: have PKI return CHIP credentials)
+ cr -> ce: the operational cert (chain of certificates incl. Oper. Certificate in future)
 *
 */

namespace chip {

using namespace Transport;
using namespace std::chrono;

static constexpr uint16_t kStandardCertsCount = 5;
static constexpr uint16_t kTestCertBufSize    = 1024; // Size of buffer needed to hold any of the test certificates
                                                      // (in either CHIP or DER form), or to decode the certificates.

const uint8_t device_certificate_declaration[] = {
    0xa3, 0x63, 0x43, 0x44, 0x50, 0xa6, 0x63, 0x56, 0x69, 0x64, 0x19, 0x45, 0x67, 0x63, 0x50, 0x69, 0x64, 0x19, 0x23, 0xc6, 0x63,
    0x43, 0x69, 0x64, 0x58, 0x19, 0x56, 0xc4, 0x47, 0x8e, 0x7a, 0xa1, 0x5e, 0x83, 0xf3, 0xea, 0xa3, 0xb7, 0x25, 0x9b, 0x05, 0x3e,
    0x23, 0xce, 0x28, 0x67, 0x22, 0x1c, 0xff, 0x38, 0x83, 0x62, 0x53, 0x4c, 0x18, 0x73, 0x62, 0x53, 0x49, 0x19, 0xdc, 0x51, 0x65,
    0x4d, 0x69, 0x6e, 0x46, 0x57, 0x19, 0x5c, 0xff, 0x62, 0x53, 0x43, 0x59, 0x02, 0x37, 0x30, 0x82, 0x02, 0x33, 0x30, 0x82, 0x01,
    0xd8, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x14, 0x58, 0x65, 0xd3, 0xa1, 0xbd, 0x4f, 0xca, 0x23, 0x6a, 0x6c, 0x0c, 0x69, 0x79,
    0xdb, 0x21, 0xad, 0xa2, 0x9b, 0x93, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x1e,
    0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x13, 0x43, 0x48, 0x49, 0x50, 0x20, 0x44, 0x45, 0x56, 0x20, 0x43,
    0x6c, 0x61, 0x73, 0x73, 0x33, 0x20, 0x49, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x30, 0x37, 0x31, 0x30, 0x31, 0x36,
    0x32, 0x38, 0x30, 0x34, 0x5a, 0x17, 0x0d, 0x33, 0x30, 0x30, 0x37, 0x30, 0x38, 0x31, 0x38, 0x32, 0x38, 0x30, 0x34, 0x5a, 0x30,
    0x81, 0x92, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
    0x55, 0x04, 0x08, 0x0c, 0x02, 0x50, 0x41, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x0c, 0x50, 0x68, 0x69,
    0x6c, 0x61, 0x64, 0x65, 0x6c, 0x70, 0x68, 0x69, 0x61, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x13, 0x61,
    0x74, 0x74, 0x65, 0x73, 0x74, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2e, 0x78, 0x70, 0x6b, 0x69, 0x2e, 0x69, 0x6f, 0x31, 0x13, 0x30,
    0x11, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x31, 0x10, 0x30,
    0x0e, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x07, 0x43, 0x6f, 0x6d, 0x63, 0x61, 0x73, 0x74, 0x31, 0x1a, 0x30, 0x18, 0x06, 0x0a,
    0x09, 0x92, 0x26, 0x89, 0x93, 0xf2, 0x2c, 0x64, 0x01, 0x01, 0x0c, 0x0a, 0x4f, 0x70, 0x65, 0x6e, 0x73, 0x6f, 0x75, 0x72, 0x63,
    0x65, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xd5, 0x43, 0x5d, 0x95, 0x53, 0x5d, 0x58, 0x49, 0xa2, 0xc4, 0xd7, 0xae, 0x43, 0x31,
    0xc3, 0x25, 0x71, 0xd9, 0x6c, 0x0c, 0xe0, 0xff, 0xc6, 0x25, 0x3c, 0x52, 0x6d, 0xf2, 0xe7, 0x0c, 0x67, 0x1b, 0xfe, 0x39, 0x0e,
    0xae, 0x94, 0xaa, 0x23, 0x07, 0xc9, 0xf7, 0x4e, 0xb7, 0xed, 0xb1, 0xdf, 0xe0, 0xc4, 0x35, 0x89, 0x96, 0x29, 0x3a, 0x2e, 0x69,
    0x1a, 0xc2, 0x05, 0xe2, 0xf4, 0xc7, 0xe8, 0x6a, 0xa3, 0x7f, 0x30, 0x7d, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01,
    0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x9e, 0x6f, 0x44,
    0xaa, 0x8d, 0xfb, 0x62, 0x5b, 0x78, 0xcd, 0x0f, 0x76, 0x1f, 0xc9, 0x21, 0xba, 0x4c, 0x3b, 0x9a, 0x90, 0x30, 0x1d, 0x06, 0x03,
    0x55, 0x1d, 0x25, 0x04, 0x16, 0x30, 0x14, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, 0x06, 0x08, 0x2b, 0x06,
    0x01, 0x05, 0x05, 0x07, 0x03, 0x01, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x92, 0xc7, 0x7c, 0x90,
    0x27, 0x84, 0xa9, 0x7d, 0x2e, 0xd4, 0xf7, 0x24, 0x13, 0x1a, 0xc2, 0xcd, 0xde, 0x5d, 0x34, 0x53, 0x30, 0x0e, 0x06, 0x03, 0x55,
    0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x05, 0xa0, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04,
    0x03, 0x02, 0x03, 0x49, 0x00, 0x30, 0x46, 0x02, 0x21, 0x00, 0xf0, 0x69, 0xbb, 0x9f, 0xfb, 0x67, 0x2c, 0xd3, 0x0d, 0xb6, 0xe9,
    0xe7, 0x2d, 0x1b, 0xef, 0xe1, 0x89, 0x90, 0xb2, 0x26, 0x39, 0xe6, 0x33, 0x6a, 0x24, 0x0d, 0x8d, 0xb5, 0x7a, 0xed, 0xcb, 0xa2,
    0x02, 0x21, 0x00, 0xab, 0x73, 0xcd, 0x8a, 0x82, 0x30, 0x93, 0x38, 0xb2, 0x59, 0x1f, 0x1f, 0x32, 0x8a, 0xb1, 0x61, 0x59, 0x70,
    0x15, 0x49, 0x08, 0xec, 0x4c, 0x68, 0xf0, 0xb2, 0x0a, 0x75, 0x3f, 0xb0, 0xa8, 0x07, 0x63, 0x53, 0x47, 0x4e, 0x58, 0x49, 0x48,
    0x30, 0x46, 0x02, 0x21, 0x00, 0xee, 0xf3, 0x1d, 0x13, 0xb5, 0x48, 0xb2, 0x24, 0x91, 0x70, 0xfc, 0x9e, 0x79, 0xdf, 0x6c, 0x44,
    0x1e, 0x12, 0x36, 0xda, 0x74, 0xba, 0x72, 0x5d, 0xc9, 0xcc, 0x8b, 0xba, 0xeb, 0xea, 0xe4, 0x55, 0x02, 0x21, 0x00, 0xe5, 0xcd,
    0x25, 0x0a, 0xaf, 0x82, 0x57, 0x3e, 0xa3, 0x4f, 0x43, 0x38, 0xef, 0xde, 0xc1, 0x53, 0x3f, 0x3c, 0x40, 0x31, 0x0e, 0x34, 0x4e,
    0xa3, 0x71, 0xda, 0xd5, 0xed, 0xba, 0xa5, 0xd5, 0x87
};

CHIP_ERROR DeviceAttestation::Init(Messaging::ExchangeContext * exchangeContext, OperationalCredentialSet * opCredSet)
{
    P256SerializedKeypair serializedKeypair;

    mExchangeContext = exchangeContext;
    mOpCredSet       = opCredSet;
    mState           = DeviceAttestationState::kDeviceWaiting;

    ReturnErrorOnFailure(serializedKeypair.SetLength(sizeof(device_private_key_tmp) + sizeof(device_public_key_tmp)));

    memcpy(serializedKeypair, device_public_key_tmp, sizeof(device_public_key_tmp));
    memcpy(serializedKeypair + sizeof(device_public_key_tmp), device_private_key_tmp, sizeof(device_private_key_tmp));

    ReturnErrorOnFailure(mKeypair.Deserialize(serializedKeypair));

    ReturnErrorOnFailure(mChipCertificateSet.Init(kStandardCertsCount, kTestCertBufSize));

    mValidContext.Reset();
    mValidContext.mRequiredKeyUsages.Set(KeyUsageFlags::kDigitalSignature);
    mValidContext.mRequiredKeyPurposes.Set(KeyPurposeFlags::kServerAuth);

    mDeviceAttested = false;

    return CHIP_NO_ERROR;
}

void DeviceAttestation::Close()
{
    mChipCertificateSet.Release();
    mDeviceAttested = false;
}

#ifdef CONFIG_COMMISSIONER_ENABLED
CHIP_ERROR DeviceAttestation::StartAttestation()
{
    CHIP_ERROR err = CommissionerSendNonce();
    if (CHIP_NO_ERROR == err)
        mState = DeviceAttestationState::kCommissionerNonceSent;
    return err;
}
#else  // CONFIG_COMMISSIONER_ENABLED
CHIP_ERROR DeviceAttestation::StartAttestation()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif // CONFIG_COMMISSIONER_ENABLED

CHIP_ERROR DeviceAttestation::OnMessageReceived(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (mState)
    {
#ifdef CONFIG_COMMISSIONER_ENABLED
    case DeviceAttestationState::kCommissionerNonceSent:
        err = CommissionerValidateNonceResponse(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            err = CommissionerSendCSRRequest(exchangeContext);
            if (err == CHIP_NO_ERROR)
            {
                mState = DeviceAttestationState::kCommissionerCSRRequsted;
            }
            else
            {
                mState = DeviceAttestationState::kDeviceWaiting;
            }
        }
        else
        {
            mState = DeviceAttestationState::kDeviceWaiting;
        }
        break;

    case DeviceAttestationState::kCommissionerCSRRequsted:
        err = CommissionerProcessCSR(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            err = CommissionerSendRootCert(exchangeContext);
            if (err == CHIP_NO_ERROR)
            {
                err = CommissionerSendCACert(exchangeContext);
                if (err == CHIP_NO_ERROR)
                {
                    err = CommissionerSendOpCert(exchangeContext);
                    if (err == CHIP_NO_ERROR)
                    {
                        err = CommissionerSendACL(exchangeContext);
                        if (err == CHIP_NO_ERROR)
                        {
                            mState = DeviceAttestationState::kFinished;
                        }
                        else
                        {
                            mState = DeviceAttestationState::kDeviceWaiting;
                        }
                    }
                    else
                    {
                        mState = DeviceAttestationState::kDeviceWaiting;
                    }
                }
                else
                {
                    mState = DeviceAttestationState::kDeviceWaiting;
                }
            }
            else
            {
                mState = DeviceAttestationState::kDeviceWaiting;
            }
        }
        else
        {
            mState = DeviceAttestationState::kDeviceWaiting;
        }
        break;
#else  // CONFIG_COMMISSIONER_ENABLED
    case DeviceAttestationState::kDeviceWaiting:
        err = DeviceProcessNonce(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            mState = DeviceAttestationState::kDeviceDACSent;
        }
        break;

    case DeviceAttestationState::kDeviceDACSent:
        err = DeviceProcessCSRRequest(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            mState = DeviceAttestationState::kDeviceCSRSent;
        }
        break;

    case DeviceAttestationState::kDeviceCSRSent:
        err = DeviceProcessRootCert(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            mState = DeviceAttestationState::kDeviceRootCertProcessed;
        }
        break;

    case DeviceAttestationState::kDeviceRootCertProcessed:
        err = DeviceProcessCACert(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            mState = DeviceAttestationState::kDeviceCACertProcessed;
        }
        break;

    case DeviceAttestationState::kDeviceCACertProcessed:
        err = DeviceProcessOpCert(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            mState = DeviceAttestationState::kCommissinerOpCertProcessed;
        }
        break;
    case DeviceAttestationState::kCommissinerOpCertProcessed:
        err = DeviceProcessACL(std::move(msgBuf), exchangeContext);
        if (err == CHIP_NO_ERROR)
        {
            mState = DeviceAttestationState::kFinished;
        }
        break;
#endif // CONFIG_COMMISSIONER_ENABLED
    default:
        break;
    }
    return err;
}

#ifdef CONFIG_COMMISSIONER_ENABLED
CHIP_ERROR DeviceAttestation::CommissionerSendNonce()
{
    CHIP_ERROR err = CHIP_CONFIG_NO_ERROR;
    System::PacketBufferTLVWriter OpCredReqWriter;
    chip::TLV::TLVType outerType            = chip::TLV::kTLVType_NotSpecified;
    System::PacketBufferHandle OpCredReqBuf = chip::System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    char * nonceLocal                       = nullptr;
    std::unique_ptr<char *> nonceLocalUptr;
    if (OpCredReqBuf.IsNull())
    {
        ChipLogError(DataManagement, "kProtocol_OpCredentials Unable to allocate packet buffer");
    };

    OpCredReqWriter.Init(std::move(OpCredReqBuf));
    OpCredReqWriter.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, outerType);
    OpCredReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("Rq"), 2);
    memset(mNonce, 0, sizeof(mNonce));
    if (NULL == (nonceLocal = (util_generate_random_value(kNonceSize, ALLOWABLE_CHARACTERS))))
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials nonce not generated!");
    }
    nonceLocalUptr = std::make_unique<char *>(nonceLocal);
    memcpy(mNonce, nonceLocal, sizeof(mNonce));
    OpCredReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kNonce), mNonce, sizeof(mNonce));
    OpCredReqWriter.EndContainer(outerType);
    OpCredReqWriter.Finalize(&OpCredReqBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    mExchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCredReqBuf), sendFlags);
    ChipLogProgress(AppServer, "kProtocol_OpCredentials Sent an attestation request to the device with the nonce:\n%s",
                    reinterpret_cast<char *>(mNonce));

    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerValidateNonceResponse(System::PacketBufferHandle msgBuf,
                                                                Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader OpCredReqReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    chip::TLV::TLVType TlvContainerType = chip::TLV::kTLVType_Structure;
    OpCredReqReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = OpCredReqReader.Next(TlvContainerType, chip::TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = OpCredReqReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = OpCredReqReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                                                  SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = OpCredReqReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kCertificate),
                                                  SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Certificate tag.");
        SuccessOrExit(err);
    }
    else
    {
        mDerCertificateLength = SupplementaryTlvReader.GetLength();
        VerifyOrExit(mDerCertificateLength <= sizeof(mDerDeviceCertificate), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        switch (SupplementaryTlvReader.GetType())
        {
        case TLV::kTLVType_ByteString:
        case TLV::kTLVType_UTF8String: {
            if (CHIP_NO_ERROR != (err = SupplementaryTlvReader.GetBytes(mDerDeviceCertificate, mDerCertificateLength)))
            {
                ChipLogProgress(AppServer, "kProtocol_OpCredentials Failed to extract device certificate.");
                SuccessOrExit(err);
            }
            break;
        }
        default:
            err = CHIP_ERROR_INVALID_ARGUMENT;
            ChipLogProgress(
                AppServer,
                "kProtocol_OpCredentials Error parsing TLV, upper level blocks should not contain elements other than strings.");
            SuccessOrExit(err);
        }

        VerifyOrExit(mDerCertificateLength > 0, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        ChipLogProgress(AppServer, "kProtocol_OpCredentials Received device attestation materials.");
        err = ValidateCertificateChain(paa_certificate, sizeof(paa_certificate), pai_certificate, sizeof(pai_certificate),
                                       mDerDeviceCertificate, mDerCertificateLength);
        SuccessOrExit(err);

        // Extract Manufacturer Certificate Public Key
        err = ExtractPubkeyFromX509Cert(mDerDeviceCertificate, mDerCertificateLength, mRemoteManufacturerPubKey);
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR ==
        (err = OpCredReqReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kNonce),
                                                  SupplementaryTlvReader)))
    {
        P256ECDSASignature nonce_signature;

        size_t nonce_signature_size = SupplementaryTlvReader.GetLength();

        VerifyOrExit(nonce_signature_size <= nonce_signature.Capacity(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        nonce_signature.SetLength(nonce_signature_size);

        switch (SupplementaryTlvReader.GetType())
        {
        case TLV::kTLVType_ByteString:
        case TLV::kTLVType_UTF8String: {
            err = SupplementaryTlvReader.GetBytes(nonce_signature, static_cast<uint32_t>(nonce_signature_size));
            SuccessOrExit(err);
            break;
        }
        default:
            err = CHIP_ERROR_INVALID_ARGUMENT;
            ChipLogProgress(
                AppServer,
                "kProtocol_OpCredentials Error parsing TLV, upper level blocks should not contain elements other than strings.");
            SuccessOrExit(err);
        }

        if (CHIP_NO_ERROR ==
            (err = mRemoteManufacturerPubKey.ECDSA_validate_msg_signature(mNonce, sizeof(mNonce), nonce_signature)))
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Device attestation succeeded.");
        else
            ChipLogError(AppServer, "kProtocol_OpCredentials Device attestation failed.");
    }

exit:

    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerSendCSRRequest(Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter OpCredReqWriter;
    chip::TLV::TLVType outerType            = chip::TLV::kTLVType_NotSpecified;
    System::PacketBufferHandle OpCredReqBuf = chip::System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    if (OpCredReqBuf.IsNull())
    {
        ChipLogError(DataManagement, "kProtocol_OpCredentials Unable to allocate packet buffer");
    };

    OpCredReqWriter.Init(std::move(OpCredReqBuf));
    OpCredReqWriter.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, outerType);
    OpCredReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("Rq"), 2);
    OpCredReqWriter.EndContainer(outerType);
    OpCredReqWriter.Finalize(&OpCredReqBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    mExchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCredReqBuf), sendFlags);

    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerProcessCSR(System::PacketBufferHandle msgBuf,
                                                     Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(AppServer, "kProtocol_OpCredentials CSR received from the device.");
    System::PacketBufferTLVReader CsrReqReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    TLV::TLVType TlvContainerType = TLV::kTLVType_Structure;
    CsrReqReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = CsrReqReader.Next(TlvContainerType, TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = CsrReqReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kCSR),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding CSR tag.");
        SuccessOrExit(err);
    }
    else
    {
        mDerCsrLength = SupplementaryTlvReader.GetLength();
        if (CHIP_NO_ERROR != (err = SupplementaryTlvReader.DupBytes(mDeviceCSR, mDerCsrLength)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Failed to extract device certificate.");
            SuccessOrExit(err);
        }
    }
    // if (CHIP_NO_ERROR !=
    //     (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kOperationalID),
    //                                            SupplementaryTlvReader)))
    // {
    //     ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type OperationalID.");
    //     SuccessOrExit(err);
    // }
    // else
    // {
    //     mOperationalIDLength = SupplementaryTlvReader.GetLength();
    //     if (CHIP_NO_ERROR !=
    //         (err = SupplementaryTlvReader.DupBytes(reinterpret_cast<uint8_t *&>(mOperationalID), mOperationalIDLength)))
    //     {
    //         ChipLogProgress(AppServer, "kProtocol_OpCredentials Failed to extract device certificate.");
    //         SuccessOrExit(err);
    //     }
    // }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kCertDeclaration),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type CertDeclaration.");
        SuccessOrExit(err);
    }
    // if (CHIP_NO_ERROR !=
    //     (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kFirmMeasurements),
    //                                            SupplementaryTlvReader)))
    // {
    //     ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type FirmMeasurements.");
    //     SuccessOrExit(err);
    // }
    // TODO: Send all necessary data back to the Commissioner
    err = ObtainDeviceOpCert();
    SuccessOrExit(err);

    // TODO: Replace w/ platform specific sleep call
    sleep(2);

    err = ObtainCommissionerOpCert();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerSendRootCert(Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_CONFIG_NO_ERROR;
    System::PacketBufferTLVWriter OpCertReqWriter;
    chip::TLV::TLVType outerType            = chip::TLV::kTLVType_NotSpecified;
    System::PacketBufferHandle OpCertReqBuf = chip::System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    if (OpCertReqBuf.IsNull())
    {
        ChipLogError(DataManagement, "kProtocol_OpCredentials Unable to allocate packet buffer");
    };

    OpCertReqWriter.Init(std::move(OpCertReqBuf));
    OpCertReqWriter.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, outerType);

    OpCertReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("OpCert"), 6);

    OpCertReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kCertificate), mChipRootCertificate,
                             mChipRootCertificateLength);
    OpCertReqWriter.EndContainer(outerType);
    OpCertReqWriter.Finalize(&OpCertReqBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    mExchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCertReqBuf), sendFlags);

    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerSendCACert(Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_CONFIG_NO_ERROR;
    System::PacketBufferTLVWriter OpCertReqWriter;
    chip::TLV::TLVType outerType            = chip::TLV::kTLVType_NotSpecified;
    System::PacketBufferHandle OpCertReqBuf = chip::System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    if (OpCertReqBuf.IsNull())
    {
        ChipLogError(DataManagement, "kProtocol_OpCredentials Unable to allocate packet buffer");
    };

    OpCertReqWriter.Init(std::move(OpCertReqBuf));
    OpCertReqWriter.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, outerType);

    OpCertReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("OpCert"), 6);

    OpCertReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kCertificate), mChipCACertificate,
                             mChipCACertificateLength);
    OpCertReqWriter.EndContainer(outerType);
    OpCertReqWriter.Finalize(&OpCertReqBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    mExchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCertReqBuf), sendFlags);

    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerSendOpCert(Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_CONFIG_NO_ERROR;
    System::PacketBufferTLVWriter OpCertReqWriter;
    chip::TLV::TLVType outerType            = chip::TLV::kTLVType_NotSpecified;
    System::PacketBufferHandle OpCertReqBuf = chip::System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    if (OpCertReqBuf.IsNull())
    {
        ChipLogError(DataManagement, "Unable to allocate packet buffer");
    };

    OpCertReqWriter.Init(std::move(OpCertReqBuf));
    OpCertReqWriter.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, outerType);

    OpCertReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("OpCert"), 6);

    OpCertReqWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kCertificate), mChipDeviceCredentials,
                             mChipDeviceCredentialsLength);
    OpCertReqWriter.EndContainer(outerType);
    OpCertReqWriter.Finalize(&OpCertReqBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    mExchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCertReqBuf), sendFlags);

    mDeviceAttested = true;

    mExchangeContext->OnAttestedDevice(mExchangeContext->GetSecureSession(), mOpCredSet, mTrustedRootId);

    return err;
}

CHIP_ERROR DeviceAttestation::CommissionerSendACL(Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVWriter OpAclWriter;
    TLV::TLVType outerType = TLV::kTLVType_NotSpecified;
    uint64_t temporaryRandomValue;
    uint8_t randomListLength;
    static unsigned char digest[33] = "";
    P256ECDSASignature signature;
    ChipCertificateData certData;
    System::PacketBufferHandle OpAclBuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    hash_sha256.Clear();
    ReturnErrorOnFailure(hash_sha256.Begin());

    OpAclWriter.Init(std::move(OpAclBuf));
    OpAclWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerType);
    OpAclWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kMessageType),
                         reinterpret_cast<const uint8_t *>("Rq"), 2);
    OpAclWriter.StartContainer(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAcl), TLV::kTLVType_List, outerType);
    {
        ReturnErrorOnFailure(DRBG_get_bytes(reinterpret_cast<uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
        ReturnErrorOnFailure(
            hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
        OpAclWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclPrivilege),
                             reinterpret_cast<const uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue));

        ReturnErrorOnFailure(DRBG_get_bytes(reinterpret_cast<uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
        ReturnErrorOnFailure(
            hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
        OpAclWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclAuthMode),
                             reinterpret_cast<const uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue));
        OpAclWriter.StartContainer(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclTargets), TLV::kTLVType_List,
                                   outerType);
        {
            ReturnErrorOnFailure(DRBG_get_bytes(&randomListLength, 1));
            randomListLength &= 0xF;
            for (int i = 0; i < randomListLength; ++i)
            {
                ReturnErrorOnFailure(
                    DRBG_get_bytes(reinterpret_cast<uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
                ReturnErrorOnFailure(
                    hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
                OpAclWriter.Put(TLV::AnonymousTag, temporaryRandomValue, true);
            }
        }
        OpAclWriter.EndContainer(outerType);
        OpAclWriter.StartContainer(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclSubjects), TLV::kTLVType_List,
                                   outerType);
        {
            ReturnErrorOnFailure(DRBG_get_bytes(&randomListLength, 1));
            randomListLength &= 0xF;
            for (int i = 0; i < randomListLength; ++i)
            {
                ReturnErrorOnFailure(
                    DRBG_get_bytes(reinterpret_cast<uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
                ReturnErrorOnFailure(
                    hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
                OpAclWriter.Put(TLV::AnonymousTag, temporaryRandomValue, true);
            }
        }
        OpAclWriter.EndContainer(outerType);

        ReturnErrorOnFailure(DecodeChipCert(mChipDeviceCredentials, mChipDeviceCredentialsLength, certData));
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Operational certificate, Issuer is 0x%zx.",
                        certData.mIssuerDN.mAttrValue[0].mChipId);

        ReturnErrorOnFailure(hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&certData.mIssuerDN.mAttrValue[0].mChipId),
                                                 sizeof(certData.mIssuerDN.mAttrValue[0].mChipId)));
        OpAclWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclIssuer),
                             reinterpret_cast<const uint8_t *>(&certData.mIssuerDN.mAttrValue[0].mChipId),
                             sizeof(certData.mIssuerDN.mAttrValue[0].mChipId));
        hash_sha256.Finish(digest);

        mKeypair.ECDSA_sign_hash(digest, sizeof(digest) - 1, signature);
        OpAclWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclSignature), (signature),
                             sizeof(signature));
    }
    OpAclWriter.EndContainer(outerType);
    OpAclWriter.EndContainer(outerType);
    OpAclWriter.Finalize(&OpAclBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kFromInitiator).Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    mExchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpAclBuf), sendFlags);

    return err;
}
#else  // CONFIG_COMMISSIONER_ENABLED
CHIP_ERROR DeviceAttestation::DeviceProcessNonce(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader OpCredReqReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    TLV::TLVType TlvContainerType = TLV::kTLVType_Structure;
    OpCredReqReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = OpCredReqReader.Next(TlvContainerType, TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = OpCredReqReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = OpCredReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kMessageType),
                                                  SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = OpCredReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kNonce),
                                                  SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Nonce tag.");
        SuccessOrExit(err);
    }
    else
    {
        size_t nonce_size = SupplementaryTlvReader.GetLength();

        VerifyOrReturnError(nonce_size <= kNonceSize + 1, CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        switch (SupplementaryTlvReader.GetType())
        {
        case TLV::kTLVType_ByteString:
        case TLV::kTLVType_UTF8String: {
            ReturnErrorOnFailure(SupplementaryTlvReader.GetBytes(mNonce, static_cast<uint32_t>(nonce_size)));
            break;
        }
        default:
            ChipLogProgress(
                AppServer,
                "kProtocol_OpCredentials Error parsing TLV, upper level blocks should not contain elements other than strings.");
        }
        ChipLogProgress(AppServer, "Received device attestation request with nonce: \n%s", reinterpret_cast<char *>(mNonce));
        SuccessOrExit(err = (DeviceSendDAC(mNonce, nonce_size, exchangeContext)));
    }

exit:;
    return err;
}

CHIP_ERROR DeviceAttestation::DeviceProcessCSRRequest(System::PacketBufferHandle msgBuf,
                                                      Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    chip::TLV::TLVType outerType = chip::TLV::kTLVType_NotSpecified;
    System::PacketBufferTLVWriter OpCredRepWriter;
    Messaging::SendFlags sendFlags;
    System::PacketBufferHandle OpCredRepBuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    size_t csrLen                           = sizeof(mCSRBuffer);

    SuccessOrExit(mKeypairOperational.Initialize());
    SuccessOrExit(mKeypairOperational.NewCertificateSigningRequestDER(mCSRBuffer, csrLen));
    // TODO create proper Operational ID
    // static uint8_t nodeAddressStr[] = "Some text for representing nodeAdr";

    // TODO create proper Firmware Measurements
    // static uint8_t firmware_measurements[64] = "STUB_MEASUREMENTS";

    if (OpCredRepBuf.IsNull())
    {
        ChipLogError(DataManagement, "Unable to allocate packet buffer");
    };

    OpCredRepWriter.Init(std::move(OpCredRepBuf));
    OpCredRepWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerType);
    OpCredRepWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("Rp"), 2);

    OpCredRepWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kCSR), mCSRBuffer,
                             static_cast<uint32_t>(csrLen));

    // OpCredRepWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kOperationalID), nodeAddressStr,
    //                          static_cast<uint32_t>(sizeof(nodeAddressStr)));

    OpCredRepWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kCertDeclaration),
                             device_certificate_declaration, static_cast<uint32_t>(sizeof(device_certificate_declaration)));

    // OpCredRepWriter.PutBytes(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kFirmMeasurements),
    //     firmware_measurements, static_cast<uint32_t>(sizeof(firmware_measurements)));

    OpCredRepWriter.EndContainer(outerType);
    OpCredRepWriter.Finalize(&OpCredRepBuf);

    sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    exchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCredRepBuf), sendFlags);

exit:
    return err;
}

CHIP_ERROR DeviceAttestation::DeviceProcessRootCert(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(AppServer, "kProtocol_OpCredentials Obtained Operational Root Certificate.");
    System::PacketBufferTLVReader CsrReqReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    TLV::TLVType TlvContainerType = TLV::kTLVType_Structure;
    CsrReqReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = CsrReqReader.Next(TlvContainerType, TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = CsrReqReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kCertificate),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type Certificate.");
        SuccessOrExit(err);
    }
    else
    {
        // Load CHIP Root Certificate and push to OperationalCredentialSet
        mChipRootCertificateLength = SupplementaryTlvReader.GetLength();

        VerifyOrExit(mChipRootCertificateLength > 0, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        switch (SupplementaryTlvReader.GetType())
        {
        case TLV::kTLVType_ByteString:
        case TLV::kTLVType_UTF8String: {
            err = SupplementaryTlvReader.GetBytes(mChipRootCertificate, mChipRootCertificateLength);
            SuccessOrExit(err);
            break;
        }
        default:
            err = CHIP_ERROR_INVALID_ARGUMENT;
            ChipLogProgress(
                AppServer,
                "kProtocol_OpCredentials Error parsing TLV, upper level blocks should not contain elements other than strings.");
            SuccessOrExit(err);
        }

        err = mChipCertificateSet.LoadCert(mChipRootCertificate, sizeof(mChipRootCertificate),
                                           BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
        SuccessOrExit(err);

        mTrustedRootId.mId  = mChipCertificateSet.GetLastCert()->mAuthKeyId.mId;
        mTrustedRootId.mLen = mChipCertificateSet.GetLastCert()->mAuthKeyId.mLen;
    }

exit:
    return err;
}

CHIP_ERROR DeviceAttestation::DeviceProcessCACert(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(AppServer, "kProtocol_OpCredentials Obtained Operational CA Certificate.");
    System::PacketBufferTLVReader CsrReqReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    TLV::TLVType TlvContainerType = TLV::kTLVType_Structure;
    CsrReqReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = CsrReqReader.Next(TlvContainerType, TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = CsrReqReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = CsrReqReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kCertificate),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type Certificate.");
        SuccessOrExit(err);
    }
    else
    {
        // Load CHIP CA Certificate and push to OperationalCredentialSet
        mChipCACertificateLength = SupplementaryTlvReader.GetLength();

        VerifyOrExit(mChipCACertificateLength > 0, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        switch (SupplementaryTlvReader.GetType())
        {
        case TLV::kTLVType_ByteString:
        case TLV::kTLVType_UTF8String: {
            err = SupplementaryTlvReader.GetBytes(mChipCACertificate, mChipCACertificateLength);
            SuccessOrExit(err);
            break;
        }
        default:
            err = CHIP_ERROR_INVALID_ARGUMENT;
            ChipLogProgress(
                AppServer,
                "kProtocol_OpCredentials Error parsing TLV, upper level blocks should not contain elements other than strings.");
            SuccessOrExit(err);
        }

        err = mChipCertificateSet.LoadCert(mChipCACertificate, mChipCACertificateLength,
                                           BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR DeviceAttestation::DeviceProcessOpCert(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(AppServer, "kProtocol_OpCredentials Obtained Device Operational Credential.");
    System::PacketBufferTLVReader OpCertReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    TLV::TLVType TlvContainerType = TLV::kTLVType_Structure;
    OpCertReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = OpCertReader.Next(TlvContainerType, TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = OpCertReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = OpCertReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = OpCertReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kCertificate),
                                               SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type Certificate.");
        SuccessOrExit(err);
    }
    else
    {
        // Load CHIP Device Certificate and push to OperationalCredentialSet
        mChipDeviceCredentialsLength = SupplementaryTlvReader.GetLength();

        VerifyOrExit(mChipDeviceCredentialsLength > 0, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

        switch (SupplementaryTlvReader.GetType())
        {
        case TLV::kTLVType_ByteString:
        case TLV::kTLVType_UTF8String: {
            err = SupplementaryTlvReader.GetBytes(mChipDeviceCredentials, mChipDeviceCredentialsLength);
            SuccessOrExit(err);
            break;
        }
        default:
            err = CHIP_ERROR_INVALID_ARGUMENT;
            ChipLogProgress(
                AppServer,
                "kProtocol_OpCredentials Error parsing TLV, upper level blocks should not contain elements other than strings.");
            SuccessOrExit(err);
        }

        err = mChipCertificateSet.LoadCert(mChipDeviceCredentials, mChipDeviceCredentialsLength,
                                           BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
        SuccessOrExit(err);

        err = SetEffectiveTime();
        SuccessOrExit(err);

        err = mChipCertificateSet.ValidateCert(mChipCertificateSet.GetLastCert(), mValidContext);
        SuccessOrExit(err);

        mOpCredSet->LoadCertSet(&mChipCertificateSet);
        mOpCredSet->SetDevOpCred(mTrustedRootId, mChipDeviceCredentials, static_cast<uint16_t>(mChipDeviceCredentialsLength));
        mOpCredSet->SetDevOpCredKeypair(mTrustedRootId, &mKeypairOperational);

        SuccessOrExit(err);
    }

    mDeviceAttested = true;

    exchangeContext->OnAttestedDevice(exchangeContext->GetSecureSession(), mOpCredSet, mTrustedRootId);

exit:
    return err;
}

CHIP_ERROR DeviceAttestation::DeviceSendDAC(const uint8_t * nonce, size_t nonce_size, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    P256ECDSASignature nonce_signature;
    System::PacketBufferTLVWriter OpCredRepWriter;
    TLV::TLVType outerType                  = TLV::kTLVType_NotSpecified;
    System::PacketBufferHandle OpCredRepBuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);

    OpCredRepWriter.Init(std::move(OpCredRepBuf));
    OpCredRepWriter.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, outerType);
    OpCredRepWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kMessageType),
                             reinterpret_cast<const uint8_t *>("Rp"), 2);
    OpCredRepWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kCertificate),
                             device_manufacturer_certificate, sizeof(device_manufacturer_certificate));

    ReturnErrorOnFailure(mKeypair.ECDSA_sign_msg(nonce, nonce_size, nonce_signature));
    ReturnErrorOnFailure(OpCredRepWriter.PutBytes(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kNonce),
                                                  nonce_signature, static_cast<uint32_t>(nonce_signature.Length())));

    OpCredRepWriter.EndContainer(outerType);
    OpCredRepWriter.Finalize(&OpCredRepBuf);

    Messaging::SendFlags sendFlags;
    sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck);
    exchangeContext->SendMessage(chip::Protocols::OpCredentials::Id, 0, std::move(OpCredRepBuf), sendFlags);

    return err;
}

CHIP_ERROR DeviceAttestation::DeviceProcessACL(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(AppServer, "kProtocol_OpCredentials ACL received from the commissioner.");
    System::PacketBufferTLVReader TopLevelTLVReader;
    System::PacketBufferTLVReader AclTlvReader;
    System::PacketBufferTLVReader SupplementaryTlvReader;
    TLV::TLVType TlvContainerType = TLV::kTLVType_Structure;
    uint64_t temporaryValue;
    static unsigned char digest[33] = "";

    TopLevelTLVReader.Init(std::move(msgBuf));
    if (CHIP_NO_ERROR != (err = TopLevelTLVReader.Next(TlvContainerType, TLV::AnonymousTag)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing top level TLV.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR != (err = TopLevelTLVReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV container.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = TopLevelTLVReader.FindElementWithTag(static_cast<uint64_t>(chip::Transport::DeviceAttestationTLVTag::kMessageType),
                                                    SupplementaryTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding Message Type tag.");
        SuccessOrExit(err);
    }
    if (CHIP_NO_ERROR !=
        (err = TopLevelTLVReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAcl), AclTlvReader)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL.");
        SuccessOrExit(err);
    }
    TlvContainerType = TLV::kTLVType_List;
    if (CHIP_NO_ERROR != (err = AclTlvReader.EnterContainer(TlvContainerType)))
    {
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV ACL container.");
        SuccessOrExit(err);
    }
    else
    {
        hash_sha256.Clear();
        ReturnErrorOnFailure(hash_sha256.Begin());

        if (CHIP_NO_ERROR !=
            (err = AclTlvReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclPrivilege),
                                                   SupplementaryTlvReader)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL Privilege.");
            SuccessOrExit(err);
        }
        SupplementaryTlvReader.Get(temporaryValue);
        ReturnErrorOnFailure(hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryValue), sizeof(temporaryValue)));

        if (CHIP_NO_ERROR !=
            (err = AclTlvReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclAuthMode),
                                                   SupplementaryTlvReader)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL AuthMode.");
            SuccessOrExit(err);
        }
        SupplementaryTlvReader.Get(temporaryValue);
        ReturnErrorOnFailure(hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryValue), sizeof(temporaryValue)));

        if (CHIP_NO_ERROR !=
            (err = AclTlvReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclTargets),
                                                   SupplementaryTlvReader)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL Targets.");
            SuccessOrExit(err);
        }
        if (CHIP_NO_ERROR != (err = SupplementaryTlvReader.EnterContainer(TlvContainerType)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV ACL Targets container.");
            SuccessOrExit(err);
        }
        else
        {
            while ((err = SupplementaryTlvReader.Next()) == CHIP_NO_ERROR)
            {
                SupplementaryTlvReader.Get(temporaryValue);
                ReturnErrorOnFailure(
                    hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryValue), sizeof(temporaryValue)));
            }
            ReturnErrorOnFailure(SupplementaryTlvReader.ExitContainer(TlvContainerType));
        }

        if (CHIP_NO_ERROR !=
            (err = AclTlvReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclSubjects),
                                                   SupplementaryTlvReader)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL Targets.");
            SuccessOrExit(err);
        }
        if (CHIP_NO_ERROR != (err = SupplementaryTlvReader.EnterContainer(TlvContainerType)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error entering TLV ACL Targets container.");
            SuccessOrExit(err);
        }
        else
        {
            while ((err = SupplementaryTlvReader.Next()) == CHIP_NO_ERROR)
            {
                SupplementaryTlvReader.Get(temporaryValue);
                ReturnErrorOnFailure(
                    hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryValue), sizeof(temporaryValue)));
            }
            ReturnErrorOnFailure(SupplementaryTlvReader.ExitContainer(TlvContainerType));
        }

        if (CHIP_NO_ERROR !=
            (err = AclTlvReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclIssuer),
                                                   SupplementaryTlvReader)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL Issuer.");
            SuccessOrExit(err);
        }
        SupplementaryTlvReader.Get(temporaryValue);
        ReturnErrorOnFailure(hash_sha256.AddData(reinterpret_cast<const uint8_t *>(&temporaryValue), sizeof(temporaryValue)));

        if (CHIP_NO_ERROR !=
            (err = AclTlvReader.FindElementWithTag(static_cast<uint64_t>(Transport::DeviceAttestationTLVTag::kAclSignature),
                                                   SupplementaryTlvReader)))
        {
            ChipLogProgress(AppServer, "kProtocol_OpCredentials Error finding ACL Issuer.");
            SuccessOrExit(err);
        }
        else
        {
            P256ECDSASignature tlvSignature;

            size_t tlvSignatureSize = SupplementaryTlvReader.GetLength();

            VerifyOrExit(tlvSignatureSize <= tlvSignature.Capacity(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

            tlvSignature.SetLength(tlvSignatureSize);

            switch (SupplementaryTlvReader.GetType())
            {
            case TLV::kTLVType_ByteString:
            case TLV::kTLVType_UTF8String: {
                err = SupplementaryTlvReader.GetBytes(tlvSignature, static_cast<uint32_t>(tlvSignatureSize));
                SuccessOrExit(err);
                break;
            }
            default:
                err = CHIP_ERROR_INVALID_ARGUMENT;
                ChipLogProgress(AppServer, "kProtocol_OpCredentials Error parsing TLV, ACL signature should be string.");
                SuccessOrExit(err);
            }

            hash_sha256.Finish(digest);

            err = mRemoteManufacturerPubKey.ECDSA_validate_msg_signature(digest, sizeof(digest), tlvSignature);
        }
        ReturnErrorOnFailure(AclTlvReader.ExitContainer(TlvContainerType));
    }

exit:

    return err;
}
#endif // CONFIG_COMMISSIONER_ENABLED

#ifdef CONFIG_COMMISSIONER_ENABLED
void DeviceAttestation::GetTimestampForCertifying()
{
    int64_t ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    sprintf(mTimestamp, "%" PRId64, ms);
}

http_response * DeviceAttestation::DoHttpExchange(uint8_t * buffer, CERTIFIER * certifier)
{
    static char auth_header[VERY_LARGE_STRING_SIZE * 4] = "";
    static char tracking_header[LARGE_STRING_SIZE]      = "";
    static char source_header[SMALL_STRING_SIZE]        = "";
    const char * tracking_id  = reinterpret_cast<const char *>(certifier_api_easy_get_opt(mCertifier, CERTIFIER_OPT_TRACKING_ID));
    const char * bearer_token = reinterpret_cast<const char *>(certifier_api_easy_get_opt(mCertifier, CERTIFIER_OPT_CRT));
    const char * source       = reinterpret_cast<const char *>(certifier_api_easy_get_opt(mCertifier, CERTIFIER_OPT_SOURCE));
    const char * certifier_url =
        reinterpret_cast<const char *>(certifier_api_easy_get_opt(mCertifier, CERTIFIER_OPT_CERTIFIER_URL));

    if (bearer_token != nullptr)
    {
        snprintf(auth_header, VERY_LARGE_STRING_SIZE * 4, "Authorization: Bearer %s", bearer_token);
    }
    snprintf(tracking_header, SMALL_STRING_SIZE, "x-xpki-tracking-id: %s", tracking_id);
    snprintf(source_header, SMALL_STRING_SIZE, "x-xpki-source: %s", source);

    const char * headers[] = { "Accept: application/json",
                               "Content-Type: application/json; charset=utf-8",
                               auth_header,
                               tracking_header,
                               source_header,
                               nullptr };
    return certifier_api_easy_http_post(certifier, certifier_url, headers, (const char *) (buffer));
}

CHIP_ERROR
DeviceAttestation::ObtainOpCert(uint8_t * derCertificate, uint32_t derCertificateLength, uint8_t * derCsr, uint32_t derCsrLength,
                                char *& pkcs7OpCert)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    static uint8_t cert_id[] = "X509";
    static char authCert[]   = "libcertifier-cert.crt";

    JSON_Value * root_value   = json_value_init_object();
    JSON_Object * root_object = json_value_get_object(root_value);

    size_t base64CertificateLength            = static_cast<size_t>(base64_encode_len(static_cast<int>(derCertificateLength) + 2));
    std::unique_ptr<char[]> base64Certificate = std::make_unique<char[]>(base64CertificateLength);
    std::unique_ptr<char[]> base64JsonCrt;

    static char signatureBase64[200] = {};
    int signatureLength              = 200;
    P256ECDSASignature signature;
    static unsigned char digest[33] = "";

    char * jsonCSR                    = nullptr;
    size_t base64CSRLength            = static_cast<size_t>(base64_encode_len(static_cast<int>(derCsrLength))) + 1;
    std::unique_ptr<char[]> base64CSR = std::make_unique<char[]>(base64CSRLength);
    std::unique_ptr<char[]> jsonCSRuptr(jsonCSR);
    char operationalID[17];
    std::unique_ptr<char[]> operationalIDup;
    uint64_t temporaryRandomValue;

    http_response * resp                = nullptr;
    const char * OpCertificateChainTemp = nullptr;

    pkcs7OpCert = nullptr;
    memset(operationalID, 0, sizeof(operationalID));
    ReturnErrorOnFailure(DRBG_get_bytes(reinterpret_cast<uint8_t *>(&temporaryRandomValue), sizeof(temporaryRandomValue)));
    snprintf(operationalID, sizeof(operationalID), "%016" PRIX64, temporaryRandomValue);

    json_object_set_string(root_object, "tokenType", "X509");
    base64_encode(base64Certificate.get(), derCertificate, static_cast<int>(derCertificateLength));
    json_object_set_string(root_object, "certificate", base64Certificate.get());
    GetTimestampForCertifying();
    json_object_set_string(root_object, "timestamp", mTimestamp);
    json_object_set_string(root_object, "nonce", reinterpret_cast<char *>(mNonce));

    hash_sha256.Clear();
    hash_sha256.Begin();
    hash_sha256.AddData(derCertificate, derCertificateLength);
    hash_sha256.AddData(reinterpret_cast<uint8_t *>(mTimestamp), strlen(mTimestamp));
    hash_sha256.AddData(reinterpret_cast<uint8_t *>(mNonce), sizeof(mNonce) - 1);
    hash_sha256.AddData(cert_id, sizeof(cert_id) - 1);
    hash_sha256.Finish(digest);

    err = mKeypair.ECDSA_sign_hash(digest, sizeof(digest) - 1, signature);
    SuccessOrExit(err);
    if (0 >= (base64_encode_len(static_cast<int>(signature.Length() + 1))))
    {
        err = CHIP_CONFIG_ERROR_MIN;
        ChipLogError(AppServer, "kProtocol_OpCredentials Error creating signature.");
        SuccessOrExit(err);
    }
    signatureLength = static_cast<int>(signature.Length());
    base64_encode(signatureBase64, signature, signatureLength);
    json_object_set_string(root_object, "signature", signatureBase64);

    mJsonCrt = json_serialize_to_string_pretty(root_value);
    ChipLogProgress(AppServer, "X509 JSON certificate for obtaining operational credentials: \n%s", mJsonCrt);
    base64JsonCrt = std::make_unique<char[]>(static_cast<size_t>(base64_encode_len(static_cast<int>(strlen(mJsonCrt)))));
    base64_encode(base64JsonCrt.get(), reinterpret_cast<const unsigned char *>(mJsonCrt), static_cast<int>(strlen(mJsonCrt)));
    mCertifier = certifier_api_easy_new();
    certifier_api_easy_set_opt(mCertifier, CERTIFIER_OPT_CRT, base64JsonCrt.get());
    certifier_api_easy_set_opt(mCertifier, CERTIFIER_OPT_CN_PREFIX, operationalID);

    base64_encode(base64CSR.get(), reinterpret_cast<const unsigned char *>(derCsr), static_cast<int>(derCsrLength));
    if (!(certifier_api_easy_create_json_csr(mCertifier, reinterpret_cast<unsigned char *>(base64CSR.get()), (char *) operationalID,
                                             &jsonCSR)))
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials Error creating JSON CSR.");
        SuccessOrExit(err);
    }
    ChipLogProgress(AppServer, "CSR for: \n%s", jsonCSR);

    certifier_api_easy_set_opt(mCertifier, CERTIFIER_OPT_CA_INFO, reinterpret_cast<void *>(authCert));

    ChipLogProgress(AppServer, "kProtocol_OpCredentials Obtaining operational credentials.");
    if (nullptr == (resp = DoHttpExchange(reinterpret_cast<uint8_t *>(jsonCSR), mCertifier)))
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials Error obtaining HTTP response.");
        err = CHIP_ERROR_STATUS_REPORT_RECEIVED;
        SuccessOrExit(err);
    }
    if ((resp->error != 0) || (resp->payload == nullptr))
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials Error in HTTP response:\n%s",
                     util_format_curl_error("certifiercommissioner_request_x509_certificate", resp->http_code, resp->error,
                                            resp->error_msg, resp->payload, __FILE__, __LINE__));
        err = CHIP_ERROR_STATUS_REPORT_RECEIVED;
        SuccessOrExit(err);
    }

    json_value_free(root_value);
    if (json_value_get_type(root_value = json_parse_string_with_comments(resp->payload)) != JSONObject)
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials Error parsing HTTP response JSON.\n%s",
                     util_format_curl_error("certifiercommissioner_request_x509_certificate", resp->http_code, resp->error,
                                            "Could not parse JSON.  Expected it to be an array.", resp->payload, __FILE__,
                                            __LINE__));
        SuccessOrExit(err);
    }

    if (nullptr == (root_object = json_value_get_object(root_value)))
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials Error parsing HTTP response JSON object.\n%s",
                     util_format_curl_error("certifiercommissioner_request_x509_certificate", resp->http_code, resp->error,
                                            "Could not parse JSON.  parsed_json_object_value is NULL!.", resp->payload, __FILE__,
                                            __LINE__));
        SuccessOrExit(err);
    }

    if (nullptr == (OpCertificateChainTemp = (json_object_get_string(root_object, "certificateChain"))))
    {
        ChipLogError(AppServer, "kProtocol_OpCredentials Error obtaining certificate chain from HTTP response JSON.\n%s",
                     util_format_curl_error("certifiercommissioner_request_x509_certificate", resp->http_code, resp->error,
                                            "Could not parse JSON.  certificate_chain is NULL!", resp->payload, __FILE__,
                                            __LINE__));
        SuccessOrExit(err);
    }

    pkcs7OpCert = static_cast<char *>(chip::Platform::MemoryAlloc(strlen(OpCertificateChainTemp) + 1));
    strcpy(pkcs7OpCert, OpCertificateChainTemp);

exit:
    json_value_free(root_value);
    return err;
}

CHIP_ERROR
DeviceAttestation::ObtainDeviceOpCert()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char * OpCertificateChainTemp = nullptr;
    std::unique_ptr<char[]> OpCertificateChain;

    SuccessOrExit(ObtainOpCert(mDerDeviceCertificate, mDerCertificateLength, mDeviceCSR, mDerCsrLength, OpCertificateChainTemp));

    OpCertificateChain = std::make_unique<char[]>(strlen(OpCertificateChainTemp) + 1);
    strcpy(OpCertificateChain.get(), OpCertificateChainTemp);
    util_trim(OpCertificateChain.get());
    ChipLogDetail(AppServer, "kProtocol_OpCredentials Device operational credentials from the certifier", OpCertificateChain.get());

    {
        X509DerCertificate deviceCertificate[3];
        uint32_t n_certs;

        err = GetNumberOfCertsFromPKCS7(reinterpret_cast<uint8_t *>(OpCertificateChain.get()), &n_certs);
        SuccessOrExit(err);

        VerifyOrExit(n_certs >= 3, err = CHIP_ERROR_CERT_NOT_FOUND);

        err = LoadCertsFromPKCS7(reinterpret_cast<uint8_t *>(OpCertificateChain.get()), &deviceCertificate[0], &n_certs);
        SuccessOrExit(err);

        err = ConvertX509CertToChipCert(deviceCertificate[0], static_cast<uint32_t>(deviceCertificate[0].Length()),
                                        mChipDeviceCredentials, sizeof(mChipDeviceCredentials), mChipDeviceCredentialsLength);
        SuccessOrExit(err);

        err = ConvertX509CertToChipCert(deviceCertificate[1], static_cast<uint32_t>(deviceCertificate[1].Length()),
                                        mChipCACertificate, sizeof(mChipCACertificate), mChipCACertificateLength);
        SuccessOrExit(err);

        err = ConvertX509CertToChipCert(deviceCertificate[2], static_cast<uint32_t>(deviceCertificate[2].Length()),
                                        mChipRootCertificate, sizeof(mChipRootCertificate), mChipRootCertificateLength);
        SuccessOrExit(err);
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Obtained Device Operational Credentials from the certifier.");
    }

exit:
    return err;
}

CHIP_ERROR
DeviceAttestation::ObtainCommissionerOpCert()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    char * OpCertificateChainTemp = nullptr;
    std::unique_ptr<char[]> OpCertificateChain;
    System::PacketBufferHandle OpCredRepBuf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    size_t csrLen                           = sizeof(mCSRBuffer);

    SuccessOrExit(mKeypairOperational.Initialize());
    SuccessOrExit(mKeypairOperational.NewCertificateSigningRequestDER(mCSRBuffer, csrLen));

    SuccessOrExit(ObtainOpCert(const_cast<uint8_t *>(device_manufacturer_certificate), sizeof(device_manufacturer_certificate),
                               mCSRBuffer, static_cast<uint32_t>(csrLen), OpCertificateChainTemp));

    OpCertificateChain = std::make_unique<char[]>(strlen(OpCertificateChainTemp) + 1);
    strcpy(OpCertificateChain.get(), OpCertificateChainTemp);
    util_trim(OpCertificateChain.get());
    ChipLogDetail(AppServer, "kProtocol_OpCredentials Commissioner operational credentials from the certifier",
                  OpCertificateChain.get());

    {
        X509DerCertificate commissionerCertificate[3];
        uint32_t n_certs;

        err = GetNumberOfCertsFromPKCS7(reinterpret_cast<uint8_t *>(OpCertificateChain.get()), &n_certs);
        SuccessOrExit(err);

        VerifyOrExit(n_certs >= 3, err = CHIP_ERROR_CERT_NOT_FOUND);

        err = LoadCertsFromPKCS7(reinterpret_cast<uint8_t *>(OpCertificateChain.get()), &commissionerCertificate[0], &n_certs);
        SuccessOrExit(err);

        err = ConvertX509CertToChipCert(commissionerCertificate[0], static_cast<uint32_t>(commissionerCertificate[0].Length()),
                                        mChipCommissionerCredentials, sizeof(mChipCommissionerCredentials),
                                        mChipCommissionerCredentialsLength);
        SuccessOrExit(err);

        err = ConvertX509CertToChipCert(commissionerCertificate[1], static_cast<uint32_t>(commissionerCertificate[1].Length()),
                                        mChipCommissionerCACertificate, sizeof(mChipCommissionerCACertificate),
                                        mChipCommissionerCACertificateLength);
        SuccessOrExit(err);

        err = ConvertX509CertToChipCert(commissionerCertificate[2], static_cast<uint32_t>(commissionerCertificate[2].Length()),
                                        mChipCommissionerRootCertificate, sizeof(mChipCommissionerRootCertificate),
                                        mChipCommissionerRootCertificateLength);
        SuccessOrExit(err);

        err = mChipCertificateSet.LoadCert(mChipCommissionerRootCertificate, mChipCommissionerRootCertificateLength,
                                           BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
        SuccessOrExit(err);

        mTrustedRootId.mId  = mChipCertificateSet.GetLastCert()->mAuthKeyId.mId;
        mTrustedRootId.mLen = mChipCertificateSet.GetLastCert()->mAuthKeyId.mLen;

        err = mChipCertificateSet.LoadCert(mChipCommissionerCACertificate, mChipCommissionerCACertificateLength,
                                           BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
        SuccessOrExit(err);

        mOpCredSet->LoadCertSet(&mChipCertificateSet);
        ReturnErrorOnFailure(mOpCredSet->SetDevOpCred(mTrustedRootId, mChipCommissionerCredentials,
                                                      static_cast<uint16_t>(mChipCommissionerCredentialsLength)));
        ReturnErrorOnFailure(mOpCredSet->SetDevOpCredKeypair(mTrustedRootId, &mKeypairOperational));
        ChipLogProgress(AppServer, "kProtocol_OpCredentials Obtained Commissioner Operational Credentials from the certifier.");
    }

exit:
    return err;
}
#endif // CONFIG_COMMISSIONER_ENABLED

// PLACEHOLDER - TODO: IMPLEMENT THIS
CHIP_ERROR DeviceAttestation::SetEffectiveTime(void)
{
    using namespace ASN1;
    ASN1UniversalTime effectiveTime;

    effectiveTime.Year   = 2021;
    effectiveTime.Month  = 4;
    effectiveTime.Day    = 30;
    effectiveTime.Hour   = 10;
    effectiveTime.Minute = 10;
    effectiveTime.Second = 10;

    return ASN1ToChipEpochTime(effectiveTime, mValidContext.mEffectiveTime);
}

} // namespace chip
