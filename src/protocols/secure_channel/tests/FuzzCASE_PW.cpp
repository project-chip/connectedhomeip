#include <cstddef>
#include <cstdint>
#include <stdarg.h>
#include <string.h>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/secure_channel/CASESession.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <credentials/tests/CHIPCert_test_vectors.h>

namespace chip {
namespace Testing {

using namespace std;

using namespace Crypto;
using namespace Credentials;
using namespace fuzztest;
using namespace Transport;
using namespace Messaging;
using namespace System::Clock::Literals;
using namespace TLV;
using namespace System;

class FuzzCASESession : public CASESession
{
public:
    // Exposed CASESession Structs
    using CASESession::EncodeSigma1Inputs;
    using CASESession::EncodeSigma2Inputs;
    using CASESession::EncodeSigma2ResumeInputs;
    using CASESession::HandleSigma3Data;
    using CASESession::ParsedSigma1;
    using CASESession::ParsedSigma2;
    using CASESession::ParsedSigma2Resume;
    using CASESession::ParsedSigma2TBEData;

    // Exposed CASESession Methods
    using CASESession::EncodeSigma1;
    using CASESession::EncodeSigma2;
    using CASESession::EncodeSigma2Resume;
    using CASESession::HandleSigma3b;
    using CASESession::ParseSigma1;
    using CASESession::ParseSigma2;
    using CASESession::ParseSigma2Resume;
    using CASESession::ParseSigma2TBEData;
    using CASESession::ParseSigma3;
    using CASESession::ParseSigma3TBEData;
};

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::ParseSigma1 robustness against malformed or arbitrary Sigma1 payloads by:
 * 1. Taking a valid fully-encoded Sigma1 byte array as a seed and feeding it to the FuzzTest
 * 2. Allow the Fuzzing Engine to mutate it as it sees fit (behind the scenes)
 * 3. Directly parsing it
 *
 * Starting from a valid payload seed ensures the fuzzer explores mutations around
 * well-formed messages, increasing the likelihood of finding edge cases errors in the parser.
 */
void ParseSigma1_RawPayload(const vector<uint8_t> & fuzzEncodedSigma1)
{

    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        // fuzzing an already fully-encoded Sigma1, and injecting it into ParseSigma1 to test if it crashes

        PacketBufferHandle EncodedSigma1 =
            PacketBufferHandle::NewWithData(fuzzEncodedSigma1.data(), fuzzEncodedSigma1.size(), 0, 38);
        PacketBufferTLVReader tlvReaderEncodedSigma1;
        tlvReaderEncodedSigma1.Init(std::move(EncodedSigma1));

        FuzzCASESession::ParsedSigma1 unused;
        FuzzCASESession::ParseSigma1(tlvReaderEncodedSigma1, unused);
    }

    Platform::MemoryShutdown();
}

// This Encoded Sigma1 was extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic inputs.
// This Payload is based on Test Vectors with the OpCred Identity: Root01:Node01_02
// TODO #37654: Replace this extracted data with official test vectors when available
uint8_t FuzzSeed_EncodedSigma1_Node01_02_Chip[] = {
    0x15, 0x30, 0x01, 0x20, 0x3b, 0x0d, 0xee, 0xab, 0x7b, 0x79, 0x31, 0xc8, 0x10, 0x9e, 0x58, 0xb2, 0x90, 0xc0, 0x9c, 0x5a,
    0x33, 0xa2, 0x10, 0xe7, 0x91, 0xf2, 0x69, 0x79, 0x93, 0x44, 0xce, 0xb3, 0xd9, 0x44, 0x84, 0x06, 0x25, 0x02, 0xa0, 0xc1,
    0x30, 0x03, 0x20, 0x47, 0xf1, 0x42, 0x0c, 0xa6, 0xd7, 0x2a, 0xea, 0x3f, 0x68, 0x97, 0x17, 0xd9, 0x27, 0x0e, 0x7f, 0x0e,
    0x7d, 0x62, 0x21, 0x73, 0x98, 0x04, 0x53, 0x81, 0x06, 0xc0, 0x14, 0x9a, 0x50, 0xa0, 0x04, 0x30, 0x04, 0x41, 0x04, 0x8f,
    0xb3, 0x21, 0xc9, 0xad, 0x4e, 0x55, 0xe0, 0xac, 0xfa, 0xe6, 0x56, 0x83, 0xf3, 0xe2, 0x3b, 0xa3, 0xeb, 0x45, 0x6f, 0x4c,
    0xb1, 0x00, 0xc5, 0x73, 0x24, 0x3a, 0x80, 0xc7, 0xbd, 0xf4, 0xd7, 0x9c, 0xaa, 0x96, 0xbb, 0xce, 0x7c, 0x6e, 0xf3, 0x8c,
    0x7b, 0xc4, 0xbb, 0xe9, 0xb8, 0xf5, 0xeb, 0xe8, 0x90, 0xa9, 0x0a, 0x85, 0xc2, 0x0a, 0x1b, 0x2e, 0x9d, 0x14, 0x4d, 0x6c,
    0x73, 0x13, 0xf9, 0x35, 0x05, 0x25, 0x01, 0x88, 0x13, 0x25, 0x02, 0x2c, 0x01, 0x25, 0x03, 0xa0, 0x0f, 0x24, 0x04, 0x12,
    0x24, 0x05, 0x0c, 0x26, 0x06, 0x00, 0x01, 0x04, 0x01, 0x24, 0x07, 0x01, 0x18, 0x18
};

auto SeededEncodedSigma1()
{
    std::vector<uint8_t> dataVec(std::begin(FuzzSeed_EncodedSigma1_Node01_02_Chip),
                                 std::end(FuzzSeed_EncodedSigma1_Node01_02_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ dataVec });
}

FUZZ_TEST(FuzzCASE, ParseSigma1_RawPayload)
    .WithDomains(
        // fuzzing a fully constructed Sigma1, based on a seed of a valid Example of Sigma1
        SeededEncodedSigma1());
/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

inline constexpr uint8_t kInitiatorRandomTag    = 1;
inline constexpr uint8_t kInitiatorSessionIdTag = 2;
inline constexpr uint8_t kDestinationIdTag      = 3;
inline constexpr uint8_t kInitiatorEphPubKeyTag = 4;
inline constexpr uint8_t kInitiatorMRPParamsTag = 5;
inline constexpr uint8_t kResumptionIDTag       = 6;
inline constexpr uint8_t kResume1MICTag         = 7;

CHIP_ERROR EncodeSigma1Helper(PacketBufferHandle & msg, FuzzCASESession::EncodeSigma1Inputs & inputParams,
                              ByteSpan & initiatorEphPubKey)
{

    size_t dataLen = TLV::EstimateStructOverhead(inputParams.initiatorRandom.size(),     // initiatorRandom
                                                 sizeof(inputParams.initiatorSessionId), // initiatorSessionId
                                                 inputParams.destinationId.size(),       // destinationId
                                                 initiatorEphPubKey.size(),              // InitiatorEphPubKey,
                                                 SessionParameters::kEstimatedTLVSize,   // initiatorSessionParams
                                                 inputParams.resumptionId.size(),        // resumptionId
                                                 inputParams.initiatorResumeMIC.size()   // initiatorResumeMIC
    );

    msg = PacketBufferHandle::New(dataLen);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);

    PacketBufferTLVWriter tlvWriter;
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    tlvWriter.Init(std::move(msg));

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kInitiatorRandomTag), inputParams.initiatorRandom));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kInitiatorSessionIdTag), inputParams.initiatorSessionId));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kDestinationIdTag), inputParams.destinationId));
    ReturnErrorOnFailure(
        tlvWriter.PutBytes(TLV::ContextTag(kInitiatorEphPubKeyTag), initiatorEphPubKey.data(), initiatorEphPubKey.size()));

    VerifyOrReturnError(inputParams.initiatorMrpConfig != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(
        CASESession::EncodeSessionParameters(TLV::ContextTag(kInitiatorMRPParamsTag), *inputParams.initiatorMrpConfig, tlvWriter));

    if (inputParams.sessionResumptionRequested)
    {
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kResumptionIDTag), inputParams.resumptionId));
        ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(kResume1MICTag), inputParams.initiatorResumeMIC));
    }

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg));

    return CHIP_NO_ERROR;
}

/**
 * FuzzTest property function: Tests CASESession::ParseSigma1 ability to handle well-formed Sigma1 messages by:
 * 1. Constructing a Sigma1 message from fuzzed individual components
 * 2. Encoding it using EncodeSigma1Helper to create a properly structured payload
 * 3. Parsing the encoded message to check for crashes or undefined behavior
 *
 * This test ensures the parser can handle various combinations of valid field values
 * without crashing, even if the field contents are unusual or edge cases.
 */

void ParseSigma1_StructuredPayload(const vector<uint8_t> & fuzzInitiatorRandom, uint32_t fuzzInitiatorSessionId,
                                   const vector<uint8_t> & fuzzDestinationID, const vector<uint8_t> & fuzzInitiatorEphPubKey,
                                   const vector<uint8_t> & fuzzResumptionID, const vector<uint8_t> & fuzzInitiatorResumeMIC,
                                   bool fuzzSessionResumptionRequested)
{

    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {

        /********************************* Preparing Sigma1 Encoding based on fuzzed elements*********************************/
        FuzzCASESession::EncodeSigma1Inputs encodeParams;
        encodeParams.initiatorRandom    = ByteSpan(fuzzInitiatorRandom.data(), fuzzInitiatorRandom.size());
        encodeParams.initiatorSessionId = fuzzInitiatorSessionId;
        encodeParams.destinationId      = ByteSpan(fuzzDestinationID.data(), fuzzDestinationID.size());
        ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                     System::Clock::Milliseconds16(4000));
        encodeParams.initiatorMrpConfig = &LocalMRPConfig;

        if (fuzzSessionResumptionRequested)
        {
            encodeParams.sessionResumptionRequested = true;
            // TODO : make all encodeParams initialisations similar to the below ones
            encodeParams.resumptionId       = ByteSpan(fuzzResumptionID.data(), fuzzResumptionID.size());
            encodeParams.initiatorResumeMIC = ByteSpan(fuzzInitiatorResumeMIC.data(), fuzzInitiatorResumeMIC.size());
        }

        ByteSpan initiatorEphPubKey = ByteSpan(fuzzInitiatorEphPubKey.data(), fuzzInitiatorEphPubKey.size());

        /********************************* Encode Sigma1 *********************************/
        PacketBufferHandle encodedSigma1Msg;
        EncodeSigma1Helper(encodedSigma1Msg, encodeParams, initiatorEphPubKey);

        /********************************* Fuzz the Encoded Sigma1 Payload *********************************/
        PacketBufferTLVReader tlvReader;
        tlvReader.Init(std::move(encodedSigma1Msg));
        FuzzCASESession::ParsedSigma1 unused;

        FuzzCASESession::ParseSigma1(tlvReader, unused);
        // std::cout << "ParseSigma1: " << err.Format() << std::endl;
    }

    Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzCASE, ParseSigma1_StructuredPayload)
    .WithDomains(
        // InitiatorRandom (Original size = kSigmaParamRandomNumberSize)
        Arbitrary<vector<uint8_t>>(),
        // InitiatorSessionId
        Arbitrary<uint32_t>(),
        // DestinationIdentifier .WithSize(kSHA256_Hash_Length)
        Arbitrary<vector<uint8_t>>(),
        // initiatorEphPubKey (Original size = kP256_PublicKey_Length)
        Arbitrary<vector<uint8_t>>(),
        // fuzzResumptionID, (Original size = SessionResumptionStorage::kResumptionIdSize)
        Arbitrary<vector<uint8_t>>(),
        // fuzzInitiatorResumeMIC, (Original size =CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES)
        Arbitrary<vector<uint8_t>>(),
        // fuzzSessionResumptionRequested
        Arbitrary<bool>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests the correctness of Sigma1 encoding and parsing through a complete round-trip:
 * 1. Encodes a Sigma1 message from fuzzed components using CASESession::EncodeSigma1
 * 2. Parses the encoded message using CASESession::ParseSigma1
 * 3. Verifies that all parsed values exactly match the original inputs
 */

void EncodeParseSigma1RoundTrip(const vector<uint8_t> & fuzzInitiatorRandom, uint32_t fuzzInitiatorSessionId,
                                const vector<uint8_t> & fuzzDestinationID, const vector<uint8_t> & fuzzInitiatorEphPubKey,
                                const vector<uint8_t> & fuzzResumptionID, const vector<uint8_t> & fuzzInitiatorResumeMIC,
                                bool fuzzSessionResumptionRequested)
{

    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {

        /********************************* Preparing Sigma1 Encoding based on fuzzed elements*********************************/

        FuzzCASESession::EncodeSigma1Inputs encodeParams;
        encodeParams.initiatorRandom    = ByteSpan(fuzzInitiatorRandom.data(), fuzzInitiatorRandom.size());
        encodeParams.initiatorSessionId = fuzzInitiatorSessionId;
        encodeParams.destinationId      = ByteSpan(fuzzDestinationID.data(), fuzzDestinationID.size());
        ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                     System::Clock::Milliseconds16(4000));
        encodeParams.initiatorMrpConfig = &LocalMRPConfig;

        if (fuzzSessionResumptionRequested)
        {
            encodeParams.sessionResumptionRequested = true;
            encodeParams.resumptionId               = ByteSpan(fuzzResumptionID.data(), fuzzResumptionID.size());
            encodeParams.initiatorResumeMIC         = ByteSpan(fuzzInitiatorResumeMIC.data(), fuzzInitiatorResumeMIC.size());
        }

        P256PublicKey initiatorEphPubKey(FixedByteSpan<kP256_PublicKey_Length>(fuzzInitiatorEphPubKey.data()));
        encodeParams.initiatorEphPubKey = &initiatorEphPubKey;

        /********************************* Encode Sigma1 Using CASESession::EncodeSigma1 *********************************/
        PacketBufferHandle encodedSigma1Msg;

        if (FuzzCASESession::EncodeSigma1(encodedSigma1Msg, encodeParams) != CHIP_NO_ERROR)
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        /********************************* Parse the Encoded Sigma1 using CASESession::ParseSigma1*****************************/

        PacketBufferTLVReader tlvReader;
        tlvReader.Init(std::move(encodedSigma1Msg));
        FuzzCASESession::ParsedSigma1 parsedMessage;

        if (FuzzCASESession::ParseSigma1(tlvReader, parsedMessage) != CHIP_NO_ERROR)
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        /**************  RoundTrip Check: Compare Parsed Values with fuzzed inputs *****************/
        ASSERT_TRUE(parsedMessage.initiatorRandom.data_equal(encodeParams.initiatorRandom));
        ASSERT_EQ(parsedMessage.initiatorSessionId, encodeParams.initiatorSessionId);
        ASSERT_TRUE(parsedMessage.destinationId.data_equal(encodeParams.destinationId));
        ASSERT_TRUE(parsedMessage.initiatorEphPubKey.data_equal(
            ByteSpan(encodeParams.initiatorEphPubKey->ConstBytes(), encodeParams.initiatorEphPubKey->Length())));

        if (fuzzSessionResumptionRequested)
        {
            ASSERT_TRUE(parsedMessage.resumptionId.data_equal(encodeParams.resumptionId));
            ASSERT_TRUE(parsedMessage.initiatorResumeMIC.data_equal(encodeParams.initiatorResumeMIC));
            ASSERT_TRUE(parsedMessage.sessionResumptionRequested);
        }
    }

    Platform::MemoryShutdown();
}
FUZZ_TEST(FuzzCASE, EncodeParseSigma1RoundTrip)
    .WithDomains(
        // InitiatorRandom (Original size .WithSize(kSigmaParamRandomNumberSize)
        Arbitrary<vector<uint8_t>>(),
        // InitiatorSessionId
        Arbitrary<uint32_t>(),
        // DestinationIdentifier, Original size .WithSize(kSHA256_Hash_Length)
        Arbitrary<vector<uint8_t>>(),
        // initiatorEphPubKey, Original size .WithSize(kP256_PublicKey_Length))
        Arbitrary<vector<uint8_t>>().WithSize(kP256_PublicKey_Length),
        // fuzzResumptionID, Original size: .WithSize(SessionResumptionStorage::kResumptionIdSize)
        Arbitrary<vector<uint8_t>>(),
        // fuzzInitiatorResumeMIC, Original size: .WithSize(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES),
        Arbitrary<vector<uint8_t>>(),
        // fuzzSessionResumptionRequested
        Arbitrary<bool>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::ParseSigma2 robustness against malformed or arbitrary Sigma2 payloads by:
 * 1. Taking a valid fully-encoded Sigma2 byte array as a seed and feeding it to the FuzzTest
 * 2. Allow the Fuzzing Engine to mutate it as it sees fit (behind the scenes)
 * 3. Directly parsing it
 *
 * Starting from a valid payload seed ensures the fuzzer explores mutations around
 * well-formed messages, increasing the likelihood of finding edge cases errors in the parser.
 */

void ParseSigma2_RawPayload(const vector<uint8_t> & seededEncodedSigma2)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        PacketBufferHandle EncodedSigma2 =
            PacketBufferHandle::NewWithData(seededEncodedSigma2.data(), seededEncodedSigma2.size(), 0, 38);
        PacketBufferTLVReader tlvReaderEncodedSigma2;
        tlvReaderEncodedSigma2.Init(std::move(EncodedSigma2));

        FuzzCASESession::ParsedSigma2 unused;
        FuzzCASESession::ParseSigma2(tlvReaderEncodedSigma2, unused);
    }

    Platform::MemoryShutdown();
}

// This Encoded Sigma2 was extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic inputs.
// This Payload is based on Test Vectors with the OpCred Identity: Root01:Node01_02
// TODO #37654: Replace this extracted data with official test vectors when available
uint8_t FuzzSeed_EncodedSigma2_Node01_02_Chip[] = {
    0x15, 0x30, 0x01, 0x20, 0xb7, 0x4a, 0xb7, 0x9e, 0x2a, 0xab, 0x6d, 0xca, 0xa2, 0x83, 0x76, 0x43, 0x2f, 0xc8, 0x66, 0xf0, 0x47,
    0x4a, 0x49, 0x7f, 0xd4, 0xbe, 0x7c, 0x08, 0x80, 0xa0, 0x3d, 0x6e, 0xf5, 0xf2, 0x6f, 0x9b, 0x25, 0x02, 0x9f, 0xc1, 0x30, 0x03,
    0x41, 0x04, 0x97, 0x4b, 0x02, 0x8e, 0xda, 0xb0, 0x62, 0x00, 0x46, 0x78, 0xf8, 0x34, 0xc0, 0x5d, 0x53, 0x79, 0xb8, 0x44, 0xd1,
    0x84, 0x2a, 0x34, 0x7d, 0xec, 0x9e, 0x6d, 0x9b, 0x56, 0x0a, 0x6f, 0x2e, 0x95, 0xb1, 0x58, 0x62, 0x4b, 0x23, 0x21, 0xc9, 0xb1,
    0x12, 0x66, 0xc8, 0x55, 0x7e, 0xa5, 0xef, 0xe7, 0x75, 0xa7, 0x1c, 0xc4, 0x30, 0x8a, 0xad, 0x42, 0xce, 0x62, 0x02, 0x52, 0x48,
    0xd9, 0x74, 0x74, 0x31, 0x04, 0x78, 0x02, 0x5e, 0xa5, 0xc6, 0x2e, 0xce, 0x0f, 0x35, 0xeb, 0x30, 0x8b, 0x30, 0x45, 0x8d, 0xf1,
    0x20, 0x48, 0xe2, 0xf3, 0x9f, 0xae, 0x90, 0x78, 0xa8, 0xb2, 0xc8, 0x57, 0x88, 0x71, 0x86, 0x21, 0x8b, 0x55, 0x86, 0x39, 0xab,
    0x10, 0x9d, 0x78, 0x22, 0x70, 0x51, 0x86, 0x19, 0xd1, 0x16, 0x0f, 0x9e, 0xa1, 0xb4, 0x0b, 0x32, 0x28, 0x48, 0xb4, 0x23, 0xd4,
    0xd4, 0xa0, 0x81, 0xe9, 0xba, 0x59, 0xd5, 0x75, 0xa7, 0x3e, 0xdf, 0x1a, 0xa0, 0x21, 0x12, 0x56, 0x96, 0x81, 0x69, 0x60, 0x5e,
    0x49, 0xa3, 0xd9, 0xa1, 0x0d, 0xed, 0x9a, 0xb4, 0x0d, 0x40, 0xaf, 0x0d, 0x05, 0x7e, 0x2c, 0xb1, 0x30, 0xf7, 0x78, 0x0a, 0x39,
    0x77, 0x90, 0x62, 0x36, 0xae, 0x3b, 0x57, 0x31, 0x5f, 0xbd, 0x67, 0x57, 0x36, 0x48, 0x10, 0x5f, 0x68, 0x1b, 0x2f, 0xf6, 0xeb,
    0xb3, 0x53, 0x69, 0x88, 0xfe, 0xf4, 0x8b, 0xa0, 0x3f, 0x93, 0xa7, 0x19, 0x87, 0x6b, 0xc8, 0xd2, 0x1e, 0xbf, 0x8e, 0x6c, 0xfc,
    0xbb, 0x87, 0x07, 0x19, 0xdf, 0xea, 0xad, 0xf2, 0xf5, 0x53, 0x0e, 0x2c, 0x2c, 0x71, 0x7b, 0xbe, 0xf4, 0xd9, 0x22, 0x94, 0x7d,
    0x15, 0xa5, 0x71, 0x67, 0xf4, 0xf9, 0x98, 0x69, 0x95, 0x93, 0x26, 0x1a, 0x52, 0x55, 0x27, 0x26, 0x32, 0xc6, 0xb3, 0x63, 0x96,
    0x1f, 0xde, 0xca, 0xf7, 0x20, 0xe6, 0x7d, 0xcf, 0x6c, 0xd4, 0xaa, 0x7f, 0xe2, 0xfe, 0x7b, 0x2c, 0xff, 0x4b, 0x7c, 0x1c, 0xc3,
    0x75, 0x3b, 0xcf, 0xf8, 0x28, 0x9d, 0x79, 0x47, 0x86, 0x4a, 0xba, 0x5f, 0x70, 0x6a, 0x64, 0xef, 0x3e, 0xb9, 0xaa, 0x75, 0xdb,
    0x29, 0xe6, 0x93, 0x2a, 0x76, 0x9d, 0x06, 0x61, 0x54, 0x21, 0x51, 0xa6, 0x78, 0x1c, 0x54, 0x95, 0x7f, 0x93, 0x2a, 0x1f, 0x36,
    0x1b, 0xda, 0x9c, 0x55, 0x45, 0xaf, 0x87, 0xa6, 0xc5, 0x1e, 0x4c, 0x81, 0x92, 0x55, 0x58, 0xc4, 0xaa, 0x63, 0x8f, 0xef, 0x07,
    0x46, 0xf1, 0x65, 0xb9, 0x00, 0x13, 0x8b, 0xb8, 0xf9, 0xd8, 0x57, 0xea, 0x8c, 0xe9, 0xe4, 0x84, 0x1c, 0x7b, 0x02, 0x44, 0x37,
    0xb2, 0x3e, 0x99, 0x86, 0x47, 0xac, 0xc1, 0x05, 0xa6, 0x35, 0x8b, 0xc5, 0x98, 0x43, 0x94, 0xab, 0x7a, 0xfe, 0x97, 0xa2, 0xb2,
    0x9d, 0xbb, 0xe3, 0xc9, 0xe3, 0x71, 0xc2, 0x8f, 0x4a, 0xc5, 0x90, 0x46, 0x38, 0x8d, 0x7d, 0x91, 0x37, 0xf8, 0x34, 0x4e, 0x16,
    0x82, 0xd0, 0x88, 0xd8, 0x43, 0xb1, 0xa4, 0x52, 0x9c, 0x88, 0xda, 0xa7, 0xd0, 0x82, 0x7d, 0xfc, 0x67, 0x19, 0xd6, 0x87, 0x81,
    0x6e, 0xc4, 0x31, 0x06, 0x2c, 0x2f, 0xa4, 0xa2, 0xf6, 0x72, 0x83, 0x4e, 0x5b, 0xcb, 0x56, 0x9d, 0x5c, 0xec, 0x30, 0x24, 0x54,
    0xeb, 0x34, 0x0e, 0x74, 0x79, 0x1e, 0x7e, 0x0a, 0x71, 0x6b, 0x3f, 0x68, 0x80, 0xfa, 0xab, 0x8e, 0x4b, 0x7c, 0x2b, 0x45, 0xd3,
    0xd6, 0xb7, 0xb1, 0x43, 0xce, 0x5f, 0x93, 0x2f, 0x5b, 0xb6, 0xd5, 0x6c, 0xe2, 0x7f, 0x5c, 0x81, 0xaf, 0x7f, 0x26, 0x60, 0x7c,
    0xa0, 0x7e, 0x7a, 0x3a, 0xa8, 0x3b, 0xd5, 0x2b, 0xc5, 0x13, 0x46, 0x86, 0x3c, 0xa7, 0xd6, 0x9c, 0x84, 0x77, 0x15, 0xda, 0xbd,
    0xaa, 0x2d, 0x31, 0x7d, 0x90, 0x7c, 0x19, 0xe3, 0x3c, 0xe9, 0xb5, 0x10, 0xf4, 0xcf, 0xe9, 0x26, 0x07, 0x1d, 0x84, 0x3d, 0x4e,
    0xc3, 0x41, 0x3e, 0x0c, 0x98, 0xbe, 0x0e, 0x86, 0xff, 0xe7, 0xed, 0x74, 0xab, 0x0d, 0x88, 0xa1, 0x51, 0x32, 0x8d, 0xc0, 0x80,
    0xd7, 0x12, 0xbd, 0xf1, 0xc8, 0x1e, 0x20, 0x83, 0x33, 0x14, 0xf0, 0x14, 0x18, 0xe2, 0x50, 0x65, 0x42, 0x85, 0x9b, 0xf8, 0x18,
    0x3f, 0xd6, 0x67, 0xfd, 0xcd, 0x03, 0x7f, 0xbc, 0xd4, 0x0e, 0xbc, 0x5f, 0x89, 0x4f, 0x8a, 0x03, 0xa5, 0x92, 0xc4, 0xf3, 0xa2,
    0xda, 0x71, 0x9e, 0x52, 0x7d, 0x49, 0x6a, 0xf1, 0xbd, 0x5a, 0x75, 0x75, 0xa9, 0x9d, 0x12, 0xc6, 0xbf, 0xcf, 0x2e, 0x25, 0xad,
    0x30, 0x42, 0x6c, 0x74, 0x73, 0x54, 0x10, 0x6e, 0x80, 0xdb, 0xdc, 0x2d, 0x93, 0x17, 0x37, 0x78, 0x82, 0x66, 0xe2, 0xd8, 0x68,
    0x0e, 0x7f, 0x28, 0xe0, 0xbb, 0x06, 0x7d, 0xde, 0x8a, 0x17, 0x52, 0xf7, 0x97, 0x86, 0xd1, 0xe0, 0xed, 0x73, 0xa4, 0x98, 0x2e,
    0x17, 0x73, 0x16, 0xdf, 0x64, 0xe4, 0xa3, 0x42, 0x90, 0xa6, 0x23, 0x6e, 0xc7, 0x7e, 0xb0, 0x8e, 0xb0, 0xc9, 0xaa, 0x1c, 0xfb,
    0xae, 0x22, 0x61, 0x94, 0xbc, 0xb4, 0x33, 0xfa, 0xca, 0xe1, 0x18, 0x3d, 0x9a, 0x16, 0xd4, 0xe6, 0x6f, 0xbc, 0xba, 0x92, 0xde,
    0x95, 0x15, 0x83, 0x1e, 0x59, 0x6a, 0xc7, 0x70, 0xd6, 0x35, 0x05, 0x26, 0x01, 0x40, 0x7e, 0x05, 0x00, 0x26, 0x02, 0xa0, 0x86,
    0x01, 0x00, 0x25, 0x03, 0x2c, 0x01, 0x24, 0x04, 0x12, 0x24, 0x05, 0x0c, 0x26, 0x06, 0x00, 0x01, 0x04, 0x01, 0x24, 0x07, 0x01,
    0x18, 0x18
};

auto SeededEncodedSigma2()
{
    std::vector<uint8_t> dataVec(std::begin(FuzzSeed_EncodedSigma2_Node01_02_Chip),
                                 std::end(FuzzSeed_EncodedSigma2_Node01_02_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ dataVec });
}

FUZZ_TEST(FuzzCASE, ParseSigma2_RawPayload).WithDomains(SeededEncodedSigma2());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

CHIP_ERROR EncodeSigma2Helper(MutableByteSpan & mem, const vector<uint8_t> & fuzzResponderRandom, uint32_t fuzzResponderSessionId,
                              const vector<uint8_t> & fuzzResponderEphPubKey, const vector<uint8_t> & fuzzEncrypted2)
{

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    TLVWriter tlvWriter;

    tlvWriter.Init(mem);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), fuzzResponderRandom.data(), fuzzResponderRandom.size()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(2), fuzzResponderSessionId));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(3), fuzzResponderEphPubKey.data(), fuzzResponderEphPubKey.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(4), fuzzEncrypted2.data(), fuzzEncrypted2.size()));

    ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                 System::Clock::Milliseconds16(4000));

    ReturnErrorOnFailure(CASESession::EncodeSessionParameters(TLV::ContextTag(5), LocalMRPConfig, tlvWriter));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    mem.reduce_size(tlvWriter.GetLengthWritten());

    ReturnErrorOnFailure(tlvWriter.Finalize());

    return CHIP_NO_ERROR;
}

/**
 * FuzzTest property function: Tests CASESession::ParseSigma2 ability to handle well-formed Sigma2 messages by:
 * 1. Constructing a Sigma2 message from fuzzed individual components
 * 2. Encoding it using EncodeSigma2Helper to create a properly structured payload
 * 3. Parsing the encoded message to check for crashes or undefined behavior
 *
 * This test ensures the parser can handle various combinations of valid field values
 * without crashing, even if the field contents are unusual or edge cases.
 */

void ParseSigma2_StructuredPayload(const vector<uint8_t> & fuzzResponderRandom, uint16_t fuzzResponderSessionId,
                                   const vector<uint8_t> & fuzzResponderEphPubKey, const vector<uint8_t> & fuzzEncrypted2)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);

    {
        size_t dataLen = TLV::EstimateStructOverhead(fuzzResponderRandom.size(),          // responderRandom
                                                     sizeof(fuzzResponderSessionId),      // responderSessionId
                                                     fuzzResponderEphPubKey.size(),       // signature
                                                     fuzzEncrypted2.size(),               // msgR2Encrypted
                                                     SessionParameters::kEstimatedTLVSize // SessionParameters

        );

        Platform::ScopedMemoryBuffer<uint8_t> mem;
        ASSERT_TRUE(mem.Calloc(dataLen));
        MutableByteSpan encodedSpan(mem.Get(), dataLen);

        if (CHIP_NO_ERROR !=
            EncodeSigma2Helper(encodedSpan, fuzzResponderRandom, fuzzResponderSessionId, fuzzResponderEphPubKey, fuzzEncrypted2))
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        TLV::ContiguousBufferTLVReader tlvReader;
        tlvReader.Init(encodedSpan);

        FuzzCASESession::ParsedSigma2 unused;
        FuzzCASESession::ParseSigma2(tlvReader, unused);
        // std::cout << err.Format() << std::endl;

        mem.Free();
    }

    Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzCASE, ParseSigma2_StructuredPayload)
    .WithDomains(
        // responderRandom Original size .WithSize(kSigmaParamRandomNumberSize)
        Arbitrary<vector<uint8_t>>(),
        // responderSessionId
        Arbitrary<uint16_t>(),
        // responderEphPubKey .WithSize(kP256_PublicKey_Length)
        Arbitrary<vector<uint8_t>>(),
        // Encrypted2 .WithMinSize(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES + 1)
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests the correctness of Sigma2 encoding and parsing through a complete round-trip:
 * 1. Encodes a Sigma1 message from fuzzed components using CASESession::EncodeSigma2
 * 2. Parses the encoded message using CASESession::ParseSigma2
 * 3. Verifies that all parsed values exactly match the original inputs
 */

void EncodeParseSigma2RoundTrip(const vector<uint8_t> & fuzzResponderRandom, uint32_t fuzzResponderSessionId,
                                const vector<uint8_t> & fuzzResponderEphPubKey, const vector<uint8_t> fuzzEncrypted2)
{

    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        /***************** prepare EncodeSigma2Inputs Struct to feed it into CASESession::EncodeSigma2******************/

        FuzzCASESession::EncodeSigma2Inputs encodeParams;
        memcpy(&encodeParams.responderRandom[0], fuzzResponderRandom.data(), fuzzResponderRandom.size());
        encodeParams.responderSessionId = fuzzResponderSessionId;

        P256PublicKey responderEphPubKey = FixedByteSpan<kP256_PublicKey_Length>(fuzzResponderEphPubKey.data());

        encodeParams.responderEphPubKey = &responderEphPubKey;

        encodeParams.msgR2Encrypted.Alloc(fuzzEncrypted2.size());
        memcpy(encodeParams.msgR2Encrypted.Get(), fuzzEncrypted2.data(), fuzzEncrypted2.size());
        encodeParams.encrypted2Length = fuzzEncrypted2.size();

        ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                     System::Clock::Milliseconds16(4000));
        encodeParams.responderMrpConfig = &LocalMRPConfig;

        /********************************* Encode Sigma2 Using CASESession::EncodeSigma2 *********************************/
        PacketBufferHandle encodedSigma2;
        if (FuzzCASESession::EncodeSigma2(encodedSigma2, encodeParams) != CHIP_NO_ERROR)
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        /********************************* Parse Encoded Sigma2 *********************************/
        PacketBufferTLVReader tlvReader;
        tlvReader.Init(std::move(encodedSigma2));
        FuzzCASESession::ParsedSigma2 parsedSigma2;

        if (FuzzCASESession::ParseSigma2(tlvReader, parsedSigma2) != CHIP_NO_ERROR)
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        /**************  RoundTrip Check: Compare Parsed Values with fuzzed inputs *****************/
        ASSERT_TRUE(parsedSigma2.responderRandom.data_equal(ByteSpan(encodeParams.responderRandom)));
        ASSERT_EQ(parsedSigma2.responderSessionId, encodeParams.responderSessionId);
        ASSERT_TRUE(parsedSigma2.responderEphPubKey.data_equal(
            ByteSpan(encodeParams.responderEphPubKey->ConstBytes(), encodeParams.responderEphPubKey->Length())));

        ASSERT_TRUE(parsedSigma2.msgR2Encrypted.Span().data_equal(ByteSpan(fuzzEncrypted2.data(), fuzzEncrypted2.size())));
    }
    Platform::MemoryShutdown();
}
FUZZ_TEST(FuzzCASE, EncodeParseSigma2RoundTrip)
    .WithDomains(
        // responderRandom (Original size = .WithSize(kSigmaParamRandomNumberSize))
        // We need to specify the size, to ensure we dont introduce buffer overflow or uninitialised data in the Test itself
        Arbitrary<vector<uint8_t>>().WithSize(kSigmaParamRandomNumberSize),
        // responderSessionId
        Arbitrary<uint32_t>(),
        // responderEphPubKey .WithSize(kP256_PublicKey_Length)
        Arbitrary<vector<uint8_t>>().WithSize(kP256_PublicKey_Length),
        // fuzzEncrypted2  .WithMinSize(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES + 1)
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::ParseSigma2TBEData robustness against malformed or arbitrary Sigma2TBEData
 * payloads by:
 * 1. Taking a valid fully-encoded Sigma2TBEData byte array as a seed and feeding it to the FuzzTest
 * 2. Allow the Fuzzing Engine to mutate it as it sees fit (behind the scenes)
 * 3. Directly parsing it
 *
 * Starting from a valid payload seed ensures the fuzzer explores mutations around
 * well-formed messages, increasing the likelihood of finding edge cases errors in the parser.
 */

void ParseSigma2TBEData_RawPayload(const vector<uint8_t> & fuzzEncodedSigma2TBEData)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        FuzzCASESession::ParsedSigma2TBEData parsedSigma2TBEDataFuzzed;
        PacketBufferHandle fuzzedMsg =
            PacketBufferHandle::NewWithData(fuzzEncodedSigma2TBEData.data(), fuzzEncodedSigma2TBEData.size(), 0, 38);
        PacketBufferTLVReader tlvReaderFuzzed;
        tlvReaderFuzzed.Init(std::move(fuzzedMsg));

        FuzzCASESession::ParseSigma2TBEData(tlvReaderFuzzed, parsedSigma2TBEDataFuzzed);
    }
    Platform::MemoryShutdown();
}

// These messages are extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic inputs.
// TODO #37654: Replace this extracted data with official test vectors when available
uint8_t FuzzSeed_EncodedSigma2TBE_Node01_02_Chip[] = {
    0x15, 0x31, 0x01, 0x0d, 0x01, 0x15, 0x30, 0x01, 0x08, 0x18, 0xe9, 0x69, 0xba, 0x0e, 0x08, 0x9e, 0x23, 0x24, 0x02, 0x01, 0x37,
    0x03, 0x27, 0x13, 0x03, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x26, 0x04, 0xef, 0x17, 0x1b, 0x27, 0x26, 0x05, 0x6e,
    0xb5, 0xb9, 0x4c, 0x37, 0x06, 0x27, 0x11, 0x01, 0x00, 0x01, 0x00, 0xde, 0xde, 0xde, 0xde, 0x27, 0x15, 0x1d, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xb0, 0xfa, 0x18, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01, 0x30, 0x09, 0x41, 0x04, 0xbc, 0xf6, 0x58, 0x0d, 0x2d, 0x71,
    0xe1, 0x44, 0x16, 0x65, 0x1f, 0x7c, 0x31, 0x1b, 0x5e, 0xfc, 0xf9, 0xae, 0xc0, 0xa8, 0xc1, 0x0a, 0xf8, 0x09, 0x27, 0x84, 0x4c,
    0x24, 0x0f, 0x51, 0xa8, 0xeb, 0x23, 0xfa, 0x07, 0x44, 0x13, 0x88, 0x87, 0xac, 0x1e, 0x73, 0xcb, 0x72, 0xa0, 0x54, 0xb6, 0xa0,
    0xdb, 0x06, 0x22, 0xaa, 0x80, 0x70, 0x71, 0x01, 0x63, 0x13, 0xb1, 0x59, 0x6c, 0x85, 0x52, 0xcf, 0x37, 0x0a, 0x35, 0x01, 0x28,
    0x01, 0x18, 0x24, 0x02, 0x01, 0x36, 0x03, 0x04, 0x02, 0x04, 0x01, 0x18, 0x30, 0x04, 0x14, 0x69, 0x67, 0xc9, 0x12, 0xf8, 0xa3,
    0xe6, 0x89, 0x55, 0x6f, 0x89, 0x9b, 0x65, 0xd7, 0x6f, 0x53, 0xfa, 0x65, 0xc7, 0xb6, 0x30, 0x05, 0x14, 0x44, 0x0c, 0xc6, 0x92,
    0x31, 0xc4, 0xcb, 0x5b, 0x37, 0x94, 0x24, 0x26, 0xf8, 0x1b, 0xbe, 0x24, 0xb7, 0xef, 0x34, 0x5c, 0x18, 0x30, 0x0b, 0x40, 0xce,
    0x6e, 0xf3, 0x93, 0xcb, 0xbc, 0x94, 0xf8, 0x0e, 0xe2, 0x90, 0xcb, 0x3c, 0x3d, 0x37, 0x33, 0x35, 0xba, 0xb9, 0x59, 0x07, 0x73,
    0x4d, 0x99, 0xd3, 0x84, 0xa6, 0x2a, 0x37, 0x3b, 0x84, 0x84, 0xe1, 0xd4, 0x1a, 0x04, 0xc3, 0x14, 0x0f, 0xaa, 0x19, 0xe8, 0xa2,
    0xb9, 0x9b, 0x0c, 0x61, 0xe3, 0x3c, 0x27, 0xea, 0x91, 0x39, 0x73, 0xe4, 0x5b, 0x5b, 0xc6, 0xe3, 0x9c, 0x27, 0x0d, 0xac, 0x53,
    0x18, 0x30, 0x02, 0xfc, 0x15, 0x30, 0x01, 0x08, 0x69, 0xd8, 0x6a, 0x8d, 0x80, 0xfc, 0x8f, 0x5d, 0x24, 0x02, 0x01, 0x37, 0x03,
    0x27, 0x14, 0x01, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x26, 0x04, 0xef, 0x17, 0x1b, 0x27, 0x26, 0x05, 0x6e, 0xb5,
    0xb9, 0x4c, 0x37, 0x06, 0x27, 0x13, 0x03, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01,
    0x30, 0x09, 0x41, 0x04, 0x5f, 0x94, 0xf5, 0x7e, 0x0b, 0x13, 0xc9, 0xcf, 0xcf, 0x96, 0xdf, 0xe1, 0xfc, 0xe7, 0x88, 0x8d, 0x56,
    0x4c, 0xc2, 0x09, 0xc5, 0x5c, 0x45, 0x08, 0xe4, 0x4d, 0xcf, 0x16, 0xba, 0x2e, 0x09, 0x66, 0x2f, 0x9e, 0xec, 0xf1, 0x9f, 0x40,
    0xb0, 0xe8, 0x8a, 0x0b, 0x28, 0x15, 0xda, 0x9e, 0xe1, 0x0a, 0x3a, 0x17, 0x7c, 0x25, 0x1f, 0x43, 0x4f, 0x5b, 0x0f, 0x26, 0x3c,
    0xe7, 0xde, 0x62, 0x78, 0xc6, 0x37, 0x0a, 0x35, 0x01, 0x29, 0x01, 0x18, 0x24, 0x02, 0x60, 0x30, 0x04, 0x14, 0x44, 0x0c, 0xc6,
    0x92, 0x31, 0xc4, 0xcb, 0x5b, 0x37, 0x94, 0x24, 0x26, 0xf8, 0x1b, 0xbe, 0x24, 0xb7, 0xef, 0x34, 0x5c, 0x30, 0x05, 0x14, 0xcc,
    0x13, 0x08, 0xaf, 0x82, 0xcf, 0xee, 0x50, 0x5e, 0xb2, 0x3b, 0x57, 0xbf, 0xe8, 0x6a, 0x31, 0x16, 0x65, 0x53, 0x5f, 0x18, 0x30,
    0x0b, 0x40, 0xad, 0xb8, 0x5b, 0x5d, 0x68, 0xcb, 0xfd, 0x36, 0x14, 0x0d, 0x8c, 0x9d, 0x12, 0x90, 0x14, 0xc4, 0x5f, 0xa7, 0xca,
    0x19, 0x1f, 0x34, 0xd9, 0xaf, 0x24, 0x1d, 0xb7, 0x17, 0x36, 0xe6, 0x0f, 0x44, 0x19, 0x9b, 0xc0, 0x7c, 0x7f, 0x79, 0x5b, 0xed,
    0x81, 0xa2, 0xe7, 0x7d, 0xc5, 0x34, 0x25, 0x76, 0xf6, 0xa0, 0xd1, 0x41, 0x98, 0xf4, 0x6b, 0x91, 0x07, 0x49, 0x42, 0x7c, 0x2e,
    0xed, 0x65, 0x9c, 0x18, 0x30, 0x03, 0x40, 0x83, 0x21, 0x57, 0x41, 0x27, 0x96, 0x60, 0x89, 0x23, 0xbd, 0x2d, 0x7b, 0x67, 0xc1,
    0xf7, 0x8d, 0x58, 0x0c, 0x07, 0x93, 0xb1, 0x4c, 0xad, 0x47, 0xb4, 0x8d, 0xae, 0xa2, 0x89, 0x37, 0xdf, 0x7b, 0x67, 0xc9, 0x88,
    0xb4, 0x0d, 0xd9, 0x5f, 0xe3, 0x7d, 0xa1, 0xe1, 0xf8, 0xd3, 0xa0, 0x40, 0xe7, 0x54, 0x62, 0x44, 0xf3, 0xe3, 0xf3, 0x84, 0x47,
    0xa1, 0xe5, 0xae, 0x14, 0xac, 0xca, 0x26, 0x11, 0x30, 0x04, 0x10, 0x88, 0xb6, 0x4a, 0xaa, 0xbd, 0x2d, 0xe3, 0x40, 0x15, 0x5c,
    0xc2, 0xd2, 0x83, 0x57, 0x3b, 0xcf, 0x18
};

auto SeededEncodedSigma2TBE()
{
    std::vector<uint8_t> EncodedSigma2TBEVector(std::begin(FuzzSeed_EncodedSigma2TBE_Node01_02_Chip),
                                                std::end(FuzzSeed_EncodedSigma2TBE_Node01_02_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ EncodedSigma2TBEVector });
}

FUZZ_TEST(FuzzCASE, ParseSigma2TBEData_RawPayload)
    .WithDomains(
        // Seeded Sigma2TBEData
        SeededEncodedSigma2TBE());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

CHIP_ERROR EncodeSigma2TBEDataHelper(MutableByteSpan & outMsgSpan, const vector<uint8_t> & responderNOC,
                                     const vector<uint8_t> & responderICAC, const vector<uint8_t> & signature,
                                     const vector<uint8_t> & resumptionID)
{
    TLVWriter tlvWriter;
    TLVType outerContainerType = kTLVType_NotSpecified;

    tlvWriter.Init(outMsgSpan);
    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));

    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), responderNOC.data(), responderNOC.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), responderICAC.data(), responderICAC.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(3), signature.data(), signature.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(4), resumptionID.data(), resumptionID.size()));

    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize());

    outMsgSpan.reduce_size(tlvWriter.GetLengthWritten());

    return CHIP_NO_ERROR;
}

/**
 * FuzzTest property function: Tests CASESession::ParseSigma2TBEData ability to handle well-formed Sigma2TBEData messages by:
 * 1. Constructing a Sigma2TBEData message from fuzzed individual components
 * 2. Encoding it using EncodeSigma2TBEDataHelper to create a properly structured payload
 * 3. Parsing the encoded message to check for crashes or undefined behavior
 *
 * This test ensures the parser can handle various combinations of valid field values
 * without crashing, even if the field contents are unusual or edge cases.
 */

void ParseSigma2TBEData_StructuredPayload(const vector<uint8_t> & fuzzResponderNOC, const vector<uint8_t> & fuzzResponderICAC,
                                          const vector<uint8_t> & fuzzSignature, const vector<uint8_t> & fuzzResumptionID)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        size_t dataLen = TLV::EstimateStructOverhead(fuzzResponderNOC.size(),  // responderNOC
                                                     fuzzResponderICAC.size(), // responderICAC
                                                     fuzzSignature.size(),     // signature
                                                     fuzzResumptionID.size()   // resumptionId
        );

        // Construct Sigma2TBEData
        Platform::ScopedMemoryBuffer<uint8_t> encodedSigma2TBEData;
        encodedSigma2TBEData.Alloc(dataLen);
        MutableByteSpan encodedSpan(encodedSigma2TBEData.Get(), dataLen);

        if (CHIP_NO_ERROR !=
            EncodeSigma2TBEDataHelper(encodedSpan, fuzzResponderNOC, fuzzResponderICAC, fuzzSignature, fuzzResumptionID))
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        // Parse Sigma2TBEData
        ContiguousBufferTLVReader tlvReader;
        tlvReader.Init(encodedSpan);
        FuzzCASESession::ParsedSigma2TBEData parsedSigma2TBEData;

        FuzzCASESession::ParseSigma2TBEData(tlvReader, parsedSigma2TBEData);
        // std::cout << "ParseSigma2TBEData: " << err.Format() << std::endl;
    }

    Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzCASE, ParseSigma2TBEData_StructuredPayload)
    .WithDomains(
        // responderNOC (Max size = Credentials::kMaxCHIPCertLength) .WithSize(Credentials::kMaxCHIPCertLength)
        Arbitrary<vector<uint8_t>>(),
        // responderICAC (Max size = Credentials::kMaxCHIPCertLength) .WithSize(Credentials::kMaxCHIPCertLength)
        Arbitrary<vector<uint8_t>>(),
        // signature (Original size = kMax_ECDSA_Signature_Length) .WithSize(kMax_ECDSA_Signature_Length),
        Arbitrary<vector<uint8_t>>(),
        // resumptionID,  .WithSize(SessionResumptionStorage::kResumptionIdSize)
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::ParseSigma2Resume robustness against malformed or arbitrary Sigma2Resume payloads
 * by:
 * 1. Taking a valid fully-encoded Sigma2Resume byte array as a seed and feeding it to the FuzzTest
 * 2. Allow the Fuzzing Engine to mutate it as it sees fit (behind the scenes)
 * 3. Directly parsing it
 *
 * Starting from a valid payload seed ensures the fuzzer explores mutations around
 * well-formed messages, increasing the likelihood of finding edge cases errors in the parser.
 */

void ParseSigma2Resume_RawPayload(const vector<uint8_t> & seededEncodedSigma2Resume)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        PacketBufferHandle encodedSigma2Resume =
            PacketBufferHandle::NewWithData(seededEncodedSigma2Resume.data(), seededEncodedSigma2Resume.size(), 0, 38);
        PacketBufferTLVReader tlvReaderSeeded;
        tlvReaderSeeded.Init(std::move(encodedSigma2Resume));

        FuzzCASESession::ParsedSigma2Resume unused;
        FuzzCASESession::ParseSigma2Resume(tlvReaderSeeded, unused);
    }
    Platform::MemoryShutdown();
}

// This Encoded Sigma2 Resume was extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic
// inputs. This Payload is based on Test Vectors with the OpCred Identity: Root01:Node01_02
// TODO #37654: Replace this extracted data with official test vectors when available
uint8_t FuzzSeed_EncodedSigma2Resume_Node01_02_Chip[] = {
    0x15, 0x30, 0x01, 0x10, 0xc6, 0x89, 0x85, 0xec, 0xd2, 0x52, 0xfe, 0x8e, 0xcf, 0xea, 0x62, 0x41, 0x6b, 0x04, 0x85,
    0x75, 0x30, 0x02, 0x10, 0x13, 0x98, 0x4f, 0x7c, 0xac, 0xdd, 0x65, 0xaf, 0x9c, 0x99, 0x9f, 0x7e, 0x27, 0xf4, 0xbc,
    0x77, 0x25, 0x03, 0xb1, 0x42, 0x35, 0x04, 0x25, 0x01, 0xf4, 0x01, 0x25, 0x02, 0x2c, 0x01, 0x25, 0x03, 0xa0, 0x0f,
    0x24, 0x04, 0x12, 0x24, 0x05, 0x0c, 0x26, 0x06, 0x00, 0x01, 0x04, 0x01, 0x24, 0x07, 0x01, 0x18, 0x18
};

auto SeededEncodedSigma2Resume()
{
    std::vector<uint8_t> dataVec(std::begin(FuzzSeed_EncodedSigma2Resume_Node01_02_Chip),
                                 std::end(FuzzSeed_EncodedSigma2Resume_Node01_02_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ dataVec });
}

FUZZ_TEST(FuzzCASE, ParseSigma2Resume_RawPayload)
    .WithDomains(
        // Seeded Encoded Sigma2 Resume Payload
        SeededEncodedSigma2Resume());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

CHIP_ERROR EncodeSigma2ResumeHelper(PacketBufferHandle & msg, const vector<uint8_t> & fuzzResumptionID,
                                    const vector<uint8_t> & fuzzSigma2ResumeMIC, uint16_t fuzzResponderSessionId)
{

    size_t dataLen = TLV::EstimateStructOverhead(fuzzResumptionID.size(), // ResumptionID
                                                 fuzzSigma2ResumeMIC.size(),
                                                 sizeof(fuzzResponderSessionId),      // responderSessionID
                                                 SessionParameters::kEstimatedTLVSize // SessionParameters

    );

    msg = PacketBufferHandle::New(dataLen);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);
    PacketBufferTLVWriter tlvWriter;
    tlvWriter.Init(std::move(msg));

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), fuzzResumptionID.data(), fuzzResumptionID.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), fuzzSigma2ResumeMIC.data(), fuzzSigma2ResumeMIC.size()));
    ReturnErrorOnFailure(tlvWriter.Put(TLV::ContextTag(3), fuzzResponderSessionId));

    ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                 System::Clock::Milliseconds16(4000));

    ReturnErrorOnFailure(CASESession::EncodeSessionParameters(TLV::ContextTag(4), LocalMRPConfig, tlvWriter));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));
    ReturnErrorOnFailure(tlvWriter.Finalize(&msg));

    return CHIP_NO_ERROR;
}

/**
 * FuzzTest property function: Tests CASESession::ParseSigma2Resume ability to handle well-formed Sigma2Resume messages by:
 * 1. Constructing a Sigma2Resume message from fuzzed individual components
 * 2. Encoding it using EncodeSigma2ResumeHelper to create a properly structured payload
 * 3. Parsing the encoded message to check for crashes or undefined behavior
 *
 * This test ensures the parser can handle various combinations of valid field values
 * without crashing, even if the field contents are unusual or edge cases.
 */

void ParseSigma2Resume_StructuredPayload(const vector<uint8_t> & fuzzResumptionID, const vector<uint8_t> & fuzzSigma2ResumeMIC,
                                         uint16_t fuzzResponderSessionId)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        PacketBufferHandle encodedSigma2Resume;

        if (CHIP_NO_ERROR !=
            EncodeSigma2ResumeHelper(encodedSigma2Resume, fuzzResumptionID, fuzzSigma2ResumeMIC, fuzzResponderSessionId))
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        PacketBufferTLVReader tlvReader;
        tlvReader.Init(std::move(encodedSigma2Resume));

        FuzzCASESession::ParsedSigma2Resume unused;
        FuzzCASESession::ParseSigma2Resume(tlvReader, unused);
    }

    Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzCASE, ParseSigma2Resume_StructuredPayload)
    .WithDomains(

        // fuzzResumptionID, (Original size = SessionResumptionStorage::kResumptionIdSize)
        Arbitrary<vector<uint8_t>>(),
        // fuzzSigma2ResumeMIC Original size .WithSize(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES)
        Arbitrary<vector<uint8_t>>(),
        // responderSessionId
        Arbitrary<uint16_t>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests the correctness of Sigma2Resume encoding and parsing through a complete round-trip:
 * 1. Encodes a Sigma2Resume message from fuzzed components using CASESession::EncodeSigma2Resume
 * 2. Parses the encoded message using CASESession::ParseSigma2Resume
 * 3. Verifies that all parsed values exactly match the original inputs
 */
void EncodeParseSigma2ResumeRoundTrip(const vector<uint8_t> & fuzzResumptionID, const vector<uint8_t> & fuzzSigma2ResumeMIC,
                                      uint16_t fuzzResponderSessionId)
{

    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        /***************** prepare EncodeSigma2ResumeInputs Struct to feed it into
         * CASESession::EncodeSigma2Resume******************/

        FuzzCASESession::EncodeSigma2ResumeInputs encodeParams;

        encodeParams.resumptionId = ByteSpan(fuzzResumptionID.data(), fuzzResumptionID.size());
        memcpy(&encodeParams.sigma2ResumeMICBuffer[0], fuzzSigma2ResumeMIC.data(), fuzzSigma2ResumeMIC.size());
        encodeParams.responderSessionId = fuzzResponderSessionId;

        ReliableMessageProtocolConfig LocalMRPConfig(System::Clock::Milliseconds32(100), System::Clock::Milliseconds32(200),
                                                     System::Clock::Milliseconds16(4000));
        encodeParams.responderMrpConfig = &LocalMRPConfig;

        /***************** Encode Sigma2Resume Using CASESession::EncodeSigma2Resume *********************/
        PacketBufferHandle encodedSigma2Resume;
        if (FuzzCASESession::EncodeSigma2Resume(encodedSigma2Resume, encodeParams) != CHIP_NO_ERROR)
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        /********************************* Parse Encoded Sigma2Resume *********************************/
        PacketBufferTLVReader tlvReader;
        tlvReader.Init(std::move(encodedSigma2Resume));
        FuzzCASESession::ParsedSigma2Resume parsedMessage;

        if (FuzzCASESession::ParseSigma2Resume(tlvReader, parsedMessage) != CHIP_NO_ERROR)
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        /**************  RoundTrip Check: Compare Parsed Values with fuzzed inputs *****************/
        ASSERT_TRUE(parsedMessage.resumptionId.data_equal(encodeParams.resumptionId));
        ASSERT_TRUE(parsedMessage.sigma2ResumeMIC.data_equal(encodeParams.sigma2ResumeMIC));
        ASSERT_EQ(parsedMessage.responderSessionId, encodeParams.responderSessionId);
    }
    Platform::MemoryShutdown();
}
FUZZ_TEST(FuzzCASE, EncodeParseSigma2ResumeRoundTrip)
    .WithDomains(
        // fuzzResumptionID
        Arbitrary<vector<uint8_t>>(),
        // fuzzSigma2ResumeMIC
        // We need to specify the size, to ensure we dont introduce buffer overflow or uninitialised data in the Test itself
        Arbitrary<vector<uint8_t>>().WithSize(CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES),
        // responderSessionId
        Arbitrary<uint32_t>());
/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::ParseSigma3 robustness against malformed or arbitrary Sigma3 payloads by:
 * 1. Taking a valid fully-encoded Sigma3 byte array as a seed and feeding it to the FuzzTest
 * 2. Allow the Fuzzing Engine to mutate it as it sees fit (behind the scenes)
 * 3. Directly parsing it
 *
 * Starting from a valid payload seed ensures the fuzzer explores mutations around
 * well-formed messages, increasing the likelihood of finding edge cases errors in the parser.
 */

void ParseSigma3_RawPayload(const vector<uint8_t> & fuzzEncodedSigma3)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        PacketBufferHandle EncodedSigma3 =
            PacketBufferHandle::NewWithData(fuzzEncodedSigma3.data(), fuzzEncodedSigma3.size(), 0, 38);

        PacketBufferTLVReader tlvReader2;
        tlvReader2.Init(std::move(EncodedSigma3));

        Platform::ScopedMemoryBufferWithSize<uint8_t> outMsgR3Encrypted;
        MutableByteSpan outMsgR3EncryptedPayload;
        ByteSpan outMsgR3MIC;

        FuzzCASESession::ParseSigma3(tlvReader2, outMsgR3Encrypted, outMsgR3EncryptedPayload, outMsgR3MIC);
    }
    Platform::MemoryShutdown();
}

// These messages are extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic inputs.
// TODO #37654: Replace this extracted data with official test vectors when available
uint8_t FuzzSeed_EncodedSigma3_Node01_02_Chip[] = {
    0x15, 0x31, 0x01, 0x65, 0x02, 0x82, 0x13, 0xbf, 0x62, 0x33, 0x08, 0x3c, 0x6e, 0x14, 0x21, 0xbc, 0x69, 0x97, 0x33, 0x94, 0xa8,
    0xea, 0x68, 0x34, 0x3b, 0xe2, 0x13, 0xe1, 0x72, 0x77, 0x7d, 0x0a, 0x3b, 0x61, 0x7b, 0x42, 0x6e, 0xa9, 0x6e, 0xed, 0x38, 0x1d,
    0x59, 0x35, 0x5e, 0x06, 0x82, 0x12, 0x84, 0xd3, 0xad, 0x80, 0x42, 0x6e, 0x3e, 0x03, 0x86, 0x3b, 0x38, 0x09, 0xda, 0xcc, 0xd0,
    0xe9, 0x8c, 0xd0, 0xe1, 0x9a, 0x6e, 0xdd, 0x4f, 0xd1, 0xfe, 0xf5, 0x91, 0xe0, 0xc1, 0xb1, 0xda, 0xf2, 0x85, 0xb5, 0x9d, 0x03,
    0x25, 0xe3, 0x21, 0xec, 0xe3, 0x7f, 0x71, 0xab, 0xe8, 0xf0, 0xaa, 0x33, 0xac, 0x21, 0x87, 0x67, 0x89, 0xe9, 0x70, 0xab, 0x93,
    0x62, 0xa2, 0x95, 0xe2, 0x1d, 0xaf, 0xf0, 0x97, 0x81, 0x93, 0x2c, 0x0d, 0x06, 0x73, 0x25, 0x60, 0x4d, 0x66, 0x2e, 0x60, 0xc5,
    0xa5, 0x8d, 0x8b, 0xe0, 0xb6, 0xa7, 0x61, 0x73, 0x80, 0x8b, 0xbc, 0x7c, 0xf3, 0xdb, 0x7f, 0x9e, 0x65, 0x8f, 0x58, 0xcd, 0xba,
    0x04, 0x60, 0x8c, 0x4c, 0x69, 0xca, 0x54, 0x33, 0x29, 0xce, 0x09, 0x75, 0x7f, 0x01, 0x5c, 0xc7, 0x0c, 0x60, 0xa3, 0x96, 0x36,
    0x62, 0x71, 0xf7, 0x30, 0xfd, 0x8f, 0x4f, 0x08, 0x6f, 0xe6, 0x0b, 0xda, 0x16, 0x3f, 0x84, 0x06, 0xed, 0xd3, 0xf5, 0x99, 0xed,
    0x04, 0x72, 0x00, 0x55, 0x80, 0x5d, 0x54, 0x4a, 0x41, 0xcb, 0x44, 0x37, 0x00, 0x9c, 0x89, 0x0b, 0xa4, 0xba, 0xe7, 0x87, 0xd6,
    0xa7, 0x32, 0x07, 0x92, 0x4e, 0xd8, 0x3d, 0x5e, 0xb2, 0xe0, 0x49, 0xc6, 0xda, 0xc1, 0x74, 0x5f, 0x3e, 0x4d, 0x02, 0x72, 0x86,
    0x2c, 0x0d, 0x0a, 0xf7, 0xce, 0x0e, 0xb7, 0x89, 0xc2, 0x92, 0xa3, 0x30, 0x1e, 0x65, 0x76, 0x05, 0x34, 0xb4, 0x29, 0x4e, 0x0a,
    0xe7, 0x4c, 0xf0, 0x51, 0xd8, 0xfd, 0x12, 0x82, 0x9e, 0xbf, 0xbd, 0x1b, 0xbd, 0x79, 0x76, 0x5d, 0xb9, 0x81, 0x64, 0xaf, 0x9b,
    0xaa, 0x24, 0x5d, 0xa5, 0x43, 0xe5, 0xa1, 0x9f, 0x3e, 0x80, 0xcc, 0x00, 0xed, 0x23, 0xb6, 0x82, 0xd0, 0xfa, 0x66, 0x47, 0x71,
    0x5d, 0x60, 0x98, 0xc6, 0x44, 0xfa, 0xf4, 0xfe, 0xc1, 0x85, 0x38, 0xfc, 0xf8, 0xff, 0xb4, 0x30, 0x08, 0x86, 0xae, 0xf6, 0xca,
    0x3f, 0xda, 0x65, 0xf3, 0x1b, 0x81, 0xcd, 0xf0, 0x78, 0xfc, 0x97, 0x92, 0x56, 0xa2, 0xa8, 0xd6, 0x50, 0x72, 0xb4, 0x53, 0xfd,
    0x41, 0xea, 0xd8, 0xef, 0x29, 0x65, 0x73, 0xf3, 0xe1, 0xe1, 0x0f, 0x57, 0x25, 0x82, 0xfc, 0x0f, 0x3e, 0x30, 0x81, 0xec, 0xa8,
    0xa0, 0x34, 0x26, 0xef, 0x07, 0x32, 0xb9, 0x5a, 0x1a, 0xdb, 0xc9, 0x4e, 0x28, 0x5f, 0xb9, 0xe8, 0x5e, 0xca, 0x11, 0xc6, 0x4b,
    0x4e, 0x73, 0x5b, 0x84, 0xb8, 0x6c, 0x8a, 0x9f, 0xe3, 0xd7, 0x8c, 0xbc, 0xbb, 0x7d, 0xaa, 0xbd, 0xd9, 0xdb, 0x28, 0x06, 0x30,
    0x2f, 0x93, 0x20, 0x68, 0x21, 0x58, 0x5d, 0x64, 0xf4, 0xd7, 0xce, 0xca, 0x7c, 0x40, 0x06, 0x3b, 0x87, 0xdc, 0xbe, 0x3e, 0x1d,
    0xc2, 0xed, 0x46, 0xcd, 0x0a, 0x44, 0x28, 0x2f, 0x78, 0x85, 0x55, 0x86, 0xb3, 0xc0, 0x96, 0xc6, 0xb6, 0x5e, 0x8f, 0xaa, 0x78,
    0x75, 0xe6, 0xbc, 0x8f, 0x90, 0x8d, 0x14, 0xad, 0xca, 0xc7, 0x16, 0xe3, 0x4b, 0xea, 0xac, 0xfd, 0x23, 0xd6, 0xc9, 0xca, 0x40,
    0x0a, 0xfd, 0x72, 0xad, 0xb2, 0x03, 0x21, 0xdb, 0x42, 0x3f, 0x2e, 0x65, 0xca, 0x07, 0xac, 0x81, 0xdf, 0xc4, 0x2b, 0xa8, 0x3c,
    0x70, 0x99, 0xf0, 0x73, 0x5f, 0x99, 0xcf, 0x53, 0x34, 0xf0, 0xcd, 0x50, 0x9c, 0xbe, 0xf0, 0x2f, 0x33, 0x3a, 0x7e, 0x5f, 0x86,
    0xca, 0xe5, 0xe5, 0x10, 0xc4, 0x35, 0xc8, 0xb9, 0x38, 0x8c, 0xa4, 0x95, 0x51, 0xcc, 0xb8, 0x77, 0xf1, 0x39, 0xcc, 0xb3, 0x31,
    0x29, 0xb9, 0x7a, 0x5c, 0x4d, 0x36, 0x04, 0xa7, 0x5c, 0x56, 0x00, 0xae, 0x2b, 0xe2, 0x57, 0x9f, 0xe0, 0x35, 0x57, 0x80, 0x15,
    0x02, 0x17, 0xfd, 0xde, 0xe7, 0x1c, 0x49, 0xe6, 0xda, 0x9f, 0xa9, 0xa9, 0x1d, 0x20, 0x4e, 0xf5, 0x82, 0xf1, 0x1f, 0xaa, 0xd2,
    0x6b, 0x35, 0x4d, 0xda, 0xe2, 0xea, 0xd5, 0xc0, 0xa7, 0x1b, 0x1a, 0xcd, 0x8d, 0x8f, 0xde, 0x25, 0x6a, 0x18, 0x7c, 0xa5, 0xc3,
    0xcc, 0x46, 0x16, 0x9a, 0x7e, 0x48, 0x84, 0x8e, 0xa6, 0xdc, 0x84, 0x51, 0x36, 0x01, 0x8c, 0xaf, 0x04, 0xd7, 0x80, 0xbd, 0x88,
    0x36, 0xe3, 0x2c, 0x32, 0xce, 0xb7, 0xc0, 0x14, 0xd2, 0x18
};

auto SeededEncodedSigma3()
{
    std::vector<uint8_t> EncodedSigma3Vector(std::begin(FuzzSeed_EncodedSigma3_Node01_02_Chip),
                                             std::end(FuzzSeed_EncodedSigma3_Node01_02_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ EncodedSigma3Vector });
}

FUZZ_TEST(FuzzCASE, ParseSigma3_RawPayload)
    .WithDomains(
        // Fuzzing a fully encoded Sigma3, to pass to ParseSigma3
        SeededEncodedSigma3());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

CHIP_ERROR EncodeSigma3Helper(const vector<uint8_t> & fuzzEncrypted3, MutableByteSpan & encodedSpan)
{

    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    TLVWriter tlvWriter;
    tlvWriter.Init(encodedSpan);

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), fuzzEncrypted3.data(), fuzzEncrypted3.size()));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));

    encodedSpan.reduce_size(tlvWriter.GetLengthWritten());
    ReturnErrorOnFailure(tlvWriter.Finalize());

    return CHIP_NO_ERROR;
}

/**
 * FuzzTest property function: Tests CASESession::ParseSigma3 ability to handle well-formed Sigma3 messages by:
 * 1. Constructing a Sigma3 message from fuzzed individual components
 * 2. Encoding it using EncodeSigma3Helper to create a properly structured payload
 * 3. Parsing the encoded message to check for crashes or undefined behavior
 *
 * This test ensures the parser can handle various combinations of valid field values
 * without crashing, even if the field contents are unusual or edge cases.
 */

void ParseSigma3_StructuredPayload(const vector<uint8_t> & fuzzEncrypted3)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        // Construct Sigma3
        size_t dataLen = TLV::EstimateStructOverhead(fuzzEncrypted3.size());
        Platform::ScopedMemoryBuffer<uint8_t> mem;
        ASSERT_TRUE(mem.Calloc(dataLen));
        MutableByteSpan encodedSpan(mem.Get(), dataLen);

        if (CHIP_NO_ERROR != EncodeSigma3Helper(fuzzEncrypted3, encodedSpan))
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        // Prepare Sigma3 Parsing
        TLV::ContiguousBufferTLVReader tlvReader;
        tlvReader.Init(encodedSpan);
        Platform::ScopedMemoryBufferWithSize<uint8_t> outMsgR3Encrypted;
        MutableByteSpan outMsgR3EncryptedPayload;
        ByteSpan outMsgR3MIC;

        // Parse Sigma3
        FuzzCASESession::ParseSigma3(tlvReader, outMsgR3Encrypted, outMsgR3EncryptedPayload, outMsgR3MIC);

        mem.Free();
    }
    Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzCASE, ParseSigma3_StructuredPayload)
    .WithDomains(
        // Encrypted3 .WithSize(kSigmaParamRandomNumberSize)
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::ParseSigma3TBEData robustness against malformed or arbitrary Sigma3TBEData
 * payloads by:
 * 1. Taking a valid fully-encoded Sigma3TBEData byte array as a seed and feeding it to the FuzzTest
 * 2. Allow the Fuzzing Engine to mutate it as it sees fit (behind the scenes)
 * 3. Directly parsing it
 *
 * Starting from a valid payload seed ensures the fuzzer explores mutations around
 * well-formed messages, increasing the likelihood of finding edge cases errors in the parser.
 */

void ParseSigma3TBEData_RawPayload(const vector<uint8_t> & fuzzEncodedSigma3TBEData)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {

        PacketBufferHandle EncodedSigma3 =
            PacketBufferHandle::NewWithData(fuzzEncodedSigma3TBEData.data(), fuzzEncodedSigma3TBEData.size(), 0, 38);
        PacketBufferTLVReader tlvReader2;
        tlvReader2.Init(std::move(EncodedSigma3));

        FuzzCASESession::HandleSigma3Data unused;
        FuzzCASESession::ParseSigma3TBEData(tlvReader2, unused);
        // std::cout << "Seeded Encoded Sigma3TBEData:" << err.Format() << std::endl;
    }

    Platform::MemoryShutdown();
}

// These messages are extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic inputs.
// TODO #37654: Replace this extracted data with official test vectors when available
uint8_t FuzzSeed_EncodedSigma3TBE_Node01_02_Chip[] = {
    0x15, 0x31, 0x01, 0x0d, 0x01, 0x15, 0x30, 0x01, 0x08, 0x0d, 0x90, 0x93, 0x53, 0x46, 0xb0, 0x5c, 0xbc, 0x24, 0x02, 0x01, 0x37,
    0x03, 0x27, 0x14, 0x01, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x26, 0x04, 0xef, 0x17, 0x1b, 0x27, 0x26, 0x05, 0x6e,
    0xb5, 0xb9, 0x4c, 0x37, 0x06, 0x27, 0x11, 0x02, 0x00, 0x01, 0x00, 0xde, 0xde, 0xde, 0xde, 0x27, 0x15, 0x1d, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xb0, 0xfa, 0x18, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01, 0x30, 0x09, 0x41, 0x04, 0x96, 0x5f, 0x78, 0xc5, 0x37, 0xec,
    0xe1, 0xb8, 0xc3, 0x4a, 0x7b, 0x98, 0xb9, 0xaa, 0x45, 0xf1, 0x35, 0x63, 0xa5, 0x02, 0xb1, 0x97, 0x9a, 0x60, 0x7b, 0xd0, 0xc4,
    0x19, 0x88, 0xbd, 0xd0, 0xf0, 0xbb, 0xb8, 0x98, 0x16, 0xc2, 0x07, 0xe3, 0xb5, 0x15, 0xd9, 0x26, 0x41, 0x59, 0xf7, 0x8b, 0xd0,
    0x97, 0x8e, 0x32, 0xd7, 0x4c, 0x6d, 0x05, 0x5a, 0x14, 0x9e, 0x8e, 0x9d, 0xba, 0x40, 0x19, 0xbf, 0x37, 0x0a, 0x35, 0x01, 0x28,
    0x01, 0x18, 0x24, 0x02, 0x01, 0x36, 0x03, 0x04, 0x02, 0x04, 0x01, 0x18, 0x30, 0x04, 0x14, 0x56, 0x7b, 0x4f, 0x20, 0xe4, 0xb9,
    0xc7, 0xbd, 0x27, 0xb2, 0x9b, 0x3d, 0xce, 0x6a, 0x76, 0xf7, 0xcd, 0x8e, 0xcc, 0xb6, 0x30, 0x05, 0x14, 0xcc, 0x13, 0x08, 0xaf,
    0x82, 0xcf, 0xee, 0x50, 0x5e, 0xb2, 0x3b, 0x57, 0xbf, 0xe8, 0x6a, 0x31, 0x16, 0x65, 0x53, 0x5f, 0x18, 0x30, 0x0b, 0x40, 0x60,
    0x58, 0x11, 0x4b, 0xa7, 0x21, 0x82, 0xfc, 0xf6, 0x30, 0x1f, 0x7a, 0x08, 0x1b, 0xca, 0x5a, 0x84, 0x82, 0x02, 0x43, 0x1a, 0x52,
    0xfd, 0xbf, 0xf4, 0x97, 0xd8, 0xdd, 0x6f, 0x9a, 0x59, 0x59, 0x7b, 0xad, 0xcc, 0xd6, 0xa5, 0x6d, 0x70, 0xef, 0xd8, 0xc9, 0x7c,
    0x49, 0x6e, 0xba, 0x7e, 0x28, 0x01, 0xd7, 0x33, 0x7d, 0xcf, 0xf7, 0x4d, 0x78, 0xe4, 0x6e, 0xcd, 0x3a, 0x08, 0xcc, 0xba, 0xe3,
    0x18, 0x30, 0x02, 0xfc, 0x15, 0x30, 0x01, 0x08, 0x69, 0xd8, 0x6a, 0x8d, 0x80, 0xfc, 0x8f, 0x5d, 0x24, 0x02, 0x01, 0x37, 0x03,
    0x27, 0x14, 0x01, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x26, 0x04, 0xef, 0x17, 0x1b, 0x27, 0x26, 0x05, 0x6e, 0xb5,
    0xb9, 0x4c, 0x37, 0x06, 0x27, 0x13, 0x03, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01,
    0x30, 0x09, 0x41, 0x04, 0x5f, 0x94, 0xf5, 0x7e, 0x0b, 0x13, 0xc9, 0xcf, 0xcf, 0x96, 0xdf, 0xe1, 0xfc, 0xe7, 0x88, 0x8d, 0x56,
    0x4c, 0xc2, 0x09, 0xc5, 0x5c, 0x45, 0x08, 0xe4, 0x4d, 0xcf, 0x16, 0xba, 0x2e, 0x09, 0x66, 0x2f, 0x9e, 0xec, 0xf1, 0x9f, 0x40,
    0xb0, 0xe8, 0x8a, 0x0b, 0x28, 0x15, 0xda, 0x9e, 0xe1, 0x0a, 0x3a, 0x17, 0x7c, 0x25, 0x1f, 0x43, 0x4f, 0x5b, 0x0f, 0x26, 0x3c,
    0xe7, 0xde, 0x62, 0x78, 0xc6, 0x37, 0x0a, 0x35, 0x01, 0x29, 0x01, 0x18, 0x24, 0x02, 0x60, 0x30, 0x04, 0x14, 0x44, 0x0c, 0xc6,
    0x92, 0x31, 0xc4, 0xcb, 0x5b, 0x37, 0x94, 0x24, 0x26, 0xf8, 0x1b, 0xbe, 0x24, 0xb7, 0xef, 0x34, 0x5c, 0x30, 0x05, 0x14, 0xcc,
    0x13, 0x08, 0xaf, 0x82, 0xcf, 0xee, 0x50, 0x5e, 0xb2, 0x3b, 0x57, 0xbf, 0xe8, 0x6a, 0x31, 0x16, 0x65, 0x53, 0x5f, 0x18, 0x30,
    0x0b, 0x40, 0xad, 0xb8, 0x5b, 0x5d, 0x68, 0xcb, 0xfd, 0x36, 0x14, 0x0d, 0x8c, 0x9d, 0x12, 0x90, 0x14, 0xc4, 0x5f, 0xa7, 0xca,
    0x19, 0x1f, 0x34, 0xd9, 0xaf, 0x24, 0x1d, 0xb7, 0x17, 0x36, 0xe6, 0x0f, 0x44, 0x19, 0x9b, 0xc0, 0x7c, 0x7f, 0x79, 0x5b, 0xed,
    0x81, 0xa2, 0xe7, 0x7d, 0xc5, 0x34, 0x25, 0x76, 0xf6, 0xa0, 0xd1, 0x41, 0x98, 0xf4, 0x6b, 0x91, 0x07, 0x49, 0x42, 0x7c, 0x2e,
    0xed, 0x65, 0x9c, 0x18, 0x30, 0x03, 0x40, 0x63, 0xb9, 0x2a, 0x76, 0xa1, 0x0b, 0x3e, 0x1c, 0x9a, 0xb5, 0xcf, 0xc3, 0x72, 0x9b,
    0x59, 0xce, 0xca, 0x6d, 0xec, 0xfb, 0x4d, 0x0a, 0xc8, 0xca, 0x09, 0x60, 0x67, 0xc9, 0xd4, 0xe0, 0xad, 0xc9, 0x6a, 0x1c, 0x52,
    0x72, 0x54, 0xc0, 0xaf, 0xa1, 0xd1, 0xf2, 0x79, 0x82, 0x2b, 0xec, 0x51, 0xd4, 0xa2, 0x3e, 0x9c, 0xbf, 0x8f, 0x2c, 0xc9, 0x84,
    0x82, 0x63, 0x73, 0x3e, 0xcf, 0xe1, 0xc1, 0xc1, 0x18
};

auto SeededEncodedSigma3TBE()
{
    std::vector<uint8_t> EncodedSigma3TBEVector(std::begin(FuzzSeed_EncodedSigma3TBE_Node01_02_Chip),
                                                std::end(FuzzSeed_EncodedSigma3TBE_Node01_02_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ EncodedSigma3TBEVector });
}

FUZZ_TEST(FuzzCASE, ParseSigma3TBEData_RawPayload).WithDomains(SeededEncodedSigma3TBE());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

CHIP_ERROR EncodeSigma3TBEDataHelper(const vector<uint8_t> & fuzzInitiatorNOC, const vector<uint8_t> & fuzzInitiatorICAC,
                                     const vector<uint8_t> & fuzzSignature, MutableByteSpan & outEncodedSpan)
{
    TLV::TLVType outerContainerType = TLV::kTLVType_NotSpecified;
    TLVWriter tlvWriter;
    tlvWriter.Init(outEncodedSpan);

    ReturnErrorOnFailure(tlvWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(1), fuzzInitiatorNOC.data(), fuzzInitiatorNOC.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(2), fuzzInitiatorICAC.data(), fuzzInitiatorICAC.size()));
    ReturnErrorOnFailure(tlvWriter.PutBytes(TLV::ContextTag(3), fuzzSignature.data(), fuzzSignature.size()));
    ReturnErrorOnFailure(tlvWriter.EndContainer(outerContainerType));

    outEncodedSpan.reduce_size(tlvWriter.GetLengthWritten());
    ReturnErrorOnFailure(tlvWriter.Finalize());

    return CHIP_NO_ERROR;
}

/**
 * FuzzTest property function: Tests CASESession::ParseSigma3TBEData ability to handle well-formed Sigma3TBEData messages by:
 * 1. Constructing a Sigma3TBEData message from fuzzed individual components
 * 2. Encoding it using EncodeSigma3TBEDataHelper to create a properly structured payload
 * 3. Parsing the encoded message to check for crashes or undefined behavior
 *
 * This test ensures the parser can handle various combinations of valid field values
 * without crashing, even if the field contents are unusual or edge cases.
 */

void ParseSigma3TBEData_StructuredPayload(const vector<uint8_t> & fuzzInitiatorNOC, const vector<uint8_t> & fuzzInitiatorICAC,
                                          const vector<uint8_t> & fuzzSignature)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
    {
        // ==== Construct Sigma3TBEData ====
        size_t dataLen = TLV::EstimateStructOverhead(fuzzInitiatorNOC.size(),  // responderNOC
                                                     fuzzInitiatorICAC.size(), // responderICAC
                                                     fuzzSignature.size()      // signature
        );

        Platform::ScopedMemoryBuffer<uint8_t> mem;
        ASSERT_TRUE(mem.Calloc(dataLen));
        MutableByteSpan encodedSpan(mem.Get(), dataLen);

        if (CHIP_NO_ERROR != EncodeSigma3TBEDataHelper(fuzzInitiatorNOC, fuzzInitiatorICAC, fuzzSignature, encodedSpan))
        {
            // Returning immediately will signal to the fuzzing engine that the given
            // input was uninteresting, and should not be added to the corpus.
            return;
        }

        // ==== Parse Sigma 3 ====
        TLV::ContiguousBufferTLVReader tlvReader;
        tlvReader.Init(encodedSpan);

        FuzzCASESession::HandleSigma3Data unused;
        FuzzCASESession::ParseSigma3TBEData(tlvReader, unused);
        // std::cout << err.Format() << std::endl;

        mem.Free();
    }

    Platform::MemoryShutdown();
}

FUZZ_TEST(FuzzCASE, ParseSigma3TBEData_StructuredPayload)
    .WithDomains(
        // initiatorNOC (Max size = Credentials::kMaxCHIPCertLength)
        Arbitrary<vector<uint8_t>>(),
        // initiatorICAC (Max size = Credentials::kMaxCHIPCertLength)
        Arbitrary<vector<uint8_t>>(),
        // Signature
        Arbitrary<vector<uint8_t>>());

/*------------------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------------------*/

/**
 * FuzzTest property function: Tests CASESession::HandleSigma3b robustness against malformed or arbitrary inputs.
 *
 * CASESession::HandleSigma3b has two important functions: Verifying Credentials and Validating Signature.
 *
 * To Test it we:
 * 1. Take valid fully-encoded NOC, ICAC and RCAC Certs as seeds and feed them to the FuzzTest.
 * 2. Also take valid fully-encoded MsgR3Signed and Tbs3Signature that correspond to the Seeded Certs above
 * 3. Allow the Fuzzing Engine to mutate all these inputs as it sees fit (behind the scenes)
 * 4. Input all these fuzzed payloads to CASESession::HandleSigma3b
 *
 */

void HandleSigma3b(const vector<uint8_t> & fuzzInitiatorNOC, const vector<uint8_t> & fuzzInitiatorICAC,
                   const vector<uint8_t> & fuzzFabricRCAC, const vector<uint8_t> & fuzzMsg3Signed,
                   const vector<uint8_t> & fuzzTbs3Signature, FabricId fuzzFabricId, const ValidationContext & fuzzValidContext)
{
    ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);

    {
        /************ Prepare HandleSigma3Data Struct to feed it into CASESession::HandleSigma3b *************/
        FuzzCASESession::HandleSigma3Data data;
        data.initiatorNOC  = ByteSpan(fuzzInitiatorNOC.data(), fuzzInitiatorNOC.size());
        data.initiatorICAC = ByteSpan(fuzzInitiatorICAC.data(), fuzzInitiatorICAC.size());
        data.fabricRCAC    = ByteSpan(fuzzFabricRCAC.data(), fuzzFabricRCAC.size());
        data.fabricId      = fuzzFabricId;
        data.validContext  = fuzzValidContext;

        // prepare the fuzzed Signed Sigma3 Message
        data.msgR3Signed.Alloc(fuzzMsg3Signed.size());
        memcpy(data.msgR3Signed.Get(), fuzzMsg3Signed.data(), fuzzMsg3Signed.size());
        data.msgR3SignedSpan = MutableByteSpan{ data.msgR3Signed.Get(), fuzzMsg3Signed.size() };

        // prepare the fuzzed signature
        data.tbsData3Signature.SetLength(fuzzTbs3Signature.size());
        memcpy(data.tbsData3Signature.Bytes(), fuzzTbs3Signature.data(), fuzzTbs3Signature.size());

        /************ Fuzz CASESession::HandleSigma3b *************/
        bool unused = false;
        FuzzCASESession::HandleSigma3b(data, unused);
        // std::cout << "fuzzed HandleSigma3b: " << err.Format() << std::endl;
    }

    Platform::MemoryShutdown();
}

// These messages are extracted from unit tests to serve as fuzzing seeds, allowing the fuzzer to start with realistic inputs.
// TODO #37654: Replace this extracted data with official test vectors when available

uint8_t FuzzSeed_MsgR3Signed_Node01_01_Chip[] = {
    0x15, 0x31, 0x01, 0x0d, 0x01, 0x15, 0x30, 0x01, 0x08, 0x18, 0xe9, 0x69, 0xba, 0x0e, 0x08, 0x9e, 0x23, 0x24, 0x02, 0x01, 0x37,
    0x03, 0x27, 0x13, 0x03, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x26, 0x04, 0xef, 0x17, 0x1b, 0x27, 0x26, 0x05, 0x6e,
    0xb5, 0xb9, 0x4c, 0x37, 0x06, 0x27, 0x11, 0x01, 0x00, 0x01, 0x00, 0xde, 0xde, 0xde, 0xde, 0x27, 0x15, 0x1d, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xb0, 0xfa, 0x18, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01, 0x30, 0x09, 0x41, 0x04, 0xbc, 0xf6, 0x58, 0x0d, 0x2d, 0x71,
    0xe1, 0x44, 0x16, 0x65, 0x1f, 0x7c, 0x31, 0x1b, 0x5e, 0xfc, 0xf9, 0xae, 0xc0, 0xa8, 0xc1, 0x0a, 0xf8, 0x09, 0x27, 0x84, 0x4c,
    0x24, 0x0f, 0x51, 0xa8, 0xeb, 0x23, 0xfa, 0x07, 0x44, 0x13, 0x88, 0x87, 0xac, 0x1e, 0x73, 0xcb, 0x72, 0xa0, 0x54, 0xb6, 0xa0,
    0xdb, 0x06, 0x22, 0xaa, 0x80, 0x70, 0x71, 0x01, 0x63, 0x13, 0xb1, 0x59, 0x6c, 0x85, 0x52, 0xcf, 0x37, 0x0a, 0x35, 0x01, 0x28,
    0x01, 0x18, 0x24, 0x02, 0x01, 0x36, 0x03, 0x04, 0x02, 0x04, 0x01, 0x18, 0x30, 0x04, 0x14, 0x69, 0x67, 0xc9, 0x12, 0xf8, 0xa3,
    0xe6, 0x89, 0x55, 0x6f, 0x89, 0x9b, 0x65, 0xd7, 0x6f, 0x53, 0xfa, 0x65, 0xc7, 0xb6, 0x30, 0x05, 0x14, 0x44, 0x0c, 0xc6, 0x92,
    0x31, 0xc4, 0xcb, 0x5b, 0x37, 0x94, 0x24, 0x26, 0xf8, 0x1b, 0xbe, 0x24, 0xb7, 0xef, 0x34, 0x5c, 0x18, 0x30, 0x0b, 0x40, 0xce,
    0x6e, 0xf3, 0x93, 0xcb, 0xbc, 0x94, 0xf8, 0x0e, 0xe2, 0x90, 0xcb, 0x3c, 0x3d, 0x37, 0x33, 0x35, 0xba, 0xb9, 0x59, 0x07, 0x73,
    0x4d, 0x99, 0xd3, 0x84, 0xa6, 0x2a, 0x37, 0x3b, 0x84, 0x84, 0xe1, 0xd4, 0x1a, 0x04, 0xc3, 0x14, 0x0f, 0xaa, 0x19, 0xe8, 0xa2,
    0xb9, 0x9b, 0x0c, 0x61, 0xe3, 0x3c, 0x27, 0xea, 0x91, 0x39, 0x73, 0xe4, 0x5b, 0x5b, 0xc6, 0xe3, 0x9c, 0x27, 0x0d, 0xac, 0x53,
    0x18, 0x30, 0x02, 0xfc, 0x15, 0x30, 0x01, 0x08, 0x69, 0xd8, 0x6a, 0x8d, 0x80, 0xfc, 0x8f, 0x5d, 0x24, 0x02, 0x01, 0x37, 0x03,
    0x27, 0x14, 0x01, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x26, 0x04, 0xef, 0x17, 0x1b, 0x27, 0x26, 0x05, 0x6e, 0xb5,
    0xb9, 0x4c, 0x37, 0x06, 0x27, 0x13, 0x03, 0x00, 0x00, 0x00, 0xca, 0xca, 0xca, 0xca, 0x18, 0x24, 0x07, 0x01, 0x24, 0x08, 0x01,
    0x30, 0x09, 0x41, 0x04, 0x5f, 0x94, 0xf5, 0x7e, 0x0b, 0x13, 0xc9, 0xcf, 0xcf, 0x96, 0xdf, 0xe1, 0xfc, 0xe7, 0x88, 0x8d, 0x56,
    0x4c, 0xc2, 0x09, 0xc5, 0x5c, 0x45, 0x08, 0xe4, 0x4d, 0xcf, 0x16, 0xba, 0x2e, 0x09, 0x66, 0x2f, 0x9e, 0xec, 0xf1, 0x9f, 0x40,
    0xb0, 0xe8, 0x8a, 0x0b, 0x28, 0x15, 0xda, 0x9e, 0xe1, 0x0a, 0x3a, 0x17, 0x7c, 0x25, 0x1f, 0x43, 0x4f, 0x5b, 0x0f, 0x26, 0x3c,
    0xe7, 0xde, 0x62, 0x78, 0xc6, 0x37, 0x0a, 0x35, 0x01, 0x29, 0x01, 0x18, 0x24, 0x02, 0x60, 0x30, 0x04, 0x14, 0x44, 0x0c, 0xc6,
    0x92, 0x31, 0xc4, 0xcb, 0x5b, 0x37, 0x94, 0x24, 0x26, 0xf8, 0x1b, 0xbe, 0x24, 0xb7, 0xef, 0x34, 0x5c, 0x30, 0x05, 0x14, 0xcc,
    0x13, 0x08, 0xaf, 0x82, 0xcf, 0xee, 0x50, 0x5e, 0xb2, 0x3b, 0x57, 0xbf, 0xe8, 0x6a, 0x31, 0x16, 0x65, 0x53, 0x5f, 0x18, 0x30,
    0x0b, 0x40, 0xad, 0xb8, 0x5b, 0x5d, 0x68, 0xcb, 0xfd, 0x36, 0x14, 0x0d, 0x8c, 0x9d, 0x12, 0x90, 0x14, 0xc4, 0x5f, 0xa7, 0xca,
    0x19, 0x1f, 0x34, 0xd9, 0xaf, 0x24, 0x1d, 0xb7, 0x17, 0x36, 0xe6, 0x0f, 0x44, 0x19, 0x9b, 0xc0, 0x7c, 0x7f, 0x79, 0x5b, 0xed,
    0x81, 0xa2, 0xe7, 0x7d, 0xc5, 0x34, 0x25, 0x76, 0xf6, 0xa0, 0xd1, 0x41, 0x98, 0xf4, 0x6b, 0x91, 0x07, 0x49, 0x42, 0x7c, 0x2e,
    0xed, 0x65, 0x9c, 0x18, 0x30, 0x03, 0x41, 0x04, 0xdd, 0x9c, 0xaa, 0x9b, 0x90, 0x8f, 0xf9, 0x3f, 0x5a, 0x7d, 0xf4, 0xb7, 0xed,
    0xe4, 0xef, 0x78, 0x7d, 0x68, 0x1c, 0xa6, 0xcd, 0x8c, 0xf3, 0x4d, 0x0d, 0x23, 0x0f, 0x73, 0xad, 0xb8, 0xfc, 0x37, 0x4b, 0x38,
    0x4f, 0xe7, 0xab, 0x5b, 0xd7, 0x53, 0x87, 0x0a, 0xb1, 0x1f, 0x34, 0x8c, 0xdb, 0x2e, 0x72, 0xe8, 0x73, 0x24, 0x2d, 0x92, 0xf1,
    0x18, 0x05, 0xb4, 0xc9, 0xe2, 0x06, 0x9b, 0x8b, 0xfa, 0x30, 0x04, 0x41, 0x04, 0x77, 0xde, 0xea, 0xfd, 0xf3, 0x7f, 0xc5, 0xc2,
    0x4e, 0xc5, 0xab, 0x25, 0x88, 0xf7, 0x73, 0xd7, 0x25, 0x90, 0xc6, 0x28, 0xba, 0x64, 0x86, 0xd3, 0xc3, 0xf7, 0xb3, 0x16, 0x47,
    0x45, 0x87, 0xe9, 0x32, 0x8b, 0xdc, 0xf0, 0x3f, 0x51, 0xda, 0xaa, 0x73, 0xb6, 0xf0, 0xef, 0x1a, 0xb7, 0x4d, 0xd9, 0xe7, 0x23,
    0xaa, 0xb1, 0x8a, 0x4b, 0x6d, 0xbc, 0x92, 0x55, 0xcf, 0xac, 0x34, 0x64, 0xd8, 0x59, 0x18

};

uint8_t FuzzSeed_Tbs3Signature_Node01_01_Chip[] = { 0xa1, 0x4b, 0x51, 0xea, 0x18, 0x8f, 0x6c, 0xac, 0x16, 0xfa, 0x52, 0xef, 0xc7,
                                                    0x3c, 0x0a, 0x57, 0x71, 0x6b, 0x78, 0xb9, 0xae, 0x73, 0x5e, 0xcd, 0xce, 0xcd,
                                                    0x53, 0x6e, 0x3f, 0xf7, 0xbb, 0x2b, 0xd1, 0xf4, 0x59, 0xba, 0x0b, 0xbc, 0x45,
                                                    0x32, 0xed, 0x19, 0x24, 0xe1, 0xa6, 0xdb, 0x2a, 0x9a, 0xba, 0xbb, 0xa2, 0x4d,
                                                    0xcf, 0xd5, 0xc6, 0xa6, 0x09, 0x55, 0x37, 0x36, 0x27, 0x72, 0xff, 0x19

};

// Use corresponding Test Vectors as "Fuzzing Seeds" for NOC, ICAC, RCAC and FabricID (with the Root01:ICA01:Node01_01 identity)
auto initiatorNOC()
{
    std::vector<uint8_t> data(TestCerts::sTestCert_Node01_01_Chip.data(),
                              TestCerts::sTestCert_Node01_01_Chip.data() + TestCerts::sTestCert_Node01_01_Chip.size());
    return Arbitrary<vector<uint8_t>>().WithSeeds({ data });
}

auto initiatorICAC()
{
    std::vector<uint8_t> data(TestCerts::sTestCert_ICA01_Chip.data(),
                              TestCerts::sTestCert_ICA01_Chip.data() + TestCerts::sTestCert_ICA01_Chip.size());
    return Arbitrary<vector<uint8_t>>().WithSeeds({ data });
}

auto fabricRCAC()
{
    std::vector<uint8_t> data(TestCerts::sTestCert_Root01_Chip.data(),
                              TestCerts::sTestCert_Root01_Chip.data() + TestCerts::sTestCert_Root01_Chip.size());
    return Arbitrary<vector<uint8_t>>().WithSeeds({ data });
}

auto AnyFabricId()
{
    return Arbitrary<FabricId>().WithSeeds({ TestCerts::kTestCert_Node01_01_FabricId });
}

// Use valid MsgR3Signed and Tbs3Signature that correspond to the Root01:ICA01:Node01_01 identity as "Fuzzing Seeds".
// This means that in some of the interations, we will end up successfully validating the signature.
auto fuzzMsgR3Signed()
{
    std::vector<uint8_t> data(FuzzSeed_MsgR3Signed_Node01_01_Chip,
                              FuzzSeed_MsgR3Signed_Node01_01_Chip + sizeof(FuzzSeed_MsgR3Signed_Node01_01_Chip));

    return Arbitrary<vector<uint8_t>>().WithSeeds({ data });
}

auto fuzzTbs3Signature()
{
    std::vector<uint8_t> data(FuzzSeed_Tbs3Signature_Node01_01_Chip,
                              FuzzSeed_Tbs3Signature_Node01_01_Chip + sizeof(FuzzSeed_Tbs3Signature_Node01_01_Chip));

    return Arbitrary<vector<uint8_t>>().WithSize(kMax_ECDSA_Signature_Length).WithSeeds({ data });
}

auto AnyValidationContext()
{
    // Defining Domains to Pass to Map
    auto requiredKeyUsages   = ElementOf<KeyUsageFlags>({
        KeyUsageFlags::kDigitalSignature,
        KeyUsageFlags::kNonRepudiation,
        KeyUsageFlags::kKeyEncipherment,
        KeyUsageFlags::kDataEncipherment,
        KeyUsageFlags::kKeyAgreement,
        KeyUsageFlags::kKeyCertSign,
        KeyUsageFlags::kCRLSign,
        KeyUsageFlags::kEncipherOnly,
        KeyUsageFlags::kDecipherOnly,
    });
    auto requiredKeyPurposes = ElementOf<KeyPurposeFlags>({ KeyPurposeFlags::kServerAuth, KeyPurposeFlags::kClientAuth,
                                                            KeyPurposeFlags::kCodeSigning, KeyPurposeFlags::kEmailProtection,
                                                            KeyPurposeFlags::kTimeStamping, KeyPurposeFlags::kOCSPSigning });
    auto requiredCertType    = ElementOf<CertType>({ CertType::kNotSpecified, CertType::kRoot, CertType::kICA, CertType::kNode,
                                                     CertType::kFirmwareSigning, CertType::kNetworkIdentity });
    return Map(
        [](const auto & keyUsages, const auto & keyPurposes, const auto & certType) {
            ValidationContext fuzzValidationContext;

            fuzzValidationContext.Reset();
            fuzzValidationContext.mRequiredKeyUsages.Set(keyUsages);
            fuzzValidationContext.mRequiredKeyPurposes.Set(keyPurposes);
            fuzzValidationContext.mRequiredCertType = certType;

            return fuzzValidationContext;
        },
        requiredKeyUsages, requiredKeyPurposes, requiredCertType);
}

FUZZ_TEST(FuzzCASE, HandleSigma3b)
    .WithDomains(
        // initiatorNOC (Max size = Credentials::kMaxCHIPCertLength)
        initiatorNOC(),
        // initiatorICAC (Max size = Credentials::kMaxCHIPCertLength)
        initiatorICAC(),
        // fabricRCAC
        fabricRCAC(),
        // Seeded Signed Sigma3
        fuzzMsgR3Signed(),
        // Seeded Signature (Original size = kMax_ECDSA_Signature_Length)
        fuzzTbs3Signature(),
        // Any Fabric ID (Seeded)
        AnyFabricId(),
        // Any Validation Context
        AnyValidationContext());

} // namespace Testing
} // namespace chip
