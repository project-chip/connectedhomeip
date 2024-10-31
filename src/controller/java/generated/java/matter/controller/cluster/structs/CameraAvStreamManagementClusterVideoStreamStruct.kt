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

class CameraAvStreamManagementClusterVideoStreamStruct(
  val videoStreamID: UShort,
  val streamType: UByte,
  val videoCodec: UByte,
  val minFrameRate: UShort,
  val maxFrameRate: UShort,
  val minResolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val maxResolution: CameraAvStreamManagementClusterVideoResolutionStruct,
  val minBitRate: UInt,
  val maxBitRate: UInt,
  val minFragmentLen: UShort,
  val maxFragmentLen: UShort,
  val watermarkEnabled: Optional<Boolean>,
  val OSDEnabled: Optional<Boolean>,
  val referenceCount: UByte,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterVideoStreamStruct {\n")
    append("\tvideoStreamID : $videoStreamID\n")
    append("\tstreamType : $streamType\n")
    append("\tvideoCodec : $videoCodec\n")
    append("\tminFrameRate : $minFrameRate\n")
    append("\tmaxFrameRate : $maxFrameRate\n")
    append("\tminResolution : $minResolution\n")
    append("\tmaxResolution : $maxResolution\n")
    append("\tminBitRate : $minBitRate\n")
    append("\tmaxBitRate : $maxBitRate\n")
    append("\tminFragmentLen : $minFragmentLen\n")
    append("\tmaxFragmentLen : $maxFragmentLen\n")
    append("\twatermarkEnabled : $watermarkEnabled\n")
    append("\tOSDEnabled : $OSDEnabled\n")
    append("\treferenceCount : $referenceCount\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_VIDEO_STREAM_ID), videoStreamID)
      put(ContextSpecificTag(TAG_STREAM_TYPE), streamType)
      put(ContextSpecificTag(TAG_VIDEO_CODEC), videoCodec)
      put(ContextSpecificTag(TAG_MIN_FRAME_RATE), minFrameRate)
      put(ContextSpecificTag(TAG_MAX_FRAME_RATE), maxFrameRate)
      minResolution.toTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), this)
      maxResolution.toTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), this)
      put(ContextSpecificTag(TAG_MIN_BIT_RATE), minBitRate)
      put(ContextSpecificTag(TAG_MAX_BIT_RATE), maxBitRate)
      put(ContextSpecificTag(TAG_MIN_FRAGMENT_LEN), minFragmentLen)
      put(ContextSpecificTag(TAG_MAX_FRAGMENT_LEN), maxFragmentLen)
      if (watermarkEnabled.isPresent) {
        val optwatermarkEnabled = watermarkEnabled.get()
        put(ContextSpecificTag(TAG_WATERMARK_ENABLED), optwatermarkEnabled)
      }
      if (OSDEnabled.isPresent) {
        val optOSDEnabled = OSDEnabled.get()
        put(ContextSpecificTag(TAG_OSD_ENABLED), optOSDEnabled)
      }
      put(ContextSpecificTag(TAG_REFERENCE_COUNT), referenceCount)
      endStructure()
    }
  }

  companion object {
    private const val TAG_VIDEO_STREAM_ID = 0
    private const val TAG_STREAM_TYPE = 1
    private const val TAG_VIDEO_CODEC = 2
    private const val TAG_MIN_FRAME_RATE = 3
    private const val TAG_MAX_FRAME_RATE = 4
    private const val TAG_MIN_RESOLUTION = 5
    private const val TAG_MAX_RESOLUTION = 6
    private const val TAG_MIN_BIT_RATE = 7
    private const val TAG_MAX_BIT_RATE = 8
    private const val TAG_MIN_FRAGMENT_LEN = 9
    private const val TAG_MAX_FRAGMENT_LEN = 10
    private const val TAG_WATERMARK_ENABLED = 11
    private const val TAG_OSD_ENABLED = 12
    private const val TAG_REFERENCE_COUNT = 13

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterVideoStreamStruct {
      tlvReader.enterStructure(tlvTag)
      val videoStreamID = tlvReader.getUShort(ContextSpecificTag(TAG_VIDEO_STREAM_ID))
      val streamType = tlvReader.getUByte(ContextSpecificTag(TAG_STREAM_TYPE))
      val videoCodec = tlvReader.getUByte(ContextSpecificTag(TAG_VIDEO_CODEC))
      val minFrameRate = tlvReader.getUShort(ContextSpecificTag(TAG_MIN_FRAME_RATE))
      val maxFrameRate = tlvReader.getUShort(ContextSpecificTag(TAG_MAX_FRAME_RATE))
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
      val minBitRate = tlvReader.getUInt(ContextSpecificTag(TAG_MIN_BIT_RATE))
      val maxBitRate = tlvReader.getUInt(ContextSpecificTag(TAG_MAX_BIT_RATE))
      val minFragmentLen = tlvReader.getUShort(ContextSpecificTag(TAG_MIN_FRAGMENT_LEN))
      val maxFragmentLen = tlvReader.getUShort(ContextSpecificTag(TAG_MAX_FRAGMENT_LEN))
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
      val referenceCount = tlvReader.getUByte(ContextSpecificTag(TAG_REFERENCE_COUNT))

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterVideoStreamStruct(
        videoStreamID,
        streamType,
        videoCodec,
        minFrameRate,
        maxFrameRate,
        minResolution,
        maxResolution,
        minBitRate,
        maxBitRate,
        minFragmentLen,
        maxFragmentLen,
        watermarkEnabled,
        OSDEnabled,
        referenceCount,
      )
    }
  }
}
