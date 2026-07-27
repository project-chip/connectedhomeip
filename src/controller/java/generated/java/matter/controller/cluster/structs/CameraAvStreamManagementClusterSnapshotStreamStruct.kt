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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterSnapshotStreamStruct(
  val snapshotStreamID: UShort,
  val imageCodec: UByte,
  val frameRate: UShort,
  val minResolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val maxResolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val quality: UByte,
  val referenceCount: UByte,
  val encodedPixels: Boolean,
  val hardwareEncoder: Boolean,
  val watermarkEnabled: Optional<Boolean>,
  val OSDEnabled: Optional<Boolean>,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterSnapshotStreamStruct {\n")
    append("\tsnapshotStreamID : $snapshotStreamID\n")
    append("\timageCodec : $imageCodec\n")
    append("\tframeRate : $frameRate\n")
    append("\tminResolution : $minResolution\n")
    append("\tmaxResolution : $maxResolution\n")
    append("\tquality : $quality\n")
    append("\treferenceCount : $referenceCount\n")
    append("\tencodedPixels : $encodedPixels\n")
    append("\thardwareEncoder : $hardwareEncoder\n")
    append("\twatermarkEnabled : $watermarkEnabled\n")
    append("\tOSDEnabled : $OSDEnabled\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SNAPSHOT_STREAM_ID), snapshotStreamID)
      put(ContextSpecificTag(TAG_IMAGE_CODEC), imageCodec)
      put(ContextSpecificTag(TAG_FRAME_RATE), frameRate)
      minResolution.toTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), this)
      maxResolution.toTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), this)
      put(ContextSpecificTag(TAG_QUALITY), quality)
      put(ContextSpecificTag(TAG_REFERENCE_COUNT), referenceCount)
      put(ContextSpecificTag(TAG_ENCODED_PIXELS), encodedPixels)
      put(ContextSpecificTag(TAG_HARDWARE_ENCODER), hardwareEncoder)
      if (watermarkEnabled.isPresent) {
        val optwatermarkEnabled = watermarkEnabled.get()
        put(ContextSpecificTag(TAG_WATERMARK_ENABLED), optwatermarkEnabled)
      }
      if (OSDEnabled.isPresent) {
        val optOSDEnabled = OSDEnabled.get()
        put(ContextSpecificTag(TAG_OSD_ENABLED), optOSDEnabled)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_SNAPSHOT_STREAM_ID = 0
    private const val TAG_IMAGE_CODEC = 1
    private const val TAG_FRAME_RATE = 2
    private const val TAG_MIN_RESOLUTION = 3
    private const val TAG_MAX_RESOLUTION = 4
    private const val TAG_QUALITY = 5
    private const val TAG_REFERENCE_COUNT = 6
    private const val TAG_ENCODED_PIXELS = 7
    private const val TAG_HARDWARE_ENCODER = 8
    private const val TAG_WATERMARK_ENABLED = 9
    private const val TAG_OSD_ENABLED = 10

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterSnapshotStreamStruct {
      tlvReader.enterStructure(tlvTag)
      val snapshotStreamID = tlvReader.getUShort(ContextSpecificTag(TAG_SNAPSHOT_STREAM_ID))
      val imageCodec = tlvReader.getUByte(ContextSpecificTag(TAG_IMAGE_CODEC))
      val frameRate = tlvReader.getUShort(ContextSpecificTag(TAG_FRAME_RATE))
      val minResolution =
        CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(
          ContextSpecificTag(TAG_MIN_RESOLUTION),
          tlvReader,
        )
      val maxResolution =
        CameraAvStreamManagementClusterVideoResolutionStruct.fromTlv(
          ContextSpecificTag(TAG_MAX_RESOLUTION),
          tlvReader,
        )
      val quality = tlvReader.getUByte(ContextSpecificTag(TAG_QUALITY))
      val referenceCount = tlvReader.getUByte(ContextSpecificTag(TAG_REFERENCE_COUNT))
      val encodedPixels = tlvReader.getBoolean(ContextSpecificTag(TAG_ENCODED_PIXELS))
      val hardwareEncoder = tlvReader.getBoolean(ContextSpecificTag(TAG_HARDWARE_ENCODER))
      val watermarkEnabled =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_WATERMARK_ENABLED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_WATERMARK_ENABLED)))
        } else {
          Optional.empty()
        }
      val OSDEnabled =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OSD_ENABLED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_OSD_ENABLED)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterSnapshotStreamStruct(
        snapshotStreamID,
        imageCodec,
        frameRate,
        minResolution,
        maxResolution,
        quality,
        referenceCount,
        encodedPixels,
        hardwareEncoder,
        watermarkEnabled,
        OSDEnabled,
      )
    }
  }
}
