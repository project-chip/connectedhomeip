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

import matter.tlv.ContextSpecificTag
import matter.tlv.IntValue
import matter.tlv.TlvReader
import matter.tlv.UnsignedIntValue
import matter.tlv.Utf8StringValue
import java.nio.ByteBuffer
import java.util.concurrent.atomic.AtomicInteger

/**
 * @class QRCodeOnboardingPayloadParser A class that can be used to convert a base38 encoded payload
 *   to a OnboardingPayload object
 */
class QRCodeOnboardingPayloadParser(private val mBase38Representation: String) {


  fun populatePayload(): OnboardingPayload {
    val indexToReadFrom = AtomicInteger(0)
    val outPayload = OnboardingPayload()

    val payload = extractPayload(mBase38Representation)
    if (payload.isEmpty()) {
      throw UnrecognizedQrCodeException("Invalid argument")
    }

    val buf = base38Decode(payload)
    var dest = readBits(buf, indexToReadFrom, kVersionFieldLengthInBits)
    outPayload.version = dest.toInt()

    dest = readBits(buf, indexToReadFrom, kVendorIDFieldLengthInBits)
    outPayload.vendorId = dest.toInt()

    dest = readBits(buf, indexToReadFrom, kProductIDFieldLengthInBits)
    outPayload.productId = dest.toInt()

    dest = readBits(buf, indexToReadFrom, kCommissioningFlowFieldLengthInBits)
    outPayload.commissioningFlow = dest.toInt()

    dest = readBits(buf, indexToReadFrom, kRendezvousInfoFieldLengthInBits)
    outPayload.setRendezvousInformation(dest)

    dest = readBits(buf, indexToReadFrom, kPayloadDiscriminatorFieldLengthInBits)
    outPayload.discriminator = dest.toInt()

    dest = readBits(buf, indexToReadFrom, kSetupPINCodeFieldLengthInBits)
    outPayload.setupPinCode = dest

    dest = readBits(buf, indexToReadFrom, kPaddingFieldLengthInBits)
    if (dest != 0L) {
      throw UnrecognizedQrCodeException("Invalid argument")
    }

    populateTLV(outPayload, buf, indexToReadFrom)

    return outPayload
  }

  private fun populateTLV(
    payload: OnboardingPayload,
    payloadData: ArrayList<Byte>,
    index: AtomicInteger
  ) {
    val bitsLeftToRead = (payloadData.count() * 8) - index.get()
    val tlvBytesLength = (bitsLeftToRead + 7) / 8

    if (tlvBytesLength == 0) {
      return
    }
    val byteBuffer = ByteBuffer.allocate(tlvBytesLength)
    for (i in 0 until tlvBytesLength) {
      val value = readBits(payloadData, index, 8)
      byteBuffer.put(value.toByte())
    }

    val reader = TlvReader(byteBuffer.array())
    while (true) {
      val element = reader.nextElement()
      if (reader.isEndOfTlv()) {
        break
      }
      val info = OptionalQRCodeInfoExtension()

      //update tag
      if (element.tag is ContextSpecificTag) {
        info.tag = element.tag.tagNumber
      }

      //update values
      if (element.value is IntValue) {
        info.int32 = element.value.value.toInt()
        info.type = OptionalQRCodeInfoType.TYPE_INT32
      }
      if (element.value is UnsignedIntValue) {
        info.uint32 = element.value.value
        info.type = OptionalQRCodeInfoType.TYPE_UINT32
      }
      if (element.value is Utf8StringValue) {
        info.data = element.value.value
        info.type = OptionalQRCodeInfoType.TYPE_STRING
      }

      if (info.tag < 0x80) {
        payload.addOptionalExtensionData(info)
      } else {
        payload.addOptionalVendorData(info)
      }
    }

  }

  companion object {
    // Populate numberOfBits into dest from buf starting at startIndex
    fun readBits(buf: ArrayList<Byte>, index: AtomicInteger, numberOfBitsToRead: Int): Long {
      var dest: Long = 0
      if (index.get() + numberOfBitsToRead > buf.size * 8 || numberOfBitsToRead > Long.SIZE_BITS) {
        throw UnrecognizedQrCodeException("Invalid argument")
      }

      var currentIndex = index.get()
      for (bitsRead in 0 until numberOfBitsToRead) {
        if (buf[currentIndex / 8].toInt() and (1 shl (currentIndex % 8)) != 0) {
          dest = dest or (1L shl bitsRead)
        }
        currentIndex++
      }
      index.addAndGet(numberOfBitsToRead)
      return dest
    }

    fun extractPayload(inString: String?): String {
      return inString
        ?.split('%')
        ?.filter { s -> s.startsWith(kQRCodePrefix) }
        ?.firstOrNull()
        ?.substring(kQRCodePrefix.length)
        ?: ""
    }
  }
}
