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

class WebRTCTransportProviderClusterWebRTCSessionStruct(
  val id: UShort,
  val peerNodeID: ULong,
  val peerFabricIndex: UByte,
  val streamType: UByte,
  val videoStreamID: UShort?,
  val audioStreamID: UShort?,
  val metadataOptions: UByte,
) {
  override fun toString(): String = buildString {
    append("WebRTCTransportProviderClusterWebRTCSessionStruct {\n")
    append("\tid : $id\n")
    append("\tpeerNodeID : $peerNodeID\n")
    append("\tpeerFabricIndex : $peerFabricIndex\n")
    append("\tstreamType : $streamType\n")
    append("\tvideoStreamID : $videoStreamID\n")
    append("\taudioStreamID : $audioStreamID\n")
    append("\tmetadataOptions : $metadataOptions\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ID), id)
      put(ContextSpecificTag(TAG_PEER_NODE_ID), peerNodeID)
      put(ContextSpecificTag(TAG_PEER_FABRIC_INDEX), peerFabricIndex)
      put(ContextSpecificTag(TAG_STREAM_TYPE), streamType)
      if (videoStreamID != null) {
        put(ContextSpecificTag(TAG_VIDEO_STREAM_ID), videoStreamID)
      } else {
        putNull(ContextSpecificTag(TAG_VIDEO_STREAM_ID))
      }
      if (audioStreamID != null) {
        put(ContextSpecificTag(TAG_AUDIO_STREAM_ID), audioStreamID)
      } else {
        putNull(ContextSpecificTag(TAG_AUDIO_STREAM_ID))
      }
      put(ContextSpecificTag(TAG_METADATA_OPTIONS), metadataOptions)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ID = 1
    private const val TAG_PEER_NODE_ID = 2
    private const val TAG_PEER_FABRIC_INDEX = 3
    private const val TAG_STREAM_TYPE = 4
    private const val TAG_VIDEO_STREAM_ID = 5
    private const val TAG_AUDIO_STREAM_ID = 6
    private const val TAG_METADATA_OPTIONS = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): WebRTCTransportProviderClusterWebRTCSessionStruct {
      tlvReader.enterStructure(tlvTag)
      val id = tlvReader.getUShort(ContextSpecificTag(TAG_ID))
      val peerNodeID = tlvReader.getULong(ContextSpecificTag(TAG_PEER_NODE_ID))
      val peerFabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_PEER_FABRIC_INDEX))
      val streamType = tlvReader.getUByte(ContextSpecificTag(TAG_STREAM_TYPE))
      val videoStreamID =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_VIDEO_STREAM_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_VIDEO_STREAM_ID))
          null
        }
      val audioStreamID =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_AUDIO_STREAM_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_AUDIO_STREAM_ID))
          null
        }
      val metadataOptions = tlvReader.getUByte(ContextSpecificTag(TAG_METADATA_OPTIONS))

      tlvReader.exitContainer()

      return WebRTCTransportProviderClusterWebRTCSessionStruct(
        id,
        peerNodeID,
        peerFabricIndex,
        streamType,
        videoStreamID,
        audioStreamID,
        metadataOptions,
      )
    }
  }
}
