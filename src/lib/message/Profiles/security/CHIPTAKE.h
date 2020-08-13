/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      responders for the CHIP Token Authenticated Key Exchange
 *      (TAKE) protocol.
 *
 */

#ifndef CHIPTAKE_H_
#define CHIPTAKE_H_

#include <core/CHIPConfig.h>

#include <core/CHIPCore.h>
#include <core/CHIPError.h>
#include <support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>

#include <stdint.h>
#include <string.h>

/**
 *   @namespace chip::Profiles::Security::TAKE
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     Token Authenticated Key Exchange (TAKE) protocol within the
 *     CHIP Security profile.
 */

namespace chip {
namespace Profiles {
namespace Security {
namespace TAKE {

using chip::ChipEncryptionKey;
using chip::System::PacketBuffer;

static const uint8_t kSaltTimeUnlimitedIdentificationKey[] = { 0x12, 0x34, 0x56, 0x78 };
static const uint8_t kSaltProtocolEncryption[]             = { 0x54, 0x41, 0x4B, 0x45 }; // TAKE

// TAKE Control Header Field Definitions
enum
{
    //
    kControlHeader_NumOptionalConfigurationMask   = 0b00001111,
    kControlHeader_NumOptionalConfigurationShift  = 0,
    kControlHeader_EncryptAuthenticationPhaseFlag = 0b00010000,
    kControlHeader_EncryptCommunicationsPhaseFlag = 0b00100000,
    kControlHeader_TimeLimitFlag                  = 0b01000000,
    kControlHeader_HasChallengerIdFlag            = 0b10000000
};

enum
{
    kConfig1_CurveSize             = 28,
    kConfig1_PrivKeySize           = kConfig1_CurveSize + 1,
    kConfig1_ECPointX962FormatSize = 2 * kConfig1_CurveSize + 1,
    kConfig1_HMACSignatureSize     = chip::Platform::Security::SHA1::kHashLength
};

// size of data, in bytes
enum
{
    kIdentificationRootKeySize = 16,
    kNonceSize                 = 16,
    kMaxChallengerIdSize       = 16,
    kIdentificationKeySize     = 16,
    kTokenMasterKeySize        = 32,
    kTokenEncryptedStateSize   = 16,
    kMaxOptionalConfigurations = 16,
    kAuthenticationKeySize     = 16,
    kMaxTokenPrivateKeySize    = ((CHIP_CONFIG_MAX_EC_BITS + 7) / 8) + 1,
    // NOTE: These parameters should be reviewed and updated when new TAKE Configs are introduced
    kMaxCurveSize                        = kConfig1_CurveSize,
    kMaxECDHPrivateKeySize               = kConfig1_PrivKeySize,
    kMaxECDHPublicKeySize                = kConfig1_ECPointX962FormatSize,
    kMaxIdentifyTokenResponseKeySaltSize = kMaxChallengerIdSize + 4,
    kMaxECDSASignatureSize               = 56,
    kMaxProtocolEncryptionKeySaltSize    = 6 + kMaxOptionalConfigurations + 2 * kNonceSize + sizeof(kSaltProtocolEncryption),
    kMaxAuthenticationKeySaltSize        = 6 + kMaxOptionalConfigurations + 2 * kNonceSize + kMaxChallengerIdSize
};

// size of messages, in bytes
enum
{
    kIdentifyTokenMsgMinSize             = 3 + kNonceSize,
    kIdentifyTokenResponseMsgSize        = 1 + kNonceSize + chip::Platform::Security::SHA1::kHashLength,
    kAuthenticateTokenMsgMinSize         = kConfig1_HMACSignatureSize,
    kAuthenticateTokenResponseMsgMinSize = kTokenEncryptedStateSize,
    kReAuthenticateTokenMsgSize          = kTokenEncryptedStateSize + kConfig1_HMACSignatureSize,
    kReAuthenticateTokenResponseMsgSize  = kConfig1_HMACSignatureSize,
    kTokenRecongfigureMsgSize            = 1
};

enum
{
    kTAKEConfig_Invalid = 0,
    kTAKEConfig_Config1 = 1
};

// Abstract delegate class called by TAKE engine to perform various
// actions related to authentication during a TAKE exchange.
class ChipTAKEChallengerAuthDelegate
{
public:
    // Rewind Identification Key Iterator.
    // Called to prepare for a new Identification Key search.
    virtual CHIP_ERROR RewindIdentificationKeyIterator(void) = 0;

    // Get next {tokenId, IK} pair.
    // returns tokenId = kNodeIdNotSpecified if no more IKs are available.
    virtual CHIP_ERROR GetNextIdentificationKey(uint64_t & tokenId, uint8_t * identificationKey,
                                                uint16_t & identificationKeyLen) = 0;

    // Get Token Authentication Data.
    // Function returns {takeConfig = 0x00, authKey = NULL, encAuthBlob = NULL} if Authentication Data associated with a specified
    // Token is not stored on the device. On the function call authKeyLen and encAuthBlobLen inputs specify sizes of the authKey and
    // encAuthBlob buffers, respectively. Function should update these parameters to reflect actual sizes.
    virtual CHIP_ERROR GetTokenAuthData(uint64_t tokenId, uint8_t & takeConfig, uint8_t * authKey, uint16_t & authKeyLen,
                                        uint8_t * encAuthBlob, uint16_t & encAuthBlobLen) = 0;

    // Store Token Authentication Data.
    // This function should clear Authentication Data that was previously stored on the device for the specified Token (if any).
    virtual CHIP_ERROR StoreTokenAuthData(uint64_t tokenId, uint8_t takeConfig, const uint8_t * authKey, uint16_t authKeyLen,
                                          const uint8_t * encAuthBlob, uint16_t encAuthBlobLen) = 0;

    // Clear Token Authentication Data.
    // This function should be called if ReAuthentication phase with the Token Authentication Data stored on the device failed.
    virtual CHIP_ERROR ClearTokenAuthData(uint64_t tokenId) = 0;

    // Get Token public key.
    // On the function call tokenPubKey length input specifies size of the tokenPubKey buffer. Function should update this parameter
    // to reflect actual sizes.
    virtual CHIP_ERROR GetTokenPublicKey(uint64_t tokenId, OID & curveOID, EncodedECPublicKey & tokenPubKey) = 0;

    // Get the challenger ID.
    virtual CHIP_ERROR GetChallengerID(uint8_t * challengerID, uint8_t & challengerIDLen) const = 0;
};

// Abstract delegate class called by TAKE Token to get
// token-specific infor related to the TAKE authentication.
class ChipTAKETokenAuthDelegate
{
public:
    // Get the token Master key. size: kTokenMasterKeySize
    virtual CHIP_ERROR GetTokenMasterKey(uint8_t * tokenMasterKey) const = 0;

    // Get the Identification Root Key. size: kIdentificationRootKeySize
    virtual CHIP_ERROR GetIdentificationRootKey(uint8_t * identificationRootKey) const = 0;

    // Get the token Private Key.
    // On the function call tokenPrivKeyLen input specifies size of the tokenPrivKey buffer.
    // Function should update this parameter to reflect actual sizes of the private key.
    virtual CHIP_ERROR GetTokenPrivateKey(OID & curveOID, EncodedECPrivateKey & tokenPrivKey) const = 0;

    // Get TAKE Time.
    // Function returns takeTime, which is Unix time rounded with 24 hour granularity
    // i.e. number of days elapsed after 1 January 1970.
    virtual CHIP_ERROR GetTAKETime(uint32_t & takeTime) const = 0;
};

// Implements the core logic of the CHIP TAKE protocol.
class DLL_EXPORT ChipTAKEEngine
{
public:
    ChipTAKEChallengerAuthDelegate * ChallengerAuthDelegate; // Challenger Authentication delegate object
    ChipTAKETokenAuthDelegate * TokenAuthDelegate;           // Token Authentication delegate object

    uint8_t ChallengerNonce[kNonceSize];
    uint8_t TokenNonce[kNonceSize];

    uint8_t ControlHeader;
    uint8_t EncryptionType;
    uint8_t ProtocolConfig;

    uint8_t OptionalConfigurations[kMaxOptionalConfigurations];

    uint16_t SessionKeyId;

    uint8_t ChosenConfiguration;

    uint8_t ChallengerId[kMaxChallengerIdSize];
    uint8_t ChallengerIdLen;

    void Init(void);
    void Shutdown(void);

    // First handshake.
    CHIP_ERROR GenerateIdentifyTokenMessage(uint16_t sessionKeyId, uint8_t takeConfig, bool encryptAuthPhase, bool encryptCommPhase,
                                            bool timeLimitedIK, bool sendChallengerId, uint8_t encryptionType, uint64_t localNodeId,
                                            PacketBuffer * msgBuf);
    CHIP_ERROR ProcessIdentifyTokenMessage(uint64_t peerNodeId, const PacketBuffer * msgBuf);
    CHIP_ERROR GenerateIdentifyTokenResponseMessage(PacketBuffer * msgBuf);
    CHIP_ERROR ProcessIdentifyTokenResponseMessage(const PacketBuffer * buf);

    // In case a reconfigure is needed.
    CHIP_ERROR GenerateTokenReconfigureMessage(PacketBuffer * msgBuf);
    CHIP_ERROR ProcessTokenReconfigureMessage(uint8_t & config, const PacketBuffer * msgBuf);

    // Generate the encryption key.
    CHIP_ERROR GenerateProtocolEncryptionKey(void);

    // Second handshake, assuming no reauthentication.
    CHIP_ERROR GenerateAuthenticateTokenMessage(PacketBuffer * msgBuf);
    CHIP_ERROR ProcessAuthenticateTokenMessage(const PacketBuffer * msgBuf);
    CHIP_ERROR GenerateAuthenticateTokenResponseMessage(PacketBuffer * msgBuf);
    CHIP_ERROR ProcessAuthenticateTokenResponseMessage(const PacketBuffer * msgBuf);

    // Second handshake, assuming reauthentication.
    CHIP_ERROR GenerateReAuthenticateTokenMessage(PacketBuffer * msgBuf);
    CHIP_ERROR ProcessReAuthenticateTokenMessage(const PacketBuffer * msgBuf);
    CHIP_ERROR GenerateReAuthenticateTokenResponseMessage(PacketBuffer * msgBuf);
    CHIP_ERROR ProcessReAuthenticateTokenResponseMessage(const PacketBuffer * msgBuf);

    // Returns the session key.
    CHIP_ERROR GetSessionKey(const ChipEncryptionKey *& encKey) const;
    uint8_t GetEncryptionType(void);

    bool UseSessionKey(void) const;

    uint8_t GetNumOptionalConfigurations(void) const;
    bool IsEncryptAuthPhase(void) const;
    bool IsEncryptCommPhase(void) const;
    bool IsTimeLimitedIK(void) const;
    bool HasSentChallengerId(void) const;

    uint16_t GetCurveLen(void) const;
    uint16_t GetPrivKeyLen(void) const;
    uint16_t GetECPointLen(void) const;
    OID GetCurveOID(void) const;

private:
    ChipEncryptionKey EncryptionKey;

    enum EngineState
    {
        kState_Reset = 0,

        // Initiator States
        kState_InitiatorStatesBase                           = 10,
        kState_InitiatorStatesEnd                            = 19,
        kState_InitiatorIdentifyTokenGenerated               = kState_InitiatorStatesBase + 0,
        kState_InitiatorIdentifyTokenResponseProcessed       = kState_InitiatorStatesBase + 1,
        kState_InitiatorAuthenticateTokenGenerated           = kState_InitiatorStatesBase + 2,
        kState_InitiatorAuthenticateTokenResponseProcessed   = kState_InitiatorStatesBase + 3,
        kState_InitiatorReAuthenticateTokenGenerated         = kState_InitiatorStatesBase + 4,
        kState_InitiatorReAuthenticateTokenResponseProcessed = kState_InitiatorStatesBase + 5,
        kState_InitiatorReconfigureProcessed                 = kState_InitiatorStatesBase + 6,

        // Responder States
        kState_ResponderStatesBase                           = 20,
        kState_ResponderStatesEnd                            = 29,
        kState_ResponderIdentifyTokenProcessed               = kState_ResponderStatesBase + 0,
        kState_ResponderIdentifyTokenResponseGenerated       = kState_ResponderStatesBase + 1,
        kState_ResponderAuthenticateTokenProcessed           = kState_ResponderStatesBase + 2,
        kState_ResponderAuthenticateTokenResponseGenerated   = kState_ResponderStatesBase + 3,
        kState_ResponderReAuthenticateTokenProcessed         = kState_ResponderStatesBase + 4,
        kState_ResponderReAuthenticateTokenResponseGenerated = kState_ResponderStatesBase + 5,
        kState_ResponderDone                                 = kState_ResponderStatesBase + 6
    };

    uint8_t State;

    enum EncryptionKeyState
    {
        kEncryptionKeyState_Uninitialized,
        kEncryptionKeyState_Initialized,
    };

    uint8_t KeyState;

    uint8_t IdentificationKey[kIdentificationKeySize];
    uint8_t AuthenticationKey[kAuthenticationKeySize];
    uint8_t EncryptedAuthenticationKey[kTokenEncryptedStateSize];

    // Used only by the initiator
    uint8_t ECDHPrivateKeyBuffer[kMaxECDHPrivateKeySize];
    uint16_t ECDHPrivateKeyLength;
    uint8_t ECDHPublicKeyBuffer[kMaxECDHPublicKeySize];
    uint64_t TokenId;

    void GenerateHMACSignature(const uint8_t * key, uint8_t * dest, const uint8_t * additionalField = NULL,
                               uint8_t additionalFieldLength = 0, uint16_t keyLength = kIdentificationKeySize);

    CHIP_ERROR GenerateAuthenticationKey(const uint8_t * challengerId, uint8_t * privateKey, uint8_t * publicKey,
                                         uint16_t privateKeyLen);

    CHIP_ERROR GenerateSignatureForAuthenticateTokenResponse(uint8_t * dest, const uint8_t * challengerECDHPublicKey,
                                                             const uint8_t * tokenECDHPublicKey, EncodedECPrivateKey TPriv,
                                                             const uint8_t * encryptedState, OID & curveOID);
    CHIP_ERROR VerifySignatureForAuthenticateTokenResponse(const uint8_t * signature, const uint8_t * challengerECDHPublicKey,
                                                           const uint8_t * tokenECDHPublicKey, const uint8_t * encryptedState,
                                                           OID & curveOID, EncodedECPublicKey & encodedPubKey);
    void GenerateHashForAuthenticateTokenResponse(uint8_t * dest, const uint8_t * challengerECDHPublicKey,
                                                  const uint8_t * tokenECDHPublicKey, const uint8_t * encryptedState);

    // Read/Write values from/to a message, incrementing the pointer by the length of the field.
    static void ReadArray(uint8_t * dest, const uint8_t *& src, uint8_t length);
    static void WriteArray(const uint8_t * src, uint8_t *& dest, uint8_t length);
};

} // namespace TAKE
} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPTAKE_H_ */
