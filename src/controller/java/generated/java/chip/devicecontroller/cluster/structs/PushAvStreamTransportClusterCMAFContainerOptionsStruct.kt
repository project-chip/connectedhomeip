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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class PushAvStreamTransportClusterCMAFContainerOptionsStruct (
    val CMAFInterface: UInt,
    val segmentDuration: UInt,
    val chunkDuration: UInt,
    val sessionGroup: UInt,
    val trackName: String,
    val CENCKey: Optional<ByteArray>,
    val CENCKeyID: Optional<ByteArray>,
    val metadataEnabled: Optional<Boolean>) {
  override fun toString(): String  = buildString {
    append("PushAvStreamTransportClusterCMAFContainerOptionsStruct {\n")
    append("\tCMAFInterface : $CMAFInterface\n")
    append("\tsegmentDuration : $segmentDuration\n")
    append("\tchunkDuration : $chunkDuration\n")
    append("\tsessionGroup : $sessionGroup\n")
    append("\ttrackName : $trackName\n")
    append("\tCENCKey : $CENCKey\n")
    append("\tCENCKeyID : $CENCKeyID\n")
    append("\tmetadataEnabled : $metadataEnabled\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CMAF_INTERFACE), CMAFInterface)
      put(ContextSpecificTag(TAG_SEGMENT_DURATION), segmentDuration)
      put(ContextSpecificTag(TAG_CHUNK_DURATION), chunkDuration)
      put(ContextSpecificTag(TAG_SESSION_GROUP), sessionGroup)
      put(ContextSpecificTag(TAG_TRACK_NAME), trackName)
      if (CENCKey.isPresent) {
      val optCENCKey = CENCKey.get()
      put(ContextSpecificTag(TAG_CENC_KEY), optCENCKey)
    }
      if (CENCKeyID.isPresent) {
      val optCENCKeyID = CENCKeyID.get()
      put(ContextSpecificTag(TAG_CENC_KEY_ID), optCENCKeyID)
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
    private const val TAG_CENC_KEY = 5
    private const val TAG_CENC_KEY_ID = 6
    private const val TAG_METADATA_ENABLED = 7

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : PushAvStreamTransportClusterCMAFContainerOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val CMAFInterface = tlvReader.getUInt(ContextSpecificTag(TAG_CMAF_INTERFACE))
      val segmentDuration = tlvReader.getUInt(ContextSpecificTag(TAG_SEGMENT_DURATION))
      val chunkDuration = tlvReader.getUInt(ContextSpecificTag(TAG_CHUNK_DURATION))
      val sessionGroup = tlvReader.getUInt(ContextSpecificTag(TAG_SESSION_GROUP))
      val trackName = tlvReader.getString(ContextSpecificTag(TAG_TRACK_NAME))
      val CENCKey = if (tlvReader.isNextTag(ContextSpecificTag(TAG_CENC_KEY))) {
      Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_CENC_KEY)))
    } else {
      Optional.empty()
    }
      val CENCKeyID = if (tlvReader.isNextTag(ContextSpecificTag(TAG_CENC_KEY_ID))) {
      Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_CENC_KEY_ID)))
    } else {
      Optional.empty()
    }
      val metadataEnabled = if (tlvReader.isNextTag(ContextSpecificTag(TAG_METADATA_ENABLED))) {
      Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_METADATA_ENABLED)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return PushAvStreamTransportClusterCMAFContainerOptionsStruct(CMAFInterface, segmentDuration, chunkDuration, sessionGroup, trackName, CENCKey, CENCKeyID, metadataEnabled)
    }
  }
}
