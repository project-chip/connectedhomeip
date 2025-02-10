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
package matter.controller.cluster.eventstructs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterAudioStreamChangedEvent(
  val audioStreamID: UShort,
  val streamUsage: Optional<UByte>,
  val audioCodec: Optional<UByte>,
  val channelCount: Optional<UByte>,
  val sampleRate: Optional<UInt>,
  val bitRate: Optional<UInt>,
  val bitDepth: Optional<UByte>,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterAudioStreamChangedEvent {\n")
    append("\taudioStreamID : $audioStreamID\n")
    append("\tstreamUsage : $streamUsage\n")
    append("\taudioCodec : $audioCodec\n")
    append("\tchannelCount : $channelCount\n")
    append("\tsampleRate : $sampleRate\n")
    append("\tbitRate : $bitRate\n")
    append("\tbitDepth : $bitDepth\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_AUDIO_STREAM_ID), audioStreamID)
      if (streamUsage.isPresent) {
        val optstreamUsage = streamUsage.get()
        put(ContextSpecificTag(TAG_STREAM_USAGE), optstreamUsage)
      }
      if (audioCodec.isPresent) {
        val optaudioCodec = audioCodec.get()
        put(ContextSpecificTag(TAG_AUDIO_CODEC), optaudioCodec)
      }
      if (channelCount.isPresent) {
        val optchannelCount = channelCount.get()
        put(ContextSpecificTag(TAG_CHANNEL_COUNT), optchannelCount)
      }
      if (sampleRate.isPresent) {
        val optsampleRate = sampleRate.get()
        put(ContextSpecificTag(TAG_SAMPLE_RATE), optsampleRate)
      }
      if (bitRate.isPresent) {
        val optbitRate = bitRate.get()
        put(ContextSpecificTag(TAG_BIT_RATE), optbitRate)
      }
      if (bitDepth.isPresent) {
        val optbitDepth = bitDepth.get()
        put(ContextSpecificTag(TAG_BIT_DEPTH), optbitDepth)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_AUDIO_STREAM_ID = 0
    private const val TAG_STREAM_USAGE = 1
    private const val TAG_AUDIO_CODEC = 2
    private const val TAG_CHANNEL_COUNT = 3
    private const val TAG_SAMPLE_RATE = 4
    private const val TAG_BIT_RATE = 5
    private const val TAG_BIT_DEPTH = 6

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterAudioStreamChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val audioStreamID = tlvReader.getUShort(ContextSpecificTag(TAG_AUDIO_STREAM_ID))
      val streamUsage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STREAM_USAGE))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_STREAM_USAGE)))
        } else {
          Optional.empty()
        }
      val audioCodec =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUDIO_CODEC))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_AUDIO_CODEC)))
        } else {
          Optional.empty()
        }
      val channelCount =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CHANNEL_COUNT))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_CHANNEL_COUNT)))
        } else {
          Optional.empty()
        }
      val sampleRate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SAMPLE_RATE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_SAMPLE_RATE)))
        } else {
          Optional.empty()
        }
      val bitRate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BIT_RATE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_BIT_RATE)))
        } else {
          Optional.empty()
        }
      val bitDepth =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BIT_DEPTH))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_BIT_DEPTH)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterAudioStreamChangedEvent(
        audioStreamID,
        streamUsage,
        audioCodec,
        channelCount,
        sampleRate,
        bitRate,
        bitDepth,
      )
    }
  }
}
