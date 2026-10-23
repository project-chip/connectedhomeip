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
  val CMAFInterface: UInt,
  val segmentDuration: UInt,
  val chunkDuration: UInt,
  val sessionGroup: Optional<UInt>,
  val trackName: Optional<String>,
  val metadataEnabled: Optional<Boolean>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterCMAFContainerOptionsStruct {\n")
    append("\tCMAFInterface : $CMAFInterface\n")
    append("\tsegmentDuration : $segmentDuration\n")
    append("\tchunkDuration : $chunkDuration\n")
    append("\tsessionGroup : $sessionGroup\n")
    append("\ttrackName : $trackName\n")
    append("\tmetadataEnabled : $metadataEnabled\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CMAF_INTERFACE), CMAFInterface)
      put(ContextSpecificTag(TAG_SEGMENT_DURATION), segmentDuration)
      put(ContextSpecificTag(TAG_CHUNK_DURATION), chunkDuration)
      if (sessionGroup.isPresent) {
        val optsessionGroup = sessionGroup.get()
        put(ContextSpecificTag(TAG_SESSION_GROUP), optsessionGroup)
      }
      if (trackName.isPresent) {
        val opttrackName = trackName.get()
        put(ContextSpecificTag(TAG_TRACK_NAME), opttrackName)
      }
      if (metadataEnabled.isPresent) {
        val optmetadataEnabled = metadataEnabled.get()
        put(ContextSpecificTag(TAG_METADATA_ENABLED), optmetadataEnabled)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CMAF_INTERFACE = 0
    private const val TAG_SEGMENT_DURATION = 1
    private const val TAG_CHUNK_DURATION = 2
    private const val TAG_SESSION_GROUP = 3
    private const val TAG_TRACK_NAME = 4
    private const val TAG_METADATA_ENABLED = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterCMAFContainerOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val CMAFInterface = tlvReader.getUInt(ContextSpecificTag(TAG_CMAF_INTERFACE))
      val segmentDuration = tlvReader.getUInt(ContextSpecificTag(TAG_SEGMENT_DURATION))
      val chunkDuration = tlvReader.getUInt(ContextSpecificTag(TAG_CHUNK_DURATION))
      val sessionGroup =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SESSION_GROUP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_GROUP)))
        } else {
          Optional.empty()
        }
      val trackName =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRACK_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_TRACK_NAME)))
        } else {
          Optional.empty()
        }
      val metadataEnabled =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_METADATA_ENABLED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_METADATA_ENABLED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterCMAFContainerOptionsStruct(
        CMAFInterface,
        segmentDuration,
        chunkDuration,
        sessionGroup,
        trackName,
        metadataEnabled,
      )
    }
  }
}
