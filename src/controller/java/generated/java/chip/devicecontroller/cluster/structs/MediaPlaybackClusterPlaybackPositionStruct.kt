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
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class MediaPlaybackClusterPlaybackPositionStruct(val updatedAt: Long, val position: Long?) {
  override fun toString(): String = buildString {
    append("MediaPlaybackClusterPlaybackPositionStruct {\n")
    append("\tupdatedAt : $updatedAt\n")
    append("\tposition : $position\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_UPDATED_AT), updatedAt)
      if (position != null) {
        put(ContextSpecificTag(TAG_POSITION), position)
      } else {
        putNull(ContextSpecificTag(TAG_POSITION))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_UPDATED_AT = 0
    private const val TAG_POSITION = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): MediaPlaybackClusterPlaybackPositionStruct {
      tlvReader.enterStructure(tag)
      val updatedAt = tlvReader.getLong(ContextSpecificTag(TAG_UPDATED_AT))
      val position =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_POSITION))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_POSITION))
          null
        }

      tlvReader.exitContainer()

      return MediaPlaybackClusterPlaybackPositionStruct(updatedAt, position)
    }
  }
}
