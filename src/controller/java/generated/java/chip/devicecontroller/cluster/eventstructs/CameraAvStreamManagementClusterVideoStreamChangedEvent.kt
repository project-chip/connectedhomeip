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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterVideoStreamChangedEvent(
  val videoStreamID: UInt,
  val streamUsage: Optional<UInt>,
  val videoCodec: Optional<UInt>,
  val minFrameRate: Optional<UInt>,
  val maxFrameRate: Optional<UInt>,
  val minResolution:
    Optional<
      chip.devicecontroller.cluster.structs.CameraAvStreamManagementClusterVideoResolutionStruct
    >,
  val maxResolution:
    Optional<
      chip.devicecontroller.cluster.structs.CameraAvStreamManagementClusterVideoResolutionStruct
    >,
  val minBitRate: Optional<ULong>,
  val maxBitRate: Optional<ULong>,
  val minFragmentLen: Optional<UInt>,
  val maxFragmentLen: Optional<UInt>
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterVideoStreamChangedEvent {\n")
    append("\tvideoStreamID : $videoStreamID\n")
    append("\tstreamUsage : $streamUsage\n")
    append("\tvideoCodec : $videoCodec\n")
    append("\tminFrameRate : $minFrameRate\n")
    append("\tmaxFrameRate : $maxFrameRate\n")
    append("\tminResolution : $minResolution\n")
    append("\tmaxResolution : $maxResolution\n")
    append("\tminBitRate : $minBitRate\n")
    append("\tmaxBitRate : $maxBitRate\n")
    append("\tminFragmentLen : $minFragmentLen\n")
    append("\tmaxFragmentLen : $maxFragmentLen\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_VIDEO_STREAM_ID), videoStreamID)
      if (streamUsage.isPresent) {
        val optstreamUsage = streamUsage.get()
        put(ContextSpecificTag(TAG_STREAM_USAGE), optstreamUsage)
      }
      if (videoCodec.isPresent) {
        val optvideoCodec = videoCodec.get()
        put(ContextSpecificTag(TAG_VIDEO_CODEC), optvideoCodec)
      }
      if (minFrameRate.isPresent) {
        val optminFrameRate = minFrameRate.get()
        put(ContextSpecificTag(TAG_MIN_FRAME_RATE), optminFrameRate)
      }
      if (maxFrameRate.isPresent) {
        val optmaxFrameRate = maxFrameRate.get()
        put(ContextSpecificTag(TAG_MAX_FRAME_RATE), optmaxFrameRate)
      }
      if (minResolution.isPresent) {
        val optminResolution = minResolution.get()
        optminResolution.toTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), this)
      }
      if (maxResolution.isPresent) {
        val optmaxResolution = maxResolution.get()
        optmaxResolution.toTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), this)
      }
      if (minBitRate.isPresent) {
        val optminBitRate = minBitRate.get()
        put(ContextSpecificTag(TAG_MIN_BIT_RATE), optminBitRate)
      }
      if (maxBitRate.isPresent) {
        val optmaxBitRate = maxBitRate.get()
        put(ContextSpecificTag(TAG_MAX_BIT_RATE), optmaxBitRate)
      }
      if (minFragmentLen.isPresent) {
        val optminFragmentLen = minFragmentLen.get()
        put(ContextSpecificTag(TAG_MIN_FRAGMENT_LEN), optminFragmentLen)
      }
      if (maxFragmentLen.isPresent) {
        val optmaxFragmentLen = maxFragmentLen.get()
        put(ContextSpecificTag(TAG_MAX_FRAGMENT_LEN), optmaxFragmentLen)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_VIDEO_STREAM_ID = 0
    private const val TAG_STREAM_USAGE = 1
    private const val TAG_VIDEO_CODEC = 2
    private const val TAG_MIN_FRAME_RATE = 3
    private const val TAG_MAX_FRAME_RATE = 4
    private const val TAG_MIN_RESOLUTION = 5
    private const val TAG_MAX_RESOLUTION = 6
    private const val TAG_MIN_BIT_RATE = 7
    private const val TAG_MAX_BIT_RATE = 8
    private const val TAG_MIN_FRAGMENT_LEN = 9
    private const val TAG_MAX_FRAGMENT_LEN = 10

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): CameraAvStreamManagementClusterVideoStreamChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val videoStreamID = tlvReader.getUInt(ContextSpecificTag(TAG_VIDEO_STREAM_ID))
      val streamUsage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STREAM_USAGE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_STREAM_USAGE)))
        } else {
          Optional.empty()
        }
      val videoCodec =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VIDEO_CODEC))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_VIDEO_CODEC)))
        } else {
          Optional.empty()
        }
      val minFrameRate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_FRAME_RATE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MIN_FRAME_RATE)))
        } else {
          Optional.empty()
        }
      val maxFrameRate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_FRAME_RATE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_FRAME_RATE)))
        } else {
          Optional.empty()
        }
      val minResolution =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_RESOLUTION))) {
          Optional.of(
            chip.devicecontroller.cluster.structs
              .CameraAvStreamManagementClusterVideoResolutionStruct
              .fromTlv(ContextSpecificTag(TAG_MIN_RESOLUTION), tlvReader)
          )
        } else {
          Optional.empty()
        }
      val maxResolution =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_RESOLUTION))) {
          Optional.of(
            chip.devicecontroller.cluster.structs
              .CameraAvStreamManagementClusterVideoResolutionStruct
              .fromTlv(ContextSpecificTag(TAG_MAX_RESOLUTION), tlvReader)
          )
        } else {
          Optional.empty()
        }
      val minBitRate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_BIT_RATE))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MIN_BIT_RATE)))
        } else {
          Optional.empty()
        }
      val maxBitRate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_BIT_RATE))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_MAX_BIT_RATE)))
        } else {
          Optional.empty()
        }
      val minFragmentLen =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MIN_FRAGMENT_LEN))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MIN_FRAGMENT_LEN)))
        } else {
          Optional.empty()
        }
      val maxFragmentLen =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_FRAGMENT_LEN))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_FRAGMENT_LEN)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterVideoStreamChangedEvent(
        videoStreamID,
        streamUsage,
        videoCodec,
        minFrameRate,
        maxFrameRate,
        minResolution,
        maxResolution,
        minBitRate,
        maxBitRate,
        minFragmentLen,
        maxFragmentLen
      )
    }
  }
}
