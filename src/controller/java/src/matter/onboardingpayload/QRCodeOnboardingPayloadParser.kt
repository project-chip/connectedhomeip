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

import java.nio.ByteBuffer
import java.util.concurrent.atomic.AtomicInteger
import matter.tlv.ContextSpecificTag
import matter.tlv.Element
import matter.tlv.IntValue
import matter.tlv.TlvReader
import matter.tlv.Utf8StringValue

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
    repeat(tlvBytesLength) {
      val value = readBits(payloadData, index, 8)
      byteBuffer.put(value.toByte())
    }

    val reader = TlvReader(byteBuffer.array())
    while (true) {
      val element = reader.nextElement()
      if (reader.isEndOfTlv()) {
        break
      }
      parseTLVFields(element, payload)
    }
  }

  private fun parseTLVFields(element: Element, payload: OnboardingPayload) {
    // update tag
    val tag = element.tag
    if (tag !is ContextSpecificTag) {
      return
    }

    if (tag.tagNumber < 0x80) {
      // add serial number
      if (tag.tagNumber == kSerialNumberTag) {
        val value = element.value
        if (value is IntValue) {
          payload.addSerialNumber(value.value.toInt())
        }
        if (value is Utf8StringValue) {
          payload.addSerialNumber(value.value)
        }
      }
    } else {
      // add extension values
      val value = element.value
      if (value is IntValue) {
        payload.addOptionalVendorData(tag.tagNumber, value.value.toInt())
      } else if (value is Utf8StringValue) {
        payload.addOptionalVendorData(tag.tagNumber, value.value)
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
