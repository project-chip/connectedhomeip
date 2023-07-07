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

package chip.onboardingpayload

import java.lang.StringBuilder
import java.util.concurrent.atomic.AtomicInteger
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvReader

/**
 * @class QRCodeOnboardingPayloadParser
 * A class that can be used to convert a base38 encoded payload to a OnboardingPayload object
 */
class QRCodeOnboardingPayloadParser(private val mBase38Representation: String) {
  private fun extractPayload(inString: String): String {
    var chipSegment = ""
    val delimiter = '%'
    val startIndices = mutableListOf<Int>()
    startIndices.add(0)

    for (i in inString.indices) {
        if (inString[i] == delimiter) {
            startIndices.add(i + 1)
        }
    }

    // Find the first string between delimiters that starts with kQRCodePrefix
    for (i in 0 until startIndices.size) {
        val startIndex = startIndices[i]
        val endIndex = if (i == startIndices.size - 1) inString.length else startIndices[i + 1] - 1
        val length = if (endIndex != inString.length) endIndex - startIndex else inString.length
        val segment = inString.substring(startIndex, startIndex + length)

        // Find a segment that starts with kQRCodePrefix
        if (segment.startsWith(kQRCodePrefix) && segment.length > kQRCodePrefix.length) {
            chipSegment = segment
            break
        }
    }

    if (chipSegment.length > 0) {
        return chipSegment.substring(kQRCodePrefix.length) // strip out prefix before returning
    }

    return chipSegment
  }

  fun populatePayload(outPayload: OnboardingPayload) {
    var indexToReadFrom: AtomicInteger = AtomicInteger(0)

    val payload = extractPayload(mBase38Representation)
    if (payload.length == 0) {
      throw UnrecognizedQrCodeException("Invalid argument")
    }

    var buf = base38Decode(payload)
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

    // TODO: populate TLV optional fields
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
  }
}
