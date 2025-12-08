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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class WebRTCTransportProviderClusterSFrameStruct(
  val cipherSuite: UShort,
  val baseKey: ByteArray,
  val kid: ByteArray,
) {
  override fun toString(): String = buildString {
    append("WebRTCTransportProviderClusterSFrameStruct {\n")
    append("\tcipherSuite : $cipherSuite\n")
    append("\tbaseKey : $baseKey\n")
    append("\tkid : $kid\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CIPHER_SUITE), cipherSuite)
      put(ContextSpecificTag(TAG_BASE_KEY), baseKey)
      put(ContextSpecificTag(TAG_KID), kid)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CIPHER_SUITE = 0
    private const val TAG_BASE_KEY = 1
    private const val TAG_KID = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): WebRTCTransportProviderClusterSFrameStruct {
      tlvReader.enterStructure(tlvTag)
      val cipherSuite = tlvReader.getUShort(ContextSpecificTag(TAG_CIPHER_SUITE))
      val baseKey = tlvReader.getByteArray(ContextSpecificTag(TAG_BASE_KEY))
      val kid = tlvReader.getByteArray(ContextSpecificTag(TAG_KID))

      tlvReader.exitContainer()

      return WebRTCTransportProviderClusterSFrameStruct(cipherSuite, baseKey, kid)
    }
  }
}
