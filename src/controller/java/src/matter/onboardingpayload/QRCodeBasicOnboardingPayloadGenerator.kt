/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

package matter.onboardingpayload

import java.util.concurrent.atomic.AtomicInteger

/**
 * A minimal QR code setup payload generator that omits any optional data, for compatibility with
 * devices that don't support std::string or STL.
 */
class QRCodeBasicOnboardingPayloadGenerator(private val payload: OnboardingPayload) {

  /**
   * This function is called to encode the binary data of a payload to a base38 string.
   *
   * The resulting size of the outBuffer span will be the size of data written.
   *
   * This function will fail if the payload has any optional data requiring TLV encoding.
   *
   * @param[out] outBuffer The buffer to copy the base38 to.
   */
  fun payloadBase38Representation(outBuffer: CharArray): Unit {
    val bits = ByteArray(kTotalPayloadDataSizeInBytes)
    if (!payload.isValidQRCodePayload()) {
      throw OnboardingPayloadException("Invalid argument")
    }

    payloadBase38RepresentationWithTLV(payload, outBuffer, bits, null, 0)
  }
}

fun payloadBase38RepresentationWithTLV(
  payload: OnboardingPayload,
  outBuffer: CharArray,
  bits: ByteArray,
  tlvDataStart: ByteArray?,
  tlvDataLengthInBytes: Int
) {
  bits.fill(0)
  generateBitSet(payload, bits, tlvDataStart, tlvDataLengthInBytes)

  val prefixLen = kQRCodePrefix.length

  if (outBuffer.size < prefixLen) {
    throw OnboardingPayloadException("Buffer is too small")
  } else {
    val subBuffer = outBuffer.copyOfRange(prefixLen, outBuffer.size)
    for (i in kQRCodePrefix.indices) {
      outBuffer[i] = kQRCodePrefix[i]
    }

    base38Encode(bits, subBuffer)

    // Copy the subBuffer back to the outBuffer
    subBuffer.copyInto(outBuffer, prefixLen)
  }
}

private fun generateBitSet(
  payload: OnboardingPayload,
  bits: ByteArray,
  tlvDataStart: ByteArray?,
  tlvDataLengthInBytes: Int
) {
  var offset: AtomicInteger = AtomicInteger(0)
  val totalPayloadSizeInBits = kTotalPayloadDataSizeInBits + (tlvDataLengthInBytes * 8)
  if (bits.size * 8 < totalPayloadSizeInBits)
    throw OnboardingPayloadException("Buffer is too small")

  populateBits(
    bits,
    offset,
    payload.version.toLong(),
    kVersionFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(
    bits,
    offset,
    payload.vendorId.toLong(),
    kVendorIDFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(
    bits,
    offset,
    payload.productId.toLong(),
    kProductIDFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(
    bits,
    offset,
    payload.commissioningFlow.toLong(),
    kCommissioningFlowFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(
    bits,
    offset,
    payload.getRendezvousInformation(),
    kRendezvousInfoFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(
    bits,
    offset,
    payload.discriminator.toLong(),
    kPayloadDiscriminatorFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(
    bits,
    offset,
    payload.setupPinCode,
    kSetupPINCodeFieldLengthInBits,
    kTotalPayloadDataSizeInBits
  )
  populateBits(bits, offset, 0L, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits)
  populateTLVBits(bits, offset, tlvDataStart, tlvDataLengthInBytes, totalPayloadSizeInBits)
}

// Populates numberOfBits starting from LSB of input into bits, which is assumed to be
// zero-initialized
private fun populateBits(
  bits: ByteArray,
  offset: AtomicInteger,
  input: Long,
  numberOfBits: Int,
  totalPayloadDataSizeInBits: Int
) {
  if (offset.get() + numberOfBits > totalPayloadDataSizeInBits)
    throw OnboardingPayloadException("Invalid argument")

  if (input >= (1L shl numberOfBits)) throw OnboardingPayloadException("Invalid argument")

  var index = offset.get()
  offset.addAndGet(numberOfBits)
  var inputValue = input
  while (inputValue != 0L) {
    if (inputValue and 1L != 0L) {
      val mask = 1 shl (index % 8)
      bits[index / 8] = (bits[index / 8].toInt() or mask).toByte()
    }
    index++
    inputValue = inputValue shr 1
  }
}

private fun populateTLVBits(
  bits: ByteArray,
  offset: AtomicInteger,
  tlvBuf: ByteArray?,
  tlvBufSizeInBytes: Int,
  totalPayloadDataSizeInBits: Int
) {
  if (tlvBuf == null) {
    return
  }

  for (i in 0 until tlvBufSizeInBytes) {
    val value = tlvBuf[i]
    populateBits(bits, offset, value.toUByte().toLong(), 8, totalPayloadDataSizeInBits)
  }
}
