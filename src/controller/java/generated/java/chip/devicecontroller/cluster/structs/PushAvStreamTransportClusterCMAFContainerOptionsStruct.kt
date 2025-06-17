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

class PushAvStreamTransportClusterCMAFContainerOptionsStruct(
  val chunkDuration: UInt,
  val CENCKey: Optional<ByteArray>,
  val metadataEnabled: Optional<Boolean>,
  val CENCKeyID: Optional<ByteArray>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterCMAFContainerOptionsStruct {\n")
    append("\tchunkDuration : $chunkDuration\n")
    append("\tCENCKey : $CENCKey\n")
    append("\tmetadataEnabled : $metadataEnabled\n")
    append("\tCENCKeyID : $CENCKeyID\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CHUNK_DURATION), chunkDuration)
      if (CENCKey.isPresent) {
        val optCENCKey = CENCKey.get()
        put(ContextSpecificTag(TAG_CENC_KEY), optCENCKey)
      }
      if (metadataEnabled.isPresent) {
        val optmetadataEnabled = metadataEnabled.get()
        put(ContextSpecificTag(TAG_METADATA_ENABLED), optmetadataEnabled)
      }
      if (CENCKeyID.isPresent) {
        val optCENCKeyID = CENCKeyID.get()
        put(ContextSpecificTag(TAG_CENC_KEY_ID), optCENCKeyID)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CHUNK_DURATION = 0
    private const val TAG_CENC_KEY = 1
    private const val TAG_METADATA_ENABLED = 2
    private const val TAG_CENC_KEY_ID = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterCMAFContainerOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val chunkDuration = tlvReader.getUInt(ContextSpecificTag(TAG_CHUNK_DURATION))
      val CENCKey =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CENC_KEY))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_CENC_KEY)))
        } else {
          Optional.empty()
        }
      val metadataEnabled =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_METADATA_ENABLED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_METADATA_ENABLED)))
        } else {
          Optional.empty()
        }
      val CENCKeyID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CENC_KEY_ID))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_CENC_KEY_ID)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterCMAFContainerOptionsStruct(
        chunkDuration,
        CENCKey,
        metadataEnabled,
        CENCKeyID,
      )
    }
  }
}
