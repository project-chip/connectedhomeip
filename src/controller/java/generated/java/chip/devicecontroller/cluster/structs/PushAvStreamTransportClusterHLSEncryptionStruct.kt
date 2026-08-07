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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class PushAvStreamTransportClusterHLSEncryptionStruct(
  val kid: ByteArray,
  val baseKey: ByteArray,
  val schemeURI: ByteArray,
  val ratchetBits: UInt,
  val ratchetTime: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterHLSEncryptionStruct {\n")
    append("\tkid : $kid\n")
    append("\tbaseKey : $baseKey\n")
    append("\tschemeURI : $schemeURI\n")
    append("\tratchetBits : $ratchetBits\n")
    append("\tratchetTime : $ratchetTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_KID), kid)
      put(ContextSpecificTag(TAG_BASE_KEY), baseKey)
      put(ContextSpecificTag(TAG_SCHEME_URI), schemeURI)
      put(ContextSpecificTag(TAG_RATCHET_BITS), ratchetBits)
      if (ratchetTime.isPresent) {
        val optratchetTime = ratchetTime.get()
        put(ContextSpecificTag(TAG_RATCHET_TIME), optratchetTime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_KID = 0
    private const val TAG_BASE_KEY = 1
    private const val TAG_SCHEME_URI = 2
    private const val TAG_RATCHET_BITS = 3
    private const val TAG_RATCHET_TIME = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterHLSEncryptionStruct {
      tlvReader.enterStructure(tlvTag)
      val kid = tlvReader.getByteArray(ContextSpecificTag(TAG_KID))
      val baseKey = tlvReader.getByteArray(ContextSpecificTag(TAG_BASE_KEY))
      val schemeURI = tlvReader.getByteArray(ContextSpecificTag(TAG_SCHEME_URI))
      val ratchetBits = tlvReader.getUInt(ContextSpecificTag(TAG_RATCHET_BITS))
      val ratchetTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RATCHET_TIME))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_RATCHET_TIME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterHLSEncryptionStruct(
        kid,
        baseKey,
        schemeURI,
        ratchetBits,
        ratchetTime,
      )
    }
  }
}
