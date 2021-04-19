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
#pragma once

#include <credentials/CHIPOperationalCredentials.h>
#include <crypto/CHIPCryptoPAL.h>
#include <support/ReturnMacros.h>

#include "transport/SecureSessionMgr.h"

#include <chrono>
#include <memory>

struct CERTIFIER;
struct http_response;

namespace chip {
namespace Transport {

using namespace Credentials;

/**
 * \brief
 * A list of registered packet types a.k.a. TLV context-specific tags to be transferred between device and commissioner.
 */
enum class DeviceAttestationTLVTag
{
    /*! \brief Tag 0. Default stub and end of transmission signal.*/
    kUnknown = 0,
    /*! \brief Tag 1. The packet contains Message Type.*/
    kMessageType = 1,
    /*! \brief Tag 2. The packet contains Certificate Declaration.*/
    kCertDeclaration = 2,
    /*! \brief Tag 3. The packet contains DER Firmware Measurements.*/
    kFirmMeasurements = 3,
    /*! \brief Tag 4. The packet contains Certificate Signing Request from the device.*/
    kCSR = 4,
    /*! \brief Tag 5. The packet contains a certificate, the meaning on device and commissioner differ.
     * The device sends its manufacturer certificate with this packet type.
     * The commissioner sends operational certificate to the device with this packet type.
     * */
    kCertificate = 5,
    /*! \brief Tag 6. The packet contains a nonce value that a commissioner sends to a device.*/
    kNonce = 6,
    /*! \brief Tag 7. The packet contains timestamp. */
    kTimestamp = 7,
    /*! \brief Tag 8. The packet contains Operational ID (a.k.a. node address). */
    kOperationalID = 8,
    /*! \brief Tag 9. The packet contains ACL from the commissioner.*/
    kAcl = 9,
    /*! \brief Tag 10. Vendor Reserved.*/
    kVendorReserved0 = 10,
    /*! \brief Tag 11. Vendor Reserved.*/
    kVendorReserved1 = 11,
    /*! \brief Tag 12. Vendor Reserved.*/
    kVendorReserved2 = 12,
    /*! \brief Tag 13. Vendor Reserved.*/
    kVendorReserved3 = 13,
    /*! \brief Tag 14. The packet contains signature of all other fields that were sent from the device.*/
    kOperationalKeySignature = 14,
    /*! \brief Tag 15. The packet contains signature of the TLV fields that were sent to the device.*/
    kAttestationKeySignature = 15,
    /*! \brief Tag 0x85. The packet contains ACL privilege field.*/
    kAclPrivilege = 0x85,
    /*! \brief Tag 0x86. The packet contains ACL targets field.*/
    kAclTargets = 0x86,
    /*! \brief Tag 0x87. The packet contains ACL subjects field.*/
    kAclSubjects = 0x87,
    /*! \brief Tag 0x88. The packet contains ACL authorization mode field.*/
    kAclAuthMode = 0x88,
    /*! \brief Tag 0x89. The packet contains ACL issuer ext list.*/
    kAclIssuer = 0x89,
    /*! \brief Tag 0x8A. The packet contains ACL signature ext field.*/
    kAclSignature = 0x8A,
};

enum class DeviceAttestationState
{
    kDeviceWaiting,
    kCommissionerNonceSent,
    kDeviceDACSent,
    kCommissionerCSRRequsted,
    kDeviceCSRSent,
    kDeviceRootCertProcessed,
    kDeviceCACertProcessed,
    kCommissinerOpCertSent,
    kCommissinerOpCertProcessed,
    kFinished
};

class DeviceAttestation
{
public:
    DeviceAttestation()                          = default;
    DeviceAttestation(DeviceAttestation &&)      = delete;
    DeviceAttestation(const DeviceAttestation &) = delete;
    DeviceAttestation & operator=(const DeviceAttestation &) = delete;
    DeviceAttestation & operator=(DeviceAttestation &&) = delete;

    ~DeviceAttestation() { Close(); }

    CHIP_ERROR Init(Messaging::ExchangeContext * exchangeContext, OperationalCredentialSet * opCredSet);
    void Close();

    CHIP_ERROR StartAttestation();

    CHIP_ERROR OnMessageReceived(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);

    bool IsDeviceAttested() const { return mDeviceAttested; }

private:
    static constexpr size_t kNonceSize = 32;
    DeviceAttestationState mState;
    Messaging::ExchangeContext * mExchangeContext;
    uint8_t mNonce[kNonceSize + 1] = { 'b', 'g', 'v', 'y', 'a', 'K', 'B', 'h', 'y', 'T', 'F', '4', 'G', '6', '7', 'x',
                                       'B', 'p', 'W', 'U', 't', 'g', 'd', 'd', 'T', 'v', 'y', 'x', 'x', 'U', 'y', '1' };
#ifdef CONFIG_COMMISSIONER_ENABLED
    char mTimestamp[21] = "";

    uint32_t mDerCertificateLength      = 0;
    uint32_t mDerCsrLength              = 0;
    uint32_t mOperationalIDLength       = 0;
    uint8_t mDerDeviceCertificate[1024] = {};
    uint8_t * mDeviceCSR                = nullptr;
    char * mOperationalID               = nullptr;
    char * mJsonCrt                     = nullptr;
    uint8_t * mJsonCsr                  = nullptr;
#endif // CONFIG_COMMISSIONER_ENABLED
    Hash_SHA256_stream hash_sha256;
#ifdef CONFIG_COMMISSIONER_ENABLED
    CERTIFIER * mCertifier = nullptr;
#endif // CONFIG_COMMISSIONER_ENABLED
    P256Keypair mKeypair;
    P256Keypair mKeypairOperational;
    uint8_t mCSRBuffer[512];
    P256PublicKey mRemoteManufacturerPubKey;
    ChipCertificateSet mChipCertificateSet;
    CertificateKeyId mTrustedRootId;
    OperationalCredentialSet * mOpCredSet;
    ValidationContext mValidContext;
    uint8_t mChipDeviceCredentials[1024];
    uint32_t mChipDeviceCredentialsLength;
    uint8_t mChipCACertificate[1024];
    uint32_t mChipCACertificateLength;
    uint8_t mChipRootCertificate[1024];
    uint32_t mChipRootCertificateLength;
#ifdef CONFIG_COMMISSIONER_ENABLED
    uint8_t mChipCommissionerCredentials[1024];
    uint32_t mChipCommissionerCredentialsLength;
    uint8_t mChipCommissionerCACertificate[1024];
    uint32_t mChipCommissionerCACertificateLength;
    uint8_t mChipCommissionerRootCertificate[1024];
    uint32_t mChipCommissionerRootCertificateLength;
    char mAuthCertificate[1024] = "libcertifier-cert.crt";
    char mCertifierCfg[1024]    = "libcertifier.cfg";
#endif // CONFIG_COMMISSIONER_ENABLED

    bool mDeviceAttested = false;

#ifdef CONFIG_COMMISSIONER_ENABLED
    CHIP_ERROR CommissionerSendNonce();
    CHIP_ERROR CommissionerValidateNonceResponse(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR CommissionerSendCSRRequest(Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR CommissionerProcessCSR(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR CommissionerSendRootCert(Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR CommissionerSendCACert(Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR CommissionerSendOpCert(Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR CommissionerSendACL(Messaging::ExchangeContext * exchangeContext);
#else  // CONFIG_COMMISSIONER_ENABLED
    CHIP_ERROR DeviceProcessNonce(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR DeviceSendDAC(const uint8_t * nonce, size_t nonce_size, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR DeviceProcessCSRRequest(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR DeviceProcessRootCert(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR DeviceProcessCACert(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR DeviceProcessOpCert(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
    CHIP_ERROR DeviceProcessACL(System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);
#endif // CONFIG_COMMISSIONER_ENABLED
#ifdef CONFIG_COMMISSIONER_ENABLED
    void GetTimestampForCertifying();
    http_response * DoHttpExchange(uint8_t * buffer, CERTIFIER * certifier);
    CHIP_ERROR ObtainDeviceOpCert();
    CHIP_ERROR ObtainCommissionerOpCert();
    CHIP_ERROR ObtainOpCert(uint8_t * derCertificate, uint32_t derCertificateLength, uint8_t * derCsr, uint32_t derCsrLength,
                            char *& pkcs7OpCert);
#endif // CONFIG_COMMISSIONER_ENABLED

    // TODO
    CHIP_ERROR SetEffectiveTime(void);
};

} // namespace Transport
} // namespace chip
