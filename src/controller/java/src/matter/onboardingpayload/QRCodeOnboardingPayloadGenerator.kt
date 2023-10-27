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

import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvWriter

class QRCodeOnboardingPayloadGenerator(private val onboardingPayload: OnboardingPayload) {
  private var allowInvalidPayload = false

  /**
   * This function is called to encode the binary data of a payload to a base38 string.
   *
   * If the payload has any optional data that needs to be TLV encoded, this function will fail.
   *
   * @retval The base38 representation string.
   */
  fun payloadBase38Representation(): String {
    return payloadBase38Representation(null, 0)
  }

  /**
   * This function is called to encode the binary data of a payload to a base38 null-terminated
   * string.
   *
   * If the payload has any optional data that needs to be TLV encoded, this function will allocate
   * a scratch heap buffer to hold the TLV data while encoding.
   *
   * @retval The base38 representation string.
   */
  fun payloadBase38RepresentationWithAutoTLVBuffer(): String {
    // Estimate the size of the needed buffer.
    var estimate = 0

    val dataItemSizeEstimate: (OptionalQRCodeInfo) -> Int = { item ->
      // Each data item needs a control byte and a context tag.
      var size = 2

      if (item.type == OptionalQRCodeInfoType.TYPE_STRING) {
        // We'll need to encode the string length and then the string data.
        // Length is at most 8 bytes.
        size += 8
        size += item.data?.length ?: 0
      } else {
        // Integer.  Assume it might need up to 8 bytes, for simplicity.
        size += 8
      }
      size
    }

    val vendorData = onboardingPayload.getAllOptionalVendorData()
    for (data in vendorData) {
      estimate += dataItemSizeEstimate(data)
    }

    val extensionData = onboardingPayload.getAllOptionalExtensionData()
    for (data in extensionData) {
      estimate += dataItemSizeEstimate(data)
    }

    estimate = estimateStructOverhead(estimate)

    val tlvDataStart: ByteArray? = ByteArray(estimate)

    return payloadBase38Representation(tlvDataStart, estimate)
  }

  fun setAllowInvalidPayload(allow: Boolean) {
    allowInvalidPayload = allow
  }

  /**
   * This function is called to encode the binary data of a payload to a base38 string, using the
   * caller-provided buffer as temporary scratch space for optional data that needs to be
   * TLV-encoded. If that buffer is not big enough to hold the TLV-encoded part of the payload, the
   * function will fail.
   *
   * @param[in] tlvDataStart The start of the buffer to use as temporary scratch space for optional
   *   data that needs to be TLV-encoded.
   * @param[in] tlvDataStartSize The size of the buffer.
   * @retval The base38 representation string.
   */
  private fun payloadBase38Representation(tlvDataStart: ByteArray?, tlvDataStartSize: Int): String {
    if (!allowInvalidPayload && !onboardingPayload.isValidQRCodePayload()) {
      throw OnboardingPayloadException("Invalid argument")
    }

    var tlvDataLengthInBytes =
      generateTLVFromOptionalData(onboardingPayload, tlvDataStart, tlvDataStartSize)
    val bits = ByteArray(kTotalPayloadDataSizeInBytes + tlvDataLengthInBytes)
    val buffer = CharArray(base38EncodedLength(bits.size) + kQRCodePrefix.length)
    payloadBase38RepresentationWithTLV(
      onboardingPayload,
      buffer,
      bits,
      tlvDataStart,
      tlvDataLengthInBytes
    )

    return String(buffer)
  }

  private fun generateTLVFromOptionalData(
    outPayload: OnboardingPayload,
    tlvDataStart: ByteArray?,
    maxLen: Int
  ): Int {
    val optionalData = outPayload.getAllOptionalVendorData()
    val optionalExtensionData = outPayload.getAllOptionalExtensionData()
    if (optionalData.isEmpty() && optionalExtensionData.isEmpty()) {
      return 0
    }

    val rootWriter = TlvWriter(maxLen)
    val innerStructureWriter = rootWriter.startStructure(AnonymousTag)

    for (info in optionalData) {
      writeTag(innerStructureWriter, ContextSpecificTag(info.tag), info)
    }

    for (info in optionalExtensionData) {
      writeTag(innerStructureWriter, ContextSpecificTag(info.tag), info)
    }

    rootWriter.endStructure()
    rootWriter.validateTlv()

    val tlvDataLengthInBytes = rootWriter.getLengthWritten()
    val encodedTlvData = rootWriter.getEncoded()

    if (tlvDataStart != null) {
      System.arraycopy(encodedTlvData, 0, tlvDataStart, 0, tlvDataLengthInBytes)
    }

    return tlvDataLengthInBytes
  }

  private fun writeTag(writer: TlvWriter, tag: Tag, info: OptionalQRCodeInfo) {
    when (info.type) {
      OptionalQRCodeInfoType.TYPE_STRING -> writer.put(tag, info.data!!)
      OptionalQRCodeInfoType.TYPE_INT32 -> writer.put(tag, info.int32)
      else -> throw OnboardingPayloadException("Invalid argument")
    }
  }

  private fun writeTag(writer: TlvWriter, tag: Tag, info: OptionalQRCodeInfoExtension) {
    when (info.type) {
      OptionalQRCodeInfoType.TYPE_STRING -> writeTag(writer, tag, info as OptionalQRCodeInfo)
      OptionalQRCodeInfoType.TYPE_INT64 -> writer.put(tag, info.int64)
      OptionalQRCodeInfoType.TYPE_UINT32 -> writer.put(tag, info.uint32)
      OptionalQRCodeInfoType.TYPE_UINT64 -> writer.put(tag, info.uint64)
      else -> throw OnboardingPayloadException("Invalid argument")
    }
  }

  private fun estimateStructOverhead(): Int {
    // The struct itself has a control byte and an end-of-struct marker.
    return 2
  }

  private fun estimateStructOverhead(firstFieldSize: Int): Int {
    // Estimate 4 bytes of overhead per field. This can happen for a large
    // octet string field: 1 byte control, 1 byte context tag, 2 bytes
    // length.
    return firstFieldSize + 4 + estimateStructOverhead()
  }
}
