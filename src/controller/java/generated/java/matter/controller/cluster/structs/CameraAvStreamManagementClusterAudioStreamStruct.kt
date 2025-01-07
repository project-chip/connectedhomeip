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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CameraAvStreamManagementClusterAudioStreamStruct(
  val audioStreamID: UShort,
  val streamType: UByte,
  val audioCodec: UByte,
  val channelCount: UByte,
  val sampleRate: UInt,
  val bitRate: UInt,
  val bitDepth: UByte,
  val referenceCount: UByte,
) {
  override fun toString(): String = buildString {
    append("CameraAvStreamManagementClusterAudioStreamStruct {\n")
    append("\taudioStreamID : $audioStreamID\n")
    append("\tstreamType : $streamType\n")
    append("\taudioCodec : $audioCodec\n")
    append("\tchannelCount : $channelCount\n")
    append("\tsampleRate : $sampleRate\n")
    append("\tbitRate : $bitRate\n")
    append("\tbitDepth : $bitDepth\n")
    append("\treferenceCount : $referenceCount\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_AUDIO_STREAM_ID), audioStreamID)
      put(ContextSpecificTag(TAG_STREAM_TYPE), streamType)
      put(ContextSpecificTag(TAG_AUDIO_CODEC), audioCodec)
      put(ContextSpecificTag(TAG_CHANNEL_COUNT), channelCount)
      put(ContextSpecificTag(TAG_SAMPLE_RATE), sampleRate)
      put(ContextSpecificTag(TAG_BIT_RATE), bitRate)
      put(ContextSpecificTag(TAG_BIT_DEPTH), bitDepth)
      put(ContextSpecificTag(TAG_REFERENCE_COUNT), referenceCount)
      endStructure()
    }
  }

  companion object {
    private const val TAG_AUDIO_STREAM_ID = 0
    private const val TAG_STREAM_TYPE = 1
    private const val TAG_AUDIO_CODEC = 2
    private const val TAG_CHANNEL_COUNT = 3
    private const val TAG_SAMPLE_RATE = 4
    private const val TAG_BIT_RATE = 5
    private const val TAG_BIT_DEPTH = 6
    private const val TAG_REFERENCE_COUNT = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CameraAvStreamManagementClusterAudioStreamStruct {
      tlvReader.enterStructure(tlvTag)
      val audioStreamID = tlvReader.getUShort(ContextSpecificTag(TAG_AUDIO_STREAM_ID))
      val streamType = tlvReader.getUByte(ContextSpecificTag(TAG_STREAM_TYPE))
      val audioCodec = tlvReader.getUByte(ContextSpecificTag(TAG_AUDIO_CODEC))
      val channelCount = tlvReader.getUByte(ContextSpecificTag(TAG_CHANNEL_COUNT))
      val sampleRate = tlvReader.getUInt(ContextSpecificTag(TAG_SAMPLE_RATE))
      val bitRate = tlvReader.getUInt(ContextSpecificTag(TAG_BIT_RATE))
      val bitDepth = tlvReader.getUByte(ContextSpecificTag(TAG_BIT_DEPTH))
      val referenceCount = tlvReader.getUByte(ContextSpecificTag(TAG_REFERENCE_COUNT))

      tlvReader.exitContainer()

      return CameraAvStreamManagementClusterAudioStreamStruct(
        audioStreamID,
        streamType,
        audioCodec,
        channelCount,
        sampleRate,
        bitRate,
        bitDepth,
        referenceCount,
      )
    }
  }
}
