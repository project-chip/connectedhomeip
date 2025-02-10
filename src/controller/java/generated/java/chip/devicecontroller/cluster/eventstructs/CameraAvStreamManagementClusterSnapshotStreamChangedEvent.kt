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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class CameraAvStreamManagementClusterSnapshotStreamChangedEvent (
    val snapshotStreamID: UInt,
    val imageCodec: Optional<UInt>,
    val frameRate: Optional<UInt>,
    val bitRate: Optional<ULong>,
    val minResolution: Optional<chip.devicecontroller.cluster.structs.CameraAvStreamManagementClusterVideoResolutionStruct>,
    val maxResolution: Optional<chip.devicecontroller.cluster.structs.CameraAvStreamManagementClusterVideoResolutionStruct>,
    val quality: Optional<UInt>) {
  override fun toString(): String  = buildString {
    append("CameraAvStreamManagementClusterSnapshotStreamChangedEvent {\n")
    append("\tsnapshotStreamID : $snapshotStreamID\n")
    append("\timageCodec : $imageCodec\n")
    append("\tframeRate : $frameRate\n")
    append("\tbitRate : $bitRate\n")
    append("\tminResolution : $minResolution\n")
    append("\tmaxResolution : $maxResolution\n")
    append("\tquality : $quality\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SNAPSHOT_STREAM_ID), snapshotStreamID)
      if (imageCodec.isPresent) {
      val optimageCodec = imageCodec.get()
      put(ContextSpecificTag(TAG_IMAGE_CODEC), optimageCodec)
    }
      if (frameRate.isPresent) {
      val optframeRate = frameRate.get()
      put(ContextSpecificTag(TAG_FRAME_RATE), optframeRate)
    }
      if (bitRate.isPresent) {
      val optbitRate = bitRate.get()
      put(ContextSpecificTag(TAG_BIT_RATE), optbitRate)
    }
      if (minResolution.isPresent) {
      val optminResolution = minResolution.get()
      optminResolution.toTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), this)
    }
      if (maxResolution.isPresent) {
      val optmaxResolution = maxResolution.get()
      optmaxResolution.toTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), this)
    }
      if (quality.isPresent) {
      val optquality = quality.get()
      put(ContextSpecificTag(TAG_QUALITY), optquality)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SNAPSHOT_STREAM_ID = 0
    private const val TAG_IMAGE_CODEC = 1
    private const val TAG_FRAME_RATE = 2
    private const val TAG_BIT_RATE = 3
    private const val TAG_MIN_RESOLUTION = 4
    private const val TAG_MAX_RESOLUTION = 5
    private const val TAG_QUALITY = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CameraAvStreamManagementClusterSnapshotStreamChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val snapshotStreamID = tlvReader.getUInt(ContextSpecificTag(TAG_SNAPSHOT_STREAM_ID))
      val imageCodec = if (tlvReader.isNextTag(ContextSpecificTag(TAG_IMAGE_CODEC))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_IMAGE_CODEC)))
    } else {
      Optional.empty()
    }
      val frameRate = if (tlvReader.isNextTag(ContextSpecificTag(TAG_FRAME_RATE))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_FRAME_RATE)))
    } else {
      Optional.empty()
    }
      val bitRate = if (tlvReader.isNextTag(ContextSpecificTag(TAG_BIT_RATE))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_BIT_RATE)))
    } else {
      Optional.empty()
    }
      val minResolution = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_RESOLUTION))) {
      Optional.of(chip.devicecontroller.cluster.structs.CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), tlvReader))
    } else {
      Optional.empty()
    }
      val maxResolution = if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_RESOLUTION))) {
      Optional.of(chip.devicecontroller.cluster.structs.CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), tlvReader))
    } else {
      Optional.empty()
    }
      val quality = if (tlvReader.isNextTag(ContextSpecificTag(TAG_QUALITY))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_QUALITY)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterSnapshotStreamChangedEvent(snapshotStreamID, imageCodec, frameRate, bitRate, minResolution, maxResolution, quality)
    }
  }
}
