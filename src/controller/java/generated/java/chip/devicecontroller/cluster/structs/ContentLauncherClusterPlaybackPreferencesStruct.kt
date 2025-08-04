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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentLauncherClusterPlaybackPreferencesStruct(
  val playbackPosition: ULong,
  val textTrack: ContentLauncherClusterTrackPreferenceStruct,
  val audioTracks: Optional<List<ContentLauncherClusterTrackPreferenceStruct>>,
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
      put(ContextSpecificTag(TAG_PLAYBACK_POSITION), playbackPosition)
      textTrack.toTlv(ContextSpecificTag(TAG_TEXT_TRACK), this)
      if (audioTracks.isPresent) {
        val optaudioTracks = audioTracks.get()
        startArray(ContextSpecificTag(TAG_AUDIO_TRACKS))
        for (item in optaudioTracks.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
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
      tlvReader: TlvReader,
    ): ContentLauncherClusterPlaybackPreferencesStruct {
      tlvReader.enterStructure(tlvTag)
      val playbackPosition = tlvReader.getULong(ContextSpecificTag(TAG_PLAYBACK_POSITION))
      val textTrack =
        ContentLauncherClusterTrackPreferenceStruct.fromTlv(
          ContextSpecificTag(TAG_TEXT_TRACK),
          tlvReader,
        )
      val audioTracks =
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

      tlvReader.exitContainer()

      return ContentLauncherClusterPlaybackPreferencesStruct(
        playbackPosition,
        textTrack,
        audioTracks,
      )
    }
  }
}
