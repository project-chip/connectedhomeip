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

class MediaPlaybackClusterContentInfoStruct(
  val contentType: UInt,
  val title: Optional<String>?,
  val show: Optional<String>?,
  val season: Optional<String>?,
  val episode: Optional<String>?,
  val provider: Optional<String>?,
  val artist: Optional<String>?,
  val album: Optional<String>?,
  val track: Optional<String>?,
) {
  override fun toString(): String = buildString {
    append("MediaPlaybackClusterContentInfoStruct {\n")
    append("\tcontentType : $contentType\n")
    append("\ttitle : $title\n")
    append("\tshow : $show\n")
    append("\tseason : $season\n")
    append("\tepisode : $episode\n")
    append("\tprovider : $provider\n")
    append("\tartist : $artist\n")
    append("\talbum : $album\n")
    append("\ttrack : $track\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONTENT_TYPE), contentType)
      if (title != null) {
        if (title.isPresent) {
          val opttitle = title.get()
          put(ContextSpecificTag(TAG_TITLE), opttitle)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TITLE))
      }
      if (show != null) {
        if (show.isPresent) {
          val optshow = show.get()
          put(ContextSpecificTag(TAG_SHOW), optshow)
        }
      } else {
        putNull(ContextSpecificTag(TAG_SHOW))
      }
      if (season != null) {
        if (season.isPresent) {
          val optseason = season.get()
          put(ContextSpecificTag(TAG_SEASON), optseason)
        }
      } else {
        putNull(ContextSpecificTag(TAG_SEASON))
      }
      if (episode != null) {
        if (episode.isPresent) {
          val optepisode = episode.get()
          put(ContextSpecificTag(TAG_EPISODE), optepisode)
        }
      } else {
        putNull(ContextSpecificTag(TAG_EPISODE))
      }
      if (provider != null) {
        if (provider.isPresent) {
          val optprovider = provider.get()
          put(ContextSpecificTag(TAG_PROVIDER), optprovider)
        }
      } else {
        putNull(ContextSpecificTag(TAG_PROVIDER))
      }
      if (artist != null) {
        if (artist.isPresent) {
          val optartist = artist.get()
          put(ContextSpecificTag(TAG_ARTIST), optartist)
        }
      } else {
        putNull(ContextSpecificTag(TAG_ARTIST))
      }
      if (album != null) {
        if (album.isPresent) {
          val optalbum = album.get()
          put(ContextSpecificTag(TAG_ALBUM), optalbum)
        }
      } else {
        putNull(ContextSpecificTag(TAG_ALBUM))
      }
      if (track != null) {
        if (track.isPresent) {
          val opttrack = track.get()
          put(ContextSpecificTag(TAG_TRACK), opttrack)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TRACK))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTENT_TYPE = 0
    private const val TAG_TITLE = 1
    private const val TAG_SHOW = 2
    private const val TAG_SEASON = 3
    private const val TAG_EPISODE = 4
    private const val TAG_PROVIDER = 5
    private const val TAG_ARTIST = 6
    private const val TAG_ALBUM = 7
    private const val TAG_TRACK = 8

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MediaPlaybackClusterContentInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val contentType = tlvReader.getUInt(ContextSpecificTag(TAG_CONTENT_TYPE))
      val title =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TITLE))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_TITLE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TITLE))
          null
        }
      val show =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_SHOW))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_SHOW)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SHOW))
          null
        }
      val season =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_SEASON))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_SEASON)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SEASON))
          null
        }
      val episode =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_EPISODE))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_EPISODE)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_EPISODE))
          null
        }
      val provider =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_PROVIDER))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_PROVIDER)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PROVIDER))
          null
        }
      val artist =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_ARTIST))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_ARTIST)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ARTIST))
          null
        }
      val album =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_ALBUM))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_ALBUM)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ALBUM))
          null
        }
      val track =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TRACK))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_TRACK)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TRACK))
          null
        }

      tlvReader.exitContainer()

      return MediaPlaybackClusterContentInfoStruct(
        contentType,
        title,
        show,
        season,
        episode,
        provider,
        artist,
        album,
        track,
      )
    }
  }
}
