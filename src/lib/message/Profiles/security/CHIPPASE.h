/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines data types and objects for initiators and
 *      responders for the CHIP Password Authenticated Session
 *      Establishment (PASE) protocol.
 *
 */

#ifndef CHIPPASE_H_
#define CHIPPASE_H_

#include <core/CHIPConfig.h>

#define CHIP_IS_EC_PASE_ENABLED                                                                                                    \
    (CHIP_CONFIG_SUPPORT_PASE_CONFIG2 || CHIP_CONFIG_SUPPORT_PASE_CONFIG3 || CHIP_CONFIG_SUPPORT_PASE_CONFIG4 ||                   \
     CHIP_CONFIG_SUPPORT_PASE_CONFIG5)

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <core/CHIPVendorIdentifiers.hpp>
#include <support/DLLUtil.h>
#include <support/crypto/HKDF.h>
#include <support/crypto/HMAC.h>
#include <support/crypto/HashAlgos.h>

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG1 && !CHIP_WITH_OPENSSL
#error                                                                                                                             \
    "INVALID CHIP CONFIG: PASE Config1 enabled but OpenSSL not available (CHIP_CONFIG_SUPPORT_PASE_CONFIG1 == 1 && CHIP_WITH_OPENSSL == 0)."
#endif

#if CHIP_IS_EC_PASE_ENABLED
#include <support/crypto/EllipticCurve.h>
#endif

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG1
#include "openssl/bn.h"
struct JPAKE_CTX;
#endif

/**
 *   @namespace chip::Profiles::Security::PASE
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     Password Authenticated Session Establishment (PASE) protocol
 *     within the CHIP security profile.
 */

namespace chip {
namespace Profiles {
namespace Security {
namespace PASE {

using chip::ChipEncryptionKey;
using chip::System::PacketBuffer;
#if CHIP_IS_EC_PASE_ENABLED
using chip::ASN1::OID;
using chip::Crypto::EllipticCurveJPAKE;
#endif

// PASE Protocol Configurations
enum
{
    // -- PASE Protocol Configuration Values
    kPASEConfig_Unspecified          = 0,
    kPASEConfig_Config0_TEST_ONLY    = (kChipVendor_NestLabs << 16) | 0,
    kPASEConfig_Config1              = (kChipVendor_NestLabs << 16) | 1,
    kPASEConfig_Config2              = (kChipVendor_NestLabs << 16) | 2,
    kPASEConfig_Config3              = (kChipVendor_NestLabs << 16) | 3,
    kPASEConfig_Config4              = (kChipVendor_NestLabs << 16) | 4,
    kPASEConfig_Config5              = (kChipVendor_NestLabs << 16) | 5,
    kPASEConfig_ConfigLast           = (kChipVendor_NestLabs << 16) | 5,
    kPASEConfig_ConfigDefault        = kPASEConfig_Config4,
    kPASEConfig_ConfigNestNumberMask = 0x07,

    // -- Security Strength Metric for PASE Configuration
    kPASEConfig_Config0SecurityStrength = 10,
    kPASEConfig_Config1SecurityStrength = 80,
    kPASEConfig_Config2SecurityStrength = 80,
    kPASEConfig_Config3SecurityStrength = 96,
    kPASEConfig_Config4SecurityStrength = 112,
    kPASEConfig_Config5SecurityStrength = 128,

    // -- PASE Supported Configurations Bit Masks
    kPASEConfig_SupportConfig0Bit_TEST_ONLY = 0x01,
    kPASEConfig_SupportConfig1Bit           = 0x02,
    kPASEConfig_SupportConfig2Bit           = 0x04,
    kPASEConfig_SupportConfig3Bit           = 0x08,
    kPASEConfig_SupportConfig4Bit           = 0x10,
    kPASEConfig_SupportConfig5Bit           = 0x20,
    kPASEConfig_SupportedConfigs            = (0x00 |
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
                                    kPASEConfig_SupportConfig0Bit_TEST_ONLY |
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG1
                                    kPASEConfig_SupportConfig1Bit |
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG2
                                    kPASEConfig_SupportConfig2Bit |
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG3
                                    kPASEConfig_SupportConfig3Bit |
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG4
                                    kPASEConfig_SupportConfig4Bit |
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG5
                                    kPASEConfig_SupportConfig5Bit |
#endif
                                    0x00)
};

// PASE Header Field Definitions
enum
{
    // Control Header Fields
    kPASEHeader_SessionKeyMask          = 0x0000FFFF,
    kPASEHeader_SessionKeyShift         = 0,
    kPASEHeader_EncryptionTypeMask      = 0x000F0000,
    kPASEHeader_EncryptionTypeShift     = 16,
    kPASEHeader_PasswordSourceMask      = 0x00F00000,
    kPASEHeader_PasswordSourceShift     = 20,
    kPASEHeader_PerformKeyConfirmFlag   = 0x80000000,
    kPASEHeader_ControlHeaderUnusedBits = ~(kPASEHeader_SessionKeyMask | kPASEHeader_EncryptionTypeMask |
                                            kPASEHeader_PasswordSourceMask | kPASEHeader_PerformKeyConfirmFlag),

    // Size Header Fields and Values
    kPASEHeader_GXWordCountMask           = 0x000000FF,
    kPASEHeader_GXWordCountShift          = 0,
    kPASEHeader_GXWordCountMaxConfig0     = 16,
    kPASEHeader_GXWordCountMaxConfig1     = 32,
    kPASEHeader_ZKPXGRWordCountMask       = 0x0000FF00,
    kPASEHeader_ZKPXGRWordCountShift      = 8,
    kPASEHeader_ZKPXGRWordCountMaxConfig0 = 16,
    kPASEHeader_ZKPXGRWordCountMaxConfig1 = 32,
    kPASEHeader_ZKPXBWordCountMask        = 0x00FF0000,
    kPASEHeader_ZKPXBWordCountShift       = 16,
    kPASEHeader_ZKPXBWordCountMaxConfig0  = 8,
    kPASEHeader_ZKPXBWordCountMaxConfig1  = 5,
    kPASEHeader_AlternateConfigCountMask  = 0xFF000000,
    kPASEHeader_AlternateConfigCountShift = 24,
    kPASEHeader_KeyConfirmWordCountMask =
        kPASEHeader_AlternateConfigCountMask, // Alternate interpretation of 0xFF000000 field used in
    kPASEHeader_KeyConfirmWordCountShift = kPASEHeader_AlternateConfigCountShift, //  InitiatorStep2Message.

    kPASESizeHeader_MaxConstantSizesConfig0 =
        ((kPASEHeader_GXWordCountMaxConfig0 << kPASEHeader_GXWordCountShift) & kPASEHeader_GXWordCountMask) |
        ((kPASEHeader_ZKPXGRWordCountMaxConfig0 << kPASEHeader_ZKPXGRWordCountShift) & kPASEHeader_ZKPXGRWordCountMask) |
        ((kPASEHeader_ZKPXBWordCountMaxConfig0 << kPASEHeader_ZKPXBWordCountShift) & kPASEHeader_ZKPXBWordCountMask),
    kPASESizeHeader_MaxConstantSizesConfig1 =
        ((kPASEHeader_GXWordCountMaxConfig1 << kPASEHeader_GXWordCountShift) & kPASEHeader_GXWordCountMask) |
        ((kPASEHeader_ZKPXGRWordCountMaxConfig1 << kPASEHeader_ZKPXGRWordCountShift) & kPASEHeader_ZKPXGRWordCountMask) |
        ((kPASEHeader_ZKPXBWordCountMaxConfig1 << kPASEHeader_ZKPXBWordCountShift) & kPASEHeader_ZKPXBWordCountMask)

};

// PASE Config0 Parameters
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
enum
{
    kPASEConfig0_GXByteCount        = 4 * kPASEHeader_GXWordCountMaxConfig0,
    kPASEConfig0_ZKPXGRByteCount    = 4 * kPASEHeader_ZKPXGRWordCountMaxConfig0,
    kPASEConfig0_ZKPXBByteCount     = 4 * kPASEHeader_ZKPXBWordCountMaxConfig0,
    kPASEConfig0_GXStep1p1Value     = 0x3A,
    kPASEConfig0_ZKPXGRStep1p1Value = 0xF1,
    kPASEConfig0_ZKPXBStep1p1Value  = 0xAA,
    kPASEConfig0_GXStep1p2Value     = 0x5C,
    kPASEConfig0_ZKPXGRStep1p2Value = 0x55,
    kPASEConfig0_ZKPXBStep1p2Value  = 0x6B,
    kPASEConfig0_GXStep2Value       = 0x9E,
    kPASEConfig0_ZKPXGRStep2Value   = 0x37,
    kPASEConfig0_ZKPXBStep2Value    = 0xDA
};
#endif

enum
{
    // Key Meterial Length for Config 0 and Ellipric Curve Configs
    kKeyMaterialLength_Config0_EC = chip::Platform::Security::SHA256::kHashLength,

    // Hash Length of ZKP_GR value
    kStep2ZKPXGRHashLength_Config1    = chip::Platform::Security::SHA1::kHashLength,
    kStep2ZKPXGRHashLength_Config0_EC = chip::Platform::Security::SHA256::kHashLength,
    kStep2ZKPXGRHashLengthMax         = chip::Platform::Security::SHA256::kHashLength,

    // Length of Key Confirmation Key, which is used to generate Key Configmation Hashes
    kKeyConfirmKeyLength_Config1    = chip::Platform::Security::SHA1::kHashLength,
    kKeyConfirmKeyLength_Config0_EC = chip::Platform::Security::SHA256::kHashLength,
    kKeyConfirmKeyLengthMax         = chip::Platform::Security::SHA256::kHashLength,

    // Length of Key Confirmation Hash
    kKeyConfirmHashLength_Config1    = chip::Platform::Security::SHA1::kHashLength,
    kKeyConfirmHashLength_Config0_EC = chip::Platform::Security::SHA256::kHashLength,
    kKeyConfirmHashLengthMax         = chip::Platform::Security::SHA256::kHashLength
};

enum
{
    kMaxAlternateProtocolConfigs = 3,
};

// Implements the core logic of the CHIP PASE protocol.
class DLL_EXPORT ChipPASEEngine
{
public:
    enum EngineState
    {
        kState_Reset = 0,

        // Initiator States
        kState_InitiatorStatesBase        = 10,
        kState_InitiatorStatesEnd         = 19,
        kState_InitiatorStep1Generated    = kState_InitiatorStatesBase + 0,
        kState_ResponderReconfigProcessed = kState_InitiatorStatesBase + 1,
        kState_ResponderStep1Processed    = kState_InitiatorStatesBase + 2,
        kState_ResponderStep2Processed    = kState_InitiatorStatesBase + 3,
        kState_InitiatorStep2Generated    = kState_InitiatorStatesBase + 4,
        kState_InitiatorDone              = kState_InitiatorStatesBase + 5,
        kState_InitiatorFailed            = kState_InitiatorStatesBase + 6,

        // Responder States
        kState_ResponderStatesBase     = 20,
        kState_ResponderStatesEnd      = 29,
        kState_InitiatorStep1Processed = kState_ResponderStatesBase + 0,
        kState_ResponderStep1Generated = kState_ResponderStatesBase + 1,
        kState_ResponderStep2Generated = kState_ResponderStatesBase + 2,
        kState_InitiatorStep2Processed = kState_ResponderStatesBase + 3,
        kState_ResponderDone           = kState_ResponderStatesBase + 4,
        kState_ResponderFailed         = kState_ResponderStatesBase + 5
    };

#if CHIP_CONFIG_SUPPORT_PASE_CONFIG1
    struct JPAKE_CTX * JPAKECtx;
#endif
#if CHIP_IS_EC_PASE_ENABLED
    EllipticCurveJPAKE mEllipticCurveJPAKE;
#endif
    EngineState State;
    uint32_t ProtocolConfig;
    const uint8_t * Pw;
    uint16_t PwLen;
    uint16_t SessionKeyId;
    uint8_t EncryptionType;
    uint8_t AllowedPASEConfigs;
    uint8_t PwSource;
    bool PerformKeyConfirmation;

    void Init(void);
    void Shutdown(void);
    void Reset(void);
    bool IsInitiator(void) const;
    bool IsResponder(void) const;

    CHIP_ERROR GenerateInitiatorStep1(PacketBuffer * buf, uint32_t proposedPASEConfig, uint64_t localNodeId, uint64_t peerNodeId,
                                      uint16_t sessionKeyId, uint8_t encType, uint8_t pwSrc, ChipFabricState * FabricState,
                                      bool confirmKey);
    CHIP_ERROR ProcessInitiatorStep1(PacketBuffer * buf, uint64_t localNodeId, uint64_t peerNodeId, ChipFabricState * FabricState);
    CHIP_ERROR GenerateResponderStep1(PacketBuffer * buf);
    CHIP_ERROR GenerateResponderStep2(PacketBuffer * buf);
    CHIP_ERROR ProcessResponderStep1(PacketBuffer * buf);
    CHIP_ERROR ProcessResponderStep2(PacketBuffer * buf);
    CHIP_ERROR GenerateInitiatorStep2(PacketBuffer * buf);
    CHIP_ERROR ProcessInitiatorStep2(PacketBuffer * buf);
    CHIP_ERROR GenerateResponderKeyConfirm(PacketBuffer * buf);
    CHIP_ERROR ProcessResponderKeyConfirm(PacketBuffer * buf);
    CHIP_ERROR GenerateResponderReconfigure(PacketBuffer * buf);
    CHIP_ERROR ProcessResponderReconfigure(PacketBuffer * buf, uint32_t & proposedPASEConfig);
    CHIP_ERROR GetSessionKey(const ChipEncryptionKey *& encKey);

private:
    union
    {
        ChipEncryptionKey EncryptionKey;
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
        uint8_t KeyMeterial_Config0[kKeyMaterialLength_Config0_EC];
#endif
    };
    union
    {
        uint8_t ResponderKeyConfirmHash[kKeyConfirmHashLengthMax];
        uint8_t ResponderStep2ZKPXGRHash[kStep2ZKPXGRHashLengthMax];
    };

    CHIP_ERROR InitState(uint64_t localNodeId, uint64_t peerNodeId, uint8_t pwSrc, ChipFabricState * FabricState,
                         uint32_t * altConfigs, uint8_t altConfigsCount, bool isInitiator);
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG1
    CHIP_ERROR FormProtocolContextString(uint64_t localNodeId, uint64_t peerNodeId, uint8_t pwSrc, uint32_t * altConfigs,
                                         uint8_t altConfigsCount, bool isInitiator, char * buf, size_t bufSize);
    CHIP_ERROR GenerateStep1Data_Config1(PacketBuffer * buf, uint16_t & stepDataLen);
    CHIP_ERROR ProcessStep1Data_Config1(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t gxWordCount, uint8_t zkpxgrWordCount,
                                        uint8_t zpkxbWordCount);
    CHIP_ERROR GenerateStep2Data_Config1(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t * Step2ZKPXGRHash);
    CHIP_ERROR ProcessStep2Data_Config1(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t gxWordCount, uint8_t zkpxgrWordCount,
                                        uint8_t zpkxbWordCount, uint8_t * step2ZKPXGRHash);
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY || CHIP_IS_EC_PASE_ENABLED
    CHIP_ERROR FormProtocolContextData(uint64_t localNodeId, uint64_t peerNodeId, uint8_t pwSrc, uint32_t * altConfigs,
                                       uint8_t altConfigsCount, bool isInitiator, uint8_t * buf, size_t bufSize,
                                       uint16_t & contextLen);
#endif
#if CHIP_CONFIG_SUPPORT_PASE_CONFIG0_TEST_ONLY
    CHIP_ERROR GenerateStep1Data_Config0_TEST_ONLY(PacketBuffer * buf, uint16_t & stepDataLen);
    CHIP_ERROR ProcessStep1Data_Config0_TEST_ONLY(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t gxWordCount,
                                                  uint8_t zkpxgrWordCount, uint8_t zkpxbWordCount);
    CHIP_ERROR GenerateStep2Data_Config0_TEST_ONLY(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t * Step2ZKPXGRHash);
    CHIP_ERROR ProcessStep2Data_Config0_TEST_ONLY(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t gxWordCount,
                                                  uint8_t zkpxgrWordCount, uint8_t zkpxbWordCount, uint8_t * Step2ZKPXGRHash);
#endif
#if CHIP_IS_EC_PASE_ENABLED
    CHIP_ERROR GenerateStep1Data_ConfigEC(PacketBuffer * buf, uint16_t & stepDataLen);
    CHIP_ERROR ProcessStep1Data_ConfigEC(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t gxWordCount, uint8_t zkpxgrWordCount,
                                         uint8_t zkpxbWordCount);
    CHIP_ERROR GenerateStep2Data_ConfigEC(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t * Step2ZKPXGRHash);
    CHIP_ERROR ProcessStep2Data_ConfigEC(PacketBuffer * buf, uint16_t & stepDataLen, uint8_t gxWordCount, uint8_t zkpxgrWordCount,
                                         uint8_t zkpxbWordCount, uint8_t * Step2ZKPXGRHash);
#endif
    void ProtocolHash(const uint8_t * data, const uint16_t dataLen, uint8_t * h);
    CHIP_ERROR DeriveKeys(const uint8_t * initiatorStep2ZKPXGRHash, const uint8_t step2ZKPXGRHashLength, uint8_t * keyConfirmKey,
                          const uint8_t keyConfirmKeyLength);
    void GenerateKeyConfirmHashes(const uint8_t * keyConfirmKey, const uint8_t keyConfirmKeyLength, uint8_t * initiatorHash,
                                  uint8_t * responderHash, const uint8_t keyConfirmHashLength);
    bool IsAllowedPASEConfig(uint32_t config) const;
    uint32_t PackSizeHeader(uint8_t altConfigCount);
    CHIP_ERROR GenerateAltConfigsList(uint32_t * altConfigs, uint8_t & altConfigsCount);
    CHIP_ERROR FindStrongerAltConfig(uint32_t * altConfigs, uint8_t altConfigsCount);
};

} // namespace PASE
} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPPASE_H_ */
