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
  val startTime: Optional<ULong>,
  val duration: Optional<ULong>,
  val sampledPosition:
    Optional<chip.devicecontroller.cluster.structs.MediaPlaybackClusterPlaybackPositionStruct>,
  val playbackSpeed: Optional<Float>,
  val seekRangeEnd: Optional<ULong>,
  val seekRangeStart: Optional<ULong>,
  val data: Optional<ByteArray>,
  val audioAdvanceUnmuted: Optional<Boolean>
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
      if (startTime.isPresent) {
        val optstartTime = startTime.get()
        put(ContextSpecificTag(TAG_START_TIME), optstartTime)
      }
      if (duration.isPresent) {
        val optduration = duration.get()
        put(ContextSpecificTag(TAG_DURATION), optduration)
      }
      if (sampledPosition.isPresent) {
        val optsampledPosition = sampledPosition.get()
        optsampledPosition.toTlv(ContextSpecificTag(TAG_SAMPLED_POSITION), this)
      }
      if (playbackSpeed.isPresent) {
        val optplaybackSpeed = playbackSpeed.get()
        put(ContextSpecificTag(TAG_PLAYBACK_SPEED), optplaybackSpeed)
      }
      if (seekRangeEnd.isPresent) {
        val optseekRangeEnd = seekRangeEnd.get()
        put(ContextSpecificTag(TAG_SEEK_RANGE_END), optseekRangeEnd)
      }
      if (seekRangeStart.isPresent) {
        val optseekRangeStart = seekRangeStart.get()
        put(ContextSpecificTag(TAG_SEEK_RANGE_START), optseekRangeStart)
      }
      if (data.isPresent) {
        val optdata = data.get()
        put(ContextSpecificTag(TAG_DATA), optdata)
      }
      if (audioAdvanceUnmuted.isPresent) {
        val optaudioAdvanceUnmuted = audioAdvanceUnmuted.get()
        put(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED), optaudioAdvanceUnmuted)
      }
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
      val startTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_TIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_START_TIME)))
        } else {
          Optional.empty()
        }
      val duration =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DURATION))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_DURATION)))
        } else {
          Optional.empty()
        }
      val sampledPosition =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SAMPLED_POSITION))) {
          Optional.of(
            chip.devicecontroller.cluster.structs.MediaPlaybackClusterPlaybackPositionStruct
              .fromTlv(ContextSpecificTag(TAG_SAMPLED_POSITION), tlvReader)
          )
        } else {
          Optional.empty()
        }
      val playbackSpeed =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PLAYBACK_SPEED))) {
          Optional.of(tlvReader.getFloat(ContextSpecificTag(TAG_PLAYBACK_SPEED)))
        } else {
          Optional.empty()
        }
      val seekRangeEnd =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SEEK_RANGE_END))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_SEEK_RANGE_END)))
        } else {
          Optional.empty()
        }
      val seekRangeStart =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SEEK_RANGE_START))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_SEEK_RANGE_START)))
        } else {
          Optional.empty()
        }
      val data =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DATA))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_DATA)))
        } else {
          Optional.empty()
        }
      val audioAdvanceUnmuted =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_AUDIO_ADVANCE_UNMUTED)))
        } else {
          Optional.empty()
        }

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
