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

class MediaPlaybackClusterStateChangedEvent(
  val currentState: UInt,
  val startTime: ULong,
  val duration: ULong,
  val sampledPosition:
    chip.devicecontroller.cluster.structs.MediaPlaybackClusterPlaybackPositionStruct,
  val playbackSpeed: Float,
  val seekRangeEnd: ULong,
  val seekRangeStart: ULong,
  val data: Optional<ByteArray>,
  val audioAdvanceUnmuted: Boolean
) {
  override fun toString(): String = buildString {
    append("MediaPlaybackClusterStateChangedEvent {\n")
    append("\tcurrentState : $currentState\n")
    append("\tstartTime : $startTime\n")
    append("\tduration : $duration\n")
    append("\tsampledPosition : $sampledPosition\n")
    append("\tplaybackSpeed : $playbackSpeed\n")
    append("\tseekRangeEnd : $seekRangeEnd\n")
    append("\tseekRangeStart : $seekRangeStart\n")
    append("\tdata : $data\n")
    append("\taudioAdvanceUnmuted : $audioAdvanceUnmuted\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CURRENT_STATE), currentState)
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      put(ContextSpecificTag(TAG_DURATION), duration)
      sampledPosition.toTlv(ContextSpecificTag(TAG_SAMPLED_POSITION), this)
      put(ContextSpecificTag(TAG_PLAYBACK_SPEED), playbackSpeed)
      put(ContextSpecificTag(TAG_SEEK_RANGE_END), seekRangeEnd)
      put(ContextSpecificTag(TAG_SEEK_RANGE_START), seekRangeStart)
      if (data.isPresent) {
        val optdata = data.get()
        put(ContextSpecificTag(TAG_DATA), optdata)
      }
      put(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED), audioAdvanceUnmuted)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CURRENT_STATE = 0
    private const val TAG_START_TIME = 1
    private const val TAG_DURATION = 2
    private const val TAG_SAMPLED_POSITION = 3
    private const val TAG_PLAYBACK_SPEED = 4
    private const val TAG_SEEK_RANGE_END = 5
    private const val TAG_SEEK_RANGE_START = 6
    private const val TAG_DATA = 7
    private const val TAG_AUDIO_ADVANCE_UNMUTED = 8

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MediaPlaybackClusterStateChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val currentState = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_STATE))
      val startTime = tlvReader.getULong(ContextSpecificTag(TAG_START_TIME))
      val duration = tlvReader.getULong(ContextSpecificTag(TAG_DURATION))
      val sampledPosition =
        chip.devicecontroller.cluster.structs.MediaPlaybackClusterPlaybackPositionStruct.fromTlv(
          ContextSpecificTag(TAG_SAMPLED_POSITION),
          tlvReader
        )
      val playbackSpeed = tlvReader.getFloat(ContextSpecificTag(TAG_PLAYBACK_SPEED))
      val seekRangeEnd = tlvReader.getULong(ContextSpecificTag(TAG_SEEK_RANGE_END))
      val seekRangeStart = tlvReader.getULong(ContextSpecificTag(TAG_SEEK_RANGE_START))
      val data =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_DATA)))
        } else {
          Optional.empty()
        }
      val audioAdvanceUnmuted = tlvReader.getBoolean(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED))

      tlvReader.exitContainer()

      return MediaPlaybackClusterStateChangedEvent(
        currentState,
        startTime,
        duration,
        sampledPosition,
        playbackSpeed,
        seekRangeEnd,
        seekRangeStart,
        data,
        audioAdvanceUnmuted
      )
    }
  }
}
