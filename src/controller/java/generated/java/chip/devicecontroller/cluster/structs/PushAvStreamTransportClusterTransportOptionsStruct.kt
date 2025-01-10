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

class PushAvStreamTransportClusterTransportOptionsStruct(
  val streamUsage: UInt,
  val videoStreamID: Optional<UInt>,
  val audioStreamID: Optional<UInt>,
  val endpointID: UInt,
  val url: String,
  val triggerOptions: PushAvStreamTransportClusterTransportTriggerOptionsStruct,
  val ingestMethod: UInt,
  val containerFormat: UInt,
  val containerOptions: PushAvStreamTransportClusterContainerOptionsStruct,
  val metadataOptions: Optional<PushAvStreamTransportClusterMetadataOptionsStruct>,
  val expiryTime: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterTransportOptionsStruct {\n")
    append("\tstreamUsage : $streamUsage\n")
    append("\tvideoStreamID : $videoStreamID\n")
    append("\taudioStreamID : $audioStreamID\n")
    append("\tendpointID : $endpointID\n")
    append("\turl : $url\n")
    append("\ttriggerOptions : $triggerOptions\n")
    append("\tingestMethod : $ingestMethod\n")
    append("\tcontainerFormat : $containerFormat\n")
    append("\tcontainerOptions : $containerOptions\n")
    append("\tmetadataOptions : $metadataOptions\n")
    append("\texpiryTime : $expiryTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_STREAM_USAGE), streamUsage)
      if (videoStreamID.isPresent) {
        val optvideoStreamID = videoStreamID.get()
        put(ContextSpecificTag(TAG_VIDEO_STREAM_ID), optvideoStreamID)
      }
      if (audioStreamID.isPresent) {
        val optaudioStreamID = audioStreamID.get()
        put(ContextSpecificTag(TAG_AUDIO_STREAM_ID), optaudioStreamID)
      }
      put(ContextSpecificTag(TAG_ENDPOINT_ID), endpointID)
      put(ContextSpecificTag(TAG_URL), url)
      triggerOptions.toTlv(ContextSpecificTag(TAG_TRIGGER_OPTIONS), this)
      put(ContextSpecificTag(TAG_INGEST_METHOD), ingestMethod)
      put(ContextSpecificTag(TAG_CONTAINER_FORMAT), containerFormat)
      containerOptions.toTlv(ContextSpecificTag(TAG_CONTAINER_OPTIONS), this)
      if (metadataOptions.isPresent) {
        val optmetadataOptions = metadataOptions.get()
        optmetadataOptions.toTlv(ContextSpecificTag(TAG_METADATA_OPTIONS), this)
      }
      if (expiryTime.isPresent) {
        val optexpiryTime = expiryTime.get()
        put(ContextSpecificTag(TAG_EXPIRY_TIME), optexpiryTime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_STREAM_USAGE = 0
    private const val TAG_VIDEO_STREAM_ID = 1
    private const val TAG_AUDIO_STREAM_ID = 2
    private const val TAG_ENDPOINT_ID = 3
    private const val TAG_URL = 4
    private const val TAG_TRIGGER_OPTIONS = 5
    private const val TAG_INGEST_METHOD = 6
    private const val TAG_CONTAINER_FORMAT = 7
    private const val TAG_CONTAINER_OPTIONS = 8
    private const val TAG_METADATA_OPTIONS = 9
    private const val TAG_EXPIRY_TIME = 10

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterTransportOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val streamUsage = tlvReader.getUInt(ContextSpecificTag(TAG_STREAM_USAGE))
      val videoStreamID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VIDEO_STREAM_ID))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_VIDEO_STREAM_ID)))
        } else {
          Optional.empty()
        }
      val audioStreamID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUDIO_STREAM_ID))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_AUDIO_STREAM_ID)))
        } else {
          Optional.empty()
        }
      val endpointID = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT_ID))
      val url = tlvReader.getString(ContextSpecificTag(TAG_URL))
      val triggerOptions =
        PushAvStreamTransportClusterTransportTriggerOptionsStruct.fromTlv(
          ContextSpecificTag(TAG_TRIGGER_OPTIONS),
          tlvReader,
        )
      val ingestMethod = tlvReader.getUInt(ContextSpecificTag(TAG_INGEST_METHOD))
      val containerFormat = tlvReader.getUInt(ContextSpecificTag(TAG_CONTAINER_FORMAT))
      val containerOptions =
        PushAvStreamTransportClusterContainerOptionsStruct.fromTlv(
          ContextSpecificTag(TAG_CONTAINER_OPTIONS),
          tlvReader,
        )
      val metadataOptions =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_METADATA_OPTIONS))) {
          Optional.of(
            PushAvStreamTransportClusterMetadataOptionsStruct.fromTlv(
              ContextSpecificTag(TAG_METADATA_OPTIONS),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val expiryTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXPIRY_TIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_EXPIRY_TIME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterTransportOptionsStruct(
        streamUsage,
        videoStreamID,
        audioStreamID,
        endpointID,
        url,
        triggerOptions,
        ingestMethod,
        containerFormat,
        containerOptions,
        metadataOptions,
        expiryTime,
      )
    }
  }
}
