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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentLauncherClusterPlaybackPreferencesStruct(
  val playbackPosition: Optional<ULong>?,
  val textTrack: Optional<ContentLauncherClusterTrackPreferenceStruct>?,
  val audioTracks: Optional<List<ContentLauncherClusterTrackPreferenceStruct>>?
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterPlaybackPreferencesStruct {\n")
    append("\tplaybackPosition : $playbackPosition\n")
    append("\ttextTrack : $textTrack\n")
    append("\taudioTracks : $audioTracks\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (playbackPosition != null) {
        if (playbackPosition.isPresent) {
          val optplaybackPosition = playbackPosition.get()
          put(ContextSpecificTag(TAG_PLAYBACK_POSITION), optplaybackPosition)
        }
      } else {
        putNull(ContextSpecificTag(TAG_PLAYBACK_POSITION))
      }
      if (textTrack != null) {
        if (textTrack.isPresent) {
          val opttextTrack = textTrack.get()
          opttextTrack.toTlv(ContextSpecificTag(TAG_TEXT_TRACK), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TEXT_TRACK))
      }
      if (audioTracks != null) {
        if (audioTracks.isPresent) {
          val optaudioTracks = audioTracks.get()
          startArray(ContextSpecificTag(TAG_AUDIO_TRACKS))
          for (item in optaudioTracks.iterator()) {
            item.toTlv(AnonymousTag, this)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_AUDIO_TRACKS))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PLAYBACK_POSITION = 0
    private const val TAG_TEXT_TRACK = 1
    private const val TAG_AUDIO_TRACKS = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ContentLauncherClusterPlaybackPreferencesStruct {
      tlvReader.enterStructure(tlvTag)
      val playbackPosition =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_PLAYBACK_POSITION))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_PLAYBACK_POSITION)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PLAYBACK_POSITION))
          null
        }
      val textTrack =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TEXT_TRACK))) {
            Optional.of(
              ContentLauncherClusterTrackPreferenceStruct.fromTlv(
                ContextSpecificTag(TAG_TEXT_TRACK),
                tlvReader
              )
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TEXT_TRACK))
          null
        }
      val audioTracks =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUDIO_TRACKS))) {
            Optional.of(
              buildList<ContentLauncherClusterTrackPreferenceStruct> {
                tlvReader.enterArray(ContextSpecificTag(TAG_AUDIO_TRACKS))
                while (!tlvReader.isEndOfContainer()) {
                  add(ContentLauncherClusterTrackPreferenceStruct.fromTlv(AnonymousTag, tlvReader))
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_AUDIO_TRACKS))
          null
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterPlaybackPreferencesStruct(
        playbackPosition,
        textTrack,
        audioTracks
      )
    }
  }
}
