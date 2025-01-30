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

class CameraAvStreamManagementClusterSnapshotStreamStruct(
  val snapshotStreamID: UInt,
  val imageCodec: UInt,
  val frameRate: UInt,
  val bitRate: ULong,
  val minResolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val maxResolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val quality: UInt,
  val referenceCount: UInt
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterSnapshotStreamStruct {\n")
    append("\tsnapshotStreamID : $snapshotStreamID\n")
    append("\timageCodec : $imageCodec\n")
    append("\tframeRate : $frameRate\n")
    append("\tbitRate : $bitRate\n")
    append("\tminResolution : $minResolution\n")
    append("\tmaxResolution : $maxResolution\n")
    append("\tquality : $quality\n")
    append("\treferenceCount : $referenceCount\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SNAPSHOT_STREAM_ID), snapshotStreamID)
      put(ContextSpecificTag(TAG_IMAGE_CODEC), imageCodec)
      put(ContextSpecificTag(TAG_FRAME_RATE), frameRate)
      put(ContextSpecificTag(TAG_BIT_RATE), bitRate)
      minResolution.toTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), this)
      maxResolution.toTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), this)
      put(ContextSpecificTag(TAG_QUALITY), quality)
      put(ContextSpecificTag(TAG_REFERENCE_COUNT), referenceCount)
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
    private const val TAG_REFERENCE_COUNT = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): CameraAvStreamManagementClusterSnapshotStreamStruct {
      tlvReader.enterStructure(tlvTag)
      val snapshotStreamID = tlvReader.getUInt(ContextSpecificTag(TAG_SNAPSHOT_STREAM_ID))
      val imageCodec = tlvReader.getUInt(ContextSpecificTag(TAG_IMAGE_CODEC))
      val frameRate = tlvReader.getUInt(ContextSpecificTag(TAG_FRAME_RATE))
      val bitRate = tlvReader.getULong(ContextSpecificTag(TAG_BIT_RATE))
      val minResolution =
        CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(
          ContextSpecificTag(TAG_MIN_RESOLUTION),
          tlvReader
        )
      val maxResolution =
        CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(
          ContextSpecificTag(TAG_MAX_RESOLUTION),
          tlvReader
        )
      val quality = tlvReader.getUInt(ContextSpecificTag(TAG_QUALITY))
      val referenceCount = tlvReader.getUInt(ContextSpecificTag(TAG_REFERENCE_COUNT))

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterSnapshotStreamStruct(
        snapshotStreamID,
        imageCodec,
        frameRate,
        bitRate,
        minResolution,
        maxResolution,
        quality,
        referenceCount
      )
    }
  }
}
