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

class PushAvStreamTransportClusterMetadataOptionsStruct(
  val multiplexing: UByte,
  val includeMotionZones: Boolean,
  val enableMetadataPrivacySensitive: Boolean,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterMetadataOptionsStruct {\n")
    append("\tmultiplexing : $multiplexing\n")
    append("\tincludeMotionZones : $includeMotionZones\n")
    append("\tenableMetadataPrivacySensitive : $enableMetadataPrivacySensitive\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MULTIPLEXING), multiplexing)
      put(ContextSpecificTag(TAG_INCLUDE_MOTION_ZONES), includeMotionZones)
      put(ContextSpecificTag(TAG_ENABLE_METADATA_PRIVACY_SENSITIVE), enableMetadataPrivacySensitive)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MULTIPLEXING = 0
    private const val TAG_INCLUDE_MOTION_ZONES = 1
    private const val TAG_ENABLE_METADATA_PRIVACY_SENSITIVE = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterMetadataOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val multiplexing = tlvReader.getUByte(ContextSpecificTag(TAG_MULTIPLEXING))
      val includeMotionZones = tlvReader.getBoolean(ContextSpecificTag(TAG_INCLUDE_MOTION_ZONES))
      val enableMetadataPrivacySensitive =
        tlvReader.getBoolean(ContextSpecificTag(TAG_ENABLE_METADATA_PRIVACY_SENSITIVE))

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterMetadataOptionsStruct(
        multiplexing,
        includeMotionZones,
        enableMetadataPrivacySensitive,
      )
    }
  }
}
