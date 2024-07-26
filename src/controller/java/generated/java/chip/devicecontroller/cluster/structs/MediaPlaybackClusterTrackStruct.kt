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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class MediaPlaybackClusterTrackStruct(
  val id: String,
  val trackAttributes: MediaPlaybackClusterTrackAttributesStruct?,
) {
  override fun toString(): String = buildString {
    append("MediaPlaybackClusterTrackStruct {\n")
    append("\tid : $id\n")
    append("\ttrackAttributes : $trackAttributes\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ID), id)
      if (trackAttributes != null) {
        trackAttributes.toTlv(ContextSpecificTag(TAG_TRACK_ATTRIBUTES), this)
      } else {
        putNull(ContextSpecificTag(TAG_TRACK_ATTRIBUTES))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ID = 0
    private const val TAG_TRACK_ATTRIBUTES = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MediaPlaybackClusterTrackStruct {
      tlvReader.enterStructure(tlvTag)
      val id = tlvReader.getString(ContextSpecificTag(TAG_ID))
      val trackAttributes =
        if (!tlvReader.isNull()) {
          MediaPlaybackClusterTrackAttributesStruct.fromTlv(
            ContextSpecificTag(TAG_TRACK_ATTRIBUTES),
            tlvReader,
          )
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TRACK_ATTRIBUTES))
          null
        }

      tlvReader.exitContainer()

      return MediaPlaybackClusterTrackStruct(id, trackAttributes)
    }
  }
}
