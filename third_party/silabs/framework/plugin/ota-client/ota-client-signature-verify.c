/**
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Verification code for the ZigBee over-the-air bootload cluster.
 * This handles retrieving the stored application from storage,
 * calculating the message digest, and checking the signature.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"

#if defined(EZSP_HOST)
  #include "stack/include/ember-types.h"
  #include "stack/include/error.h"
  #include "hal/hal.h"
  #include "app/util/ezsp/ezsp-protocol.h"
  #include "app/util/ezsp/ezsp.h"
  #include "stack/include/library.h"

  #define emberDsaVerify283k1 ezspDsaVerify283k1
#else
  #include "stack/include/cbke-crypto-engine.h"
  #include "stack/include/library.h"
#endif

#include "app/framework/security/crypto-state.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/plugin/ota-client/ota-client.h"
#include "app/framework/plugin/ota-client-policy/ota-client-policy.h"
#include "ota-client-signature-verify.h"

#if defined(EMBER_AF_PLUGIN_OTA_CLIENT_SIGNATURE_VERIFICATION_SUPPORT)

//------------------------------------------------------------------------------
// Globals
//------------------------------------------------------------------------------

// The size of this buffer is based on the max size that EZSP can handle,
// including framing.
// This MUST be a multiple of 16, because the emberAesMmmoHashUpdate()
// code requires this.
#define MAX_BLOCK_SIZE_FOR_HASH 96

#define MAX_SIGNERS 3
static const uint8_t allowedSignerEuiBigEndian[MAX_SIGNERS][EUI64_SIZE] = {
  EMBER_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI0,
  EMBER_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI1,
  EMBER_AF_PLUGIN_OTA_CLIENT_SIGNER_EUI2,
};

#define SUBJECT_OFFSET_163K1  (EMBER_PUBLIC_KEY_SIZE)
#define SUBJECT_OFFSET_283K1  28
#define ISSUER_OFFSET_163K1   (SUBJECT_OFFSET_163K1 + EUI64_SIZE)
#define ISSUER_OFFSET_283K1   11

#define DIGEST_CALCULATE_PRINT_UPDATE_RATE  5

// TODO:  This is the data that we keep track of while verification is in
// progress.  It consumes a bit of RAM that is rarely used.  It would be
// ideal on the SOC to put this in an Ember message buffer.  However
// we must abstract the data storage since it needs to support both SOC and Host
// based apps.
static EmberAesMmoHashContext context;
static uint32_t currentOffset = 0;

typedef enum {
  DIGEST_CALCULATE_COMPLETE    = 0,
  DIGEST_CALCULATE_IN_PROGRESS = 1,
  DIGEST_CALCULATE_ERROR       = 2,
} DigestCalculateStatus;

//------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------

#if defined (EZSP_HOST)
// External
void emberReverseMemCopy(uint8_t* dest, const uint8_t* src, uint16_t length);

// For now we declare these here, but they should be moved to a more appropriate
// header.
void emberAesMmoHashInit(EmberAesMmoHashContext* context);
EmberStatus emberAesMmoHashUpdate(EmberAesMmoHashContext* context,
                                  uint32_t length,
                                  const uint8_t* data);
EmberStatus emberAesMmoHashFinal(EmberAesMmoHashContext* context,
                                 uint32_t length,
                                 const uint8_t* finalData);
#endif // EZSP_HOST

static bool checkSigner(const uint8_t* bigEndianSignerEui64);

static void dsaVerifyHandler(EmberStatus status);

static DigestCalculateStatus calculateDigest(uint16_t maxHashCalculations,
                                             const EmberAfOtaImageId *id,
                                             EmberMessageDigest* digest,
                                             bool is163k1);

//#define DEBUG_DIGEST_PRINT
#if defined(DEBUG_DIGEST_PRINT)
static void debugDigestPrint(const EmberAesMmoHashContext* context);
#else
  #define debugDigestPrint(x)
#endif

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

EmberAfImageVerifyStatus emAfOtaImageSignatureVerify(uint16_t maxHashCalculations,
                                                     const EmberAfOtaImageId* id,
                                                     bool newVerification)
{
  // Smart Energy policy requires verification of the signature of the
  // downloaded file.

  // We only support images that have the embedded certificate in them.
  // However we validate that the certificate only comes from a signer that we
  // know and trust by making sure the hardcoded EUI in the software matches
  // with the EUI in the certificate.
  // The reason for supporting only images with embedded certificates is that
  // it consumes less flash space to keep track of a signer EUI (8 bytes)
  // than to keep track of an entire signer ceritficate (48 bytes).

  // This verification procedure follows 11.7.1 of 15-0324-02

  // 1) Extract the signature of the signer
  //      Verify the signer is known to us
  // 2) Check the ECDSA crypto suites
  //      Verify certificate and signature using same suite
  //      Verify library support for suite present
  // 3) Extract the certificate of the signer
  //      Verify the subject matches the signer of the signature
  // 4) Extract the Certificate Authority public key from the signing cert
  //      Verify the IEEE of the CA is recognized
  // 5) Calculate the message digest of the image
  // 6) Extract the signer's public key from the certificate
  // 7-9) ECDSA verify the calculated digest, signer certificate, and CA pub key

  bool is163k1 = false, is283k1 = false;
  EmberMessageDigest digest;
  uint32_t dataLength;
  uint8_t certificate[EMBER_CERTIFICATE_283K1_SIZE];  // Used for both curves
  uint8_t signature[SIGNATURE_283K1_TAG_DATA_SIZE];   // Used for both curves
  DigestCalculateStatus digestStatus;
  EmberEUI64 signer;
  uint8_t *subject, *issuer, *myIssuer;

  if (newVerification) {
    otaPrintln("Client Verifying Signature.");
    currentOffset = 0;
  }

  bool is163k1DsaVerifyLibraryPresent =
    emberGetLibraryStatus(EMBER_CBKE_DSA_VERIFY_LIBRARY_ID)
    & EMBER_LIBRARY_PRESENT_MASK;
  bool is283k1DsaVerifyLibraryPresent =
    emberGetLibraryStatus(EMBER_CBKE_283K1_DSA_VERIFY_LIBRARY_ID)
    & EMBER_LIBRARY_PRESENT_MASK;

  if (!is163k1DsaVerifyLibraryPresent && !is283k1DsaVerifyLibraryPresent) {
    // We treat this is as a validation error and not a
    // EMBER_AF_NO_IMAGE_VERIFY_SUPPORT error because we want the software
    // to halt the upgrade and discard the image.  If the software
    // has been configured to perform signature verification then
    // it should NOT accept images because it lacks the required
    // libraries to validate them.  Instead, we want it to always fail to
    // upgrade so that this error case can be detected and fixed by
    // developers.
    otaPrintln("Verify Error: No CBKE library present!");
    return EMBER_AF_IMAGE_VERIFY_ERROR;
  }

  // Step 1: extract the signature, check signer

  if ((EMBER_AF_OTA_STORAGE_SUCCESS == emAfOtaStorageGetTagDataFromImage(
         id,
         OTA_TAG_ECDSA_SIGNATURE,
         signature,
         &dataLength,
         SIGNATURE_TAG_DATA_SIZE))
      && (dataLength == SIGNATURE_TAG_DATA_SIZE)) {
    is163k1 = true;
  }
  if ((EMBER_AF_OTA_STORAGE_SUCCESS == emAfOtaStorageGetTagDataFromImage(
         id,
         OTA_TAG_ECDSA_SIGNATURE_283K1,
         signature,
         &dataLength,
         SIGNATURE_283K1_TAG_DATA_SIZE))
      && (dataLength == SIGNATURE_283K1_TAG_DATA_SIZE)) {
    is283k1 = true;
  }

  if (is163k1 == is283k1) {
    otaPrintln("Verify Error: %s",
               (false == is163k1)
               ? "Could not obtain signature from image."
               : "OTA image is double signed with crypto suites 1 and 2");
    return EMBER_AF_IMAGE_VERIFY_ERROR;
  }

  emberReverseMemCopy(signer, signature, EUI64_SIZE);

  if (!checkSigner(signer)) {
    otaPrint("Verify Error: signature's signer EUI in image does not "
             "match any known signer EUI: ");
    emberAfPrint8ByteBlocks(1, signer, false);
    otaPrintln("");
    return EMBER_AF_IMAGE_BAD;
  }

  // Steps 2-3: check the ECDSA crypto suites and extract the certificate

  uint16_t tag     = is163k1 ? OTA_TAG_ECDSA_SIGNING_CERTIFICATE
                     : OTA_TAG_ECDSA_SIGNING_CERTIFICATE_283K1;
  uint32_t tagSize = is163k1 ? EMBER_CERTIFICATE_SIZE
                     : EMBER_CERTIFICATE_283K1_SIZE;

  if (EMBER_AF_OTA_STORAGE_SUCCESS != emAfOtaStorageGetTagDataFromImage(
        id,
        tag,
        certificate,
        &dataLength,
        tagSize)
      || dataLength != tagSize) {
    otaPrintln("Verify Error: Could not obtain signing certificate from image!");
    return EMBER_AF_IMAGE_BAD;
  }

  if (newVerification) {
    otaPrintln("Signer Certificate:");
    emberAfCoreFlush();
    if (is163k1) {
      emberAfPrintCert(certificate);
    } else {
      emberAfPrintCert283k1(certificate);
    }
    emberAfCoreFlush();
  }

  if (is163k1 && !is163k1DsaVerifyLibraryPresent) {
    otaPrintln("Verify Error: No 163k1 CBKE library present!");
    return EMBER_AF_IMAGE_VERIFY_ERROR;
  }

  if (is283k1 && !is283k1DsaVerifyLibraryPresent) {
    otaPrintln("Verify Error: No 283k1 CBKE library present!");
    return EMBER_AF_IMAGE_VERIFY_ERROR;
  }

  // Verify that the signer's EUI64 in the signature tag is the same
  // as the subject EUI64 in the certificate.  This makes sure no one can have
  // a different device sign than is advertised in the "signer" field
  // of the signature tag sub-element.
  subject = is163k1 ? (certificate + SUBJECT_OFFSET_163K1)
            : (certificate + SUBJECT_OFFSET_283K1);
  if (0 != MEMCOMPARE(signer, subject, EUI64_SIZE)) {
    otaPrintln("Verify Error: Certificate EUI and signer EUI do not match!\n");
    return EMBER_AF_IMAGE_BAD;
  }

  // Step 4 (slightly modified): verify the CA IEEE is something we recognize
  //    something we recognize = the same as our CA IEEE

  if (newVerification) {
    EmberCertificateData myCert163k1;
    EmberCertificate283k1Data myCert283k1;

    EmberStatus status = is163k1 ? emberGetCertificate(&myCert163k1)
                         : emberGetCertificate283k1(&myCert283k1);

    if (EMBER_SUCCESS != status) {
      otaPrintln("Verify Error: Could not extract my local certificate");
      return EMBER_AF_IMAGE_BAD;
    }

    // This check ultimately prevents a Test Cert device from being used to
    // sign images for a Production cert device.
    // The CBKE DSA verify will fail later, but this prevents us from getting
    // that far.
    issuer   = is163k1 ? (certificate + ISSUER_OFFSET_163K1)
               : (certificate + ISSUER_OFFSET_283K1);
    myIssuer = is163k1 ? (myCert163k1.contents + ISSUER_OFFSET_163K1)
               : (myCert283k1.contents + ISSUER_OFFSET_283K1);
    if (0 != MEMCOMPARE(issuer, myIssuer, EUI64_SIZE)) {
      otaPrintln("Verify Error: Certificate issuer mismatch.");
      return EMBER_AF_IMAGE_BAD;
    }

    otaPrintln("Signature");
    // This print is 6 bytes too long, but it doesn't matter.  It is just a print
    // The 'emberAfPrintCert()' routines prints 48 bytes but the signature is only 42.
    emberAfOtaBootloadClusterDebugExec(
      is163k1 ? emberAfPrintCert(signature + EUI64_SIZE)
      : emberAfPrintCert283k1(signature + EUI64_SIZE));
  }

  // Steps 5-9: calculate the digest, feed the signer certificate to the verify
  // algorithm

  digestStatus = calculateDigest(maxHashCalculations, id, &digest, is163k1);
  if (digestStatus == DIGEST_CALCULATE_ERROR) {
    otaPrintln("Digest calculate error.");
    return EMBER_AF_IMAGE_VERIFY_ERROR;
  } else if (digestStatus == DIGEST_CALCULATE_IN_PROGRESS) {
    return EMBER_AF_IMAGE_VERIFY_IN_PROGRESS;
  } // Else, DIGEST_CALCULATE_COMPLETE
    //   Fall through

  if (is163k1) {
    if (EMBER_OPERATION_IN_PROGRESS
        == emberDsaVerify(
          &digest,
          (EmberCertificateData*)certificate,
          (EmberSignatureData*)(signature + EUI64_SIZE))) {
      // This indicates that the stack is going to do crypto operations so the
      // application should hold off on normal message sending until that is
      // complete.
      emAfSetCryptoOperationInProgress();
      return EMBER_AF_IMAGE_VERIFY_WAIT;
    }
  } else {
    if (EMBER_OPERATION_IN_PROGRESS
        == emberDsaVerify283k1(
          &digest,
          (EmberCertificate283k1Data*)certificate,
          (EmberSignature283k1Data*)(signature + EUI64_SIZE))) {
      // This indicates that the stack is going to do crypto operations so the
      // application should hold off on normal message sending until that is
      // complete.
      emAfSetCryptoOperationInProgress();
      return EMBER_AF_IMAGE_VERIFY_WAIT;
    }
  }

  return EMBER_AF_IMAGE_VERIFY_ERROR;
}

static DigestCalculateStatus calculateDigest(uint16_t maxHashCalculations,
                                             const EmberAfOtaImageId *id,
                                             EmberMessageDigest* digest,
                                             bool is163k1)
{
  EmberAfOtaStorageStatus status;
  uint32_t imageSize;
  uint8_t block[MAX_BLOCK_SIZE_FOR_HASH];
  uint32_t readSize;
  uint32_t dataLeftToRead;
  uint32_t remainder = 0;
  uint16_t iterations = 0;

  status
    = emAfOtaStorageGetHeaderLengthAndImageSize(id,
                                                NULL, // header length (don't care)
                                                &imageSize);
  if (status) {
    return DIGEST_CALCULATE_ERROR;
  }

  dataLeftToRead = imageSize - currentOffset;
  dataLeftToRead -= is163k1 ? EMBER_SIGNATURE_SIZE : EMBER_SIGNATURE_283K1_SIZE;
  if (currentOffset == 0) {
    otaPrintln("Starting new digest calculation");
    emberAesMmoHashInit(&context);
    emAfPrintPercentageSetStartAndEnd(0, dataLeftToRead);
  }

  readSize = (dataLeftToRead < MAX_BLOCK_SIZE_FOR_HASH
              ? dataLeftToRead
              : MAX_BLOCK_SIZE_FOR_HASH);

  while (dataLeftToRead) {
    uint32_t returnedLength;
    if ((EMBER_AF_OTA_STORAGE_SUCCESS
         != emberAfOtaStorageReadImageDataCallback(id,
                                                   currentOffset,
                                                   readSize,
                                                   block,
                                                   &returnedLength))
        || (readSize != returnedLength)) {
      return DIGEST_CALCULATE_ERROR;
    }
    if (readSize == MAX_BLOCK_SIZE_FOR_HASH) {
      if (EMBER_SUCCESS != emberAesMmoHashUpdate(&context,
                                                 MAX_BLOCK_SIZE_FOR_HASH,
                                                 block)) {
        return DIGEST_CALCULATE_ERROR;
      }

      debugDigestPrint(&context);
    } else {
      remainder = readSize;
    }

    currentOffset += readSize;
    dataLeftToRead -= readSize;
    readSize = (dataLeftToRead < MAX_BLOCK_SIZE_FOR_HASH
                ? dataLeftToRead
                : MAX_BLOCK_SIZE_FOR_HASH);
    emAfPrintPercentageUpdate("Digest Calculate",
                              DIGEST_CALCULATE_PRINT_UPDATE_RATE,
                              currentOffset);
    iterations++;
    if (dataLeftToRead
        && maxHashCalculations != 0
        && iterations >= maxHashCalculations) {
      // Bugzid: 12566.
      // We don't return if there is no dataLeftToRead,
      // since that means we are on the last calculation
      // for the remainder.  The remainder is only stored
      // on the stack so if we return, it will be wiped out
      // for the next execution.
      return DIGEST_CALCULATE_IN_PROGRESS;
    }
  }

  if (EMBER_SUCCESS != emberAesMmoHashFinal(&context,
                                            remainder,
                                            block)) {
    return DIGEST_CALCULATE_ERROR;
  }
  currentOffset += remainder;
  debugDigestPrint(&context);

  emAfPrintPercentageUpdate("Digest Calculate",
                            DIGEST_CALCULATE_PRINT_UPDATE_RATE,
                            currentOffset);

  MEMMOVE(digest->contents,
          context.result,
          EMBER_AES_HASH_BLOCK_SIZE);

  emberAfOtaBootloadClusterFlush();
  otaPrintln("Calculated Digest: ");
  emberAfPrintZigbeeKey(digest->contents);
  otaPrintln("");

  return DIGEST_CALCULATE_COMPLETE;
}

static bool checkSigner(const uint8_t* bigEndianSignerEui64)
{
  uint8_t i;
  for (i = 0; i < MAX_SIGNERS; i++) {
    uint8_t j;
    bool nullEui64 = true;
//    otaPrint("Considering Signer EUI: ");
//    emberAfPrintBigEndianEui64(allowedSignerEuiBigEndian[i]);
    for (j = 0; nullEui64 && j < EUI64_SIZE; j++) {
      if (allowedSignerEuiBigEndian[i][j] != 0) {
        nullEui64 = false;
      }
    }
    if (nullEui64) {
      continue;
    }

    if (0 == MEMCOMPARE(bigEndianSignerEui64,
                        allowedSignerEuiBigEndian[i],
                        EUI64_SIZE)) {
      return true;
    }
  }
  return false;
}

void emAfOtaClientSignatureVerifyPrintSigners(void)
{
  uint8_t i;
  emberAfCoreFlush();
  otaPrintln("Allowed Signers of images, big endian (NULL EUI64 is invalid)");
  emberAfCoreFlush();
  for (i = 0; i < MAX_SIGNERS; i++) {
    emberAfCoreFlush();
    otaPrint("%d: ", i);
    emberAfPrintBigEndianEui64(allowedSignerEuiBigEndian[i]);
  }
}

#if defined DEBUG_DIGEST_PRINT
static void debugDigestPrint(const EmberAesMmoHashContext* context)
{
  emberAfOtaBootloadClusterPrint("Current Digest for length 0x%4X: ",
                                 context->length);
  emberAfPrintZigbeeKey(context->result);
}
#endif

static void dsaVerifyHandler(EmberStatus status)
{
  otaPrintln("DSA Verify returned: 0x%x (%p)",
             status,
             (status == EMBER_SUCCESS
              ? "Success"
              : (status == EMBER_SIGNATURE_VERIFY_FAILURE
                 ? "Invalid Signature"
                 : "ERROR")));

  // This notes that the stack is done doing crypto and has
  // resumed normal operations.  The application's normal
  // behavior will no longer be held off.
  emAfCryptoOperationComplete();

  // Any error status is treated as an image verification failure.
  // One could argue that we could retry an operation after a transient
  // failure (out of buffers) but for now we don't.
  emAfOtaVerifyStoredDataFinish(status == EMBER_SUCCESS
                                ? EMBER_AF_IMAGE_GOOD
                                : EMBER_AF_IMAGE_BAD);
}

#if defined (EZSP_HOST)

void ezspDsaVerifyHandler(EmberStatus status)
{
  dsaVerifyHandler(status);
}

#else

void emberDsaVerifyHandler(EmberStatus status)
{
  dsaVerifyHandler(status);
}

#endif

//------------------------------------------------------------------------------
#else //  !defined(EMBER_AF_PLUGIN_OTA_CLIENT_SIGNATURE_VERIFICATION_SUPPORT)

EmberAfImageVerifyStatus emAfOtaImageSignatureVerify(uint16_t maxHashCalculations,
                                                     const EmberAfOtaImageId* id,
                                                     bool newVerification)
{
  return EMBER_AF_NO_IMAGE_VERIFY_SUPPORT;
}

#endif

//------------------------------------------------------------------------------
