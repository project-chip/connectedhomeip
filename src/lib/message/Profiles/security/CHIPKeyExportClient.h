/*
 *
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      Supporting classes for acting as a client in the key export protocol.
 *
 */

#ifndef CHIPKEYEXPORTCLIENT_H_
#define CHIPKEYEXPORTCLIENT_H_

#include "CHIPKeyExport.h"
#include <core/CHIPCore.h>

namespace chip {
namespace Profiles {
namespace Security {
namespace KeyExport {

/**
 * @brief
 *   Implements the client side of the CHIP key export protocol for use in stand-alone
 *   (non-CHIP messaging) contexts.
 */
class ChipStandAloneKeyExportClient : private ChipKeyExportDelegate
{
public:
    void Init(void);
    void Reset(void);

    CHIP_ERROR GenerateKeyExportRequest(uint32_t keyId, uint64_t responderNodeId, const uint8_t * clientCert,
                                        uint16_t clientCertLen, const uint8_t * clientKey, uint16_t clientKeyLen, uint8_t * reqBuf,
                                        uint16_t reqBufSize, uint16_t & reqLen);

    CHIP_ERROR GenerateKeyExportRequest(uint32_t keyId, uint64_t responderNodeId, const uint8_t * accessToken,
                                        uint16_t accessTokenLen, uint8_t * reqBuf, uint16_t reqBufSize, uint16_t & reqLen);

    CHIP_ERROR ProcessKeyExportResponse(const uint8_t * respBuf, uint16_t respLen, uint64_t responderNodeId,
                                        uint8_t * exportedKeyBuf, uint16_t exportedKeyBufSize, uint16_t & exportedKeyLen,
                                        uint32_t & exportedKeyId);

    CHIP_ERROR ProcessKeyExportReconfigure(const uint8_t * reconfBuf, uint16_t reconfLen);

    uint8_t ProposedConfig() const { return mProposedConfig; }
    void ProposedConfig(uint8_t val) { mProposedConfig = val; }

    bool AllowNestDevelopmentDevices() const { return mAllowNestDevDevices; }
    void AllowNestDevelopmentDevices(bool val) { mAllowNestDevDevices = val; }

    bool AllowSHA1DeviceCerts() const { return mAllowSHA1DeviceCerts; }
    void AllowSHA1DeviceCerts(bool val) { mAllowSHA1DeviceCerts = val; }

private:
    ChipKeyExport mKeyExportObj;
    uint32_t mKeyId;
    uint64_t mResponderNodeId;
    const uint8_t * mClientCert;
    uint16_t mClientCertLen;
    const uint8_t * mClientKey;
    uint16_t mClientKeyLen;
    const uint8_t * mAccessToken;
    uint16_t mAccessTokenLen;
    uint8_t mProposedConfig;
    bool mAllowNestDevDevices;
    bool mAllowSHA1DeviceCerts;

#if !CHIP_CONFIG_LEGACY_KEY_EXPORT_DELEGATE

    CHIP_ERROR GetNodeCertSet(ChipKeyExport * keyExport, ChipCertificateSet & certSet) __OVERRIDE;
    CHIP_ERROR ReleaseNodeCertSet(ChipKeyExport * keyExport, ChipCertificateSet & certSet) __OVERRIDE;
    CHIP_ERROR GenerateNodeSignature(ChipKeyExport * keyExport, const uint8_t * msgHash, uint8_t msgHashLen,
                                     TLVWriter & writer) __OVERRIDE;
    CHIP_ERROR BeginCertValidation(ChipKeyExport * keyExport, ValidationContext & validCtx,
                                   ChipCertificateSet & certSet) __OVERRIDE;
    CHIP_ERROR HandleCertValidationResult(ChipKeyExport * keyExport, ValidationContext & validCtx, ChipCertificateSet & certSet,
                                          uint32_t requestedKeyId) __OVERRIDE;
    CHIP_ERROR EndCertValidation(ChipKeyExport * keyExport, ValidationContext & validCtx, ChipCertificateSet & certSet) __OVERRIDE;
    CHIP_ERROR ValidateUnsignedKeyExportMessage(ChipKeyExport * keyExport, uint32_t requestedKeyId) __OVERRIDE;

#endif // !CHIP_CONFIG_LEGACY_KEY_EXPORT_DELEGATE

    CHIP_ERROR GetNodeCertSet(bool isInitiator, ChipCertificateSet & certSet);
    CHIP_ERROR ReleaseNodeCertSet(bool isInitiator, ChipCertificateSet & certSet);
    CHIP_ERROR GetNodePrivateKey(bool isInitiator, const uint8_t *& weavePrivKey, uint16_t & weavePrivKeyLen);
    CHIP_ERROR ReleaseNodePrivateKey(bool isInitiator, const uint8_t *& weavePrivKey);
    CHIP_ERROR BeginCertValidation(bool isInitiator, ChipCertificateSet & certSet, ValidationContext & validContext);
    CHIP_ERROR HandleCertValidationResult(bool isInitiator, ChipCertificateSet & certSet, ValidationContext & validContext,
                                          const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo, uint32_t requestedKeyId);
    CHIP_ERROR EndCertValidation(bool isInitiator, ChipCertificateSet & certSet, ValidationContext & validContext);
    CHIP_ERROR ValidateUnsignedKeyExportMessage(bool isInitiator, const IPPacketInfo * pktInfo, const ChipMessageInfo * msgInfo,
                                                uint32_t requestedKeyId);
};

} // namespace KeyExport
} // namespace Security
} // namespace Profiles
} // namespace chip

#endif // CHIPKEYEXPORTCLIENT_H_
