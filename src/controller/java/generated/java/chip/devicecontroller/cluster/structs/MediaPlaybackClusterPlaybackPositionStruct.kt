/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
