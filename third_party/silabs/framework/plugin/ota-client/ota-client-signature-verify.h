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
 * @brief Signature verification APIs for the OTA Client plugin.
 *******************************************************************************
   ******************************************************************************/

// Hashing a file can be expensive for small processors. The maxHashCalculations
// determines how many iterations are made before returning back to the caller.
// A value of 0 indicates to completely calculate the digest before returning.
// A value greater than 0 means that a number of hashes will be performed and
// then the routine will return EMBER_AF_IMAGE_VERIFY_IN_PROGRESS.  In that
// case it is expected that this function must be called repeatedly until it
// returns another status code.
// When EMBER_AF_IMAGE_VERIFY_WAIT is returned then no further calls
// are necessary.  The verify code will fire the callback
// emAfOtaVerifyStoredDataFinish() when it is has a result.
EmberAfImageVerifyStatus emAfOtaImageSignatureVerify(uint16_t maxHashCalculations,
                                                     const EmberAfOtaImageId* id,
                                                     bool newVerification);

// This is the maximum number of digest calculations we perform per call to
// emAfOtaImageSignatureVerify().  Arbitrary chosen value to limit
// time spent in this routine.  A value of 0 means we will NOT return
// until we are completely done with our digest calculations.
// Empirically waiting until digest calculations are complete can
// take quite a while for EZSP hosts (~40 seconds for a UART connected host).
// So we want to make sure that other parts of the framework can run during
// this time.  On SOC systems a similar problem occurs.  If we set this to 0
// then emberTick() will not fire and therefore the watchdog timer will not be
// serviced.
#define MAX_DIGEST_CALCULATIONS_PER_CALL 5

void emAfOtaVerifyStoredDataFinish(EmberAfImageVerifyStatus status);

void emAfOtaClientSignatureVerifyPrintSigners(void);
