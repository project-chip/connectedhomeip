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

class ChannelClusterProgramStruct(
  val identifier: String,
  val channel: ChannelClusterChannelInfoStruct,
  val startTime: UInt,
  val endTime: UInt,
  val title: String,
  val subtitle: Optional<String>,
  val description: Optional<String>,
  val audioLanguages: Optional<List<String>>,
  val ratings: Optional<List<String>>,
  val thumbnailUrl: Optional<String>,
  val posterArtUrl: Optional<String>,
  val dvbiUrl: Optional<String>,
  val releaseDate: Optional<String>,
  val parentalGuidanceText: Optional<String>,
  val recordingFlag: Optional<UInt>,
  val seriesInfo: Optional<ChannelClusterSeriesInfoStruct>?,
  val categoryList: Optional<List<ChannelClusterProgramCategoryStruct>>,
  val castList: Optional<List<ChannelClusterProgramCastStruct>>,
  val externalIDList: Optional<List<ChannelClusterProgramCastStruct>>
) {
  override fun toString(): String = buildString {
    append("ChannelClusterProgramStruct {\n")
    append("\tidentifier : $identifier\n")
    append("\tchannel : $channel\n")
    append("\tstartTime : $startTime\n")
    append("\tendTime : $endTime\n")
    append("\ttitle : $title\n")
    append("\tsubtitle : $subtitle\n")
    append("\tdescription : $description\n")
    append("\taudioLanguages : $audioLanguages\n")
    append("\tratings : $ratings\n")
    append("\tthumbnailUrl : $thumbnailUrl\n")
    append("\tposterArtUrl : $posterArtUrl\n")
    append("\tdvbiUrl : $dvbiUrl\n")
    append("\treleaseDate : $releaseDate\n")
    append("\tparentalGuidanceText : $parentalGuidanceText\n")
    append("\trecordingFlag : $recordingFlag\n")
    append("\tseriesInfo : $seriesInfo\n")
    append("\tcategoryList : $categoryList\n")
    append("\tcastList : $castList\n")
    append("\texternalIDList : $externalIDList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_IDENTIFIER), identifier)
      channel.toTlv(ContextSpecificTag(TAG_CHANNEL), this)
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      put(ContextSpecificTag(TAG_END_TIME), endTime)
      put(ContextSpecificTag(TAG_TITLE), title)
      if (subtitle.isPresent) {
        val optsubtitle = subtitle.get()
        put(ContextSpecificTag(TAG_SUBTITLE), optsubtitle)
      }
      if (description.isPresent) {
        val optdescription = description.get()
        put(ContextSpecificTag(TAG_DESCRIPTION), optdescription)
      }
      if (audioLanguages.isPresent) {
        val optaudioLanguages = audioLanguages.get()
        startArray(ContextSpecificTag(TAG_AUDIO_LANGUAGES))
        for (item in optaudioLanguages.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      }
      if (ratings.isPresent) {
        val optratings = ratings.get()
        startArray(ContextSpecificTag(TAG_RATINGS))
        for (item in optratings.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      }
      if (thumbnailUrl.isPresent) {
        val optthumbnailUrl = thumbnailUrl.get()
        put(ContextSpecificTag(TAG_THUMBNAIL_URL), optthumbnailUrl)
      }
      if (posterArtUrl.isPresent) {
        val optposterArtUrl = posterArtUrl.get()
        put(ContextSpecificTag(TAG_POSTER_ART_URL), optposterArtUrl)
      }
      if (dvbiUrl.isPresent) {
        val optdvbiUrl = dvbiUrl.get()
        put(ContextSpecificTag(TAG_DVBI_URL), optdvbiUrl)
      }
      if (releaseDate.isPresent) {
        val optreleaseDate = releaseDate.get()
        put(ContextSpecificTag(TAG_RELEASE_DATE), optreleaseDate)
      }
      if (parentalGuidanceText.isPresent) {
        val optparentalGuidanceText = parentalGuidanceText.get()
        put(ContextSpecificTag(TAG_PARENTAL_GUIDANCE_TEXT), optparentalGuidanceText)
      }
      if (recordingFlag.isPresent) {
        val optrecordingFlag = recordingFlag.get()
        put(ContextSpecificTag(TAG_RECORDING_FLAG), optrecordingFlag)
      }
      if (seriesInfo != null) {
        if (seriesInfo.isPresent) {
          val optseriesInfo = seriesInfo.get()
          optseriesInfo.toTlv(ContextSpecificTag(TAG_SERIES_INFO), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_SERIES_INFO))
      }
      if (categoryList.isPresent) {
        val optcategoryList = categoryList.get()
        startArray(ContextSpecificTag(TAG_CATEGORY_LIST))
        for (item in optcategoryList.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (castList.isPresent) {
        val optcastList = castList.get()
        startArray(ContextSpecificTag(TAG_CAST_LIST))
        for (item in optcastList.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      if (externalIDList.isPresent) {
        val optexternalIDList = externalIDList.get()
        startArray(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))
        for (item in optexternalIDList.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_IDENTIFIER = 0
    private const val TAG_CHANNEL = 1
    private const val TAG_START_TIME = 2
    private const val TAG_END_TIME = 3
    private const val TAG_TITLE = 4
    private const val TAG_SUBTITLE = 5
    private const val TAG_DESCRIPTION = 6
    private const val TAG_AUDIO_LANGUAGES = 7
    private const val TAG_RATINGS = 8
    private const val TAG_THUMBNAIL_URL = 9
    private const val TAG_POSTER_ART_URL = 10
    private const val TAG_DVBI_URL = 11
    private const val TAG_RELEASE_DATE = 12
    private const val TAG_PARENTAL_GUIDANCE_TEXT = 13
    private const val TAG_RECORDING_FLAG = 14
    private const val TAG_SERIES_INFO = 15
    private const val TAG_CATEGORY_LIST = 16
    private const val TAG_CAST_LIST = 17
    private const val TAG_EXTERNAL_I_D_LIST = 18

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ChannelClusterProgramStruct {
      tlvReader.enterStructure(tlvTag)
      val identifier = tlvReader.getString(ContextSpecificTag(TAG_IDENTIFIER))
      val channel =
        ChannelClusterChannelInfoStruct.fromTlv(ContextSpecificTag(TAG_CHANNEL), tlvReader)
      val startTime = tlvReader.getUInt(ContextSpecificTag(TAG_START_TIME))
      val endTime = tlvReader.getUInt(ContextSpecificTag(TAG_END_TIME))
      val title = tlvReader.getString(ContextSpecificTag(TAG_TITLE))
      val subtitle =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SUBTITLE))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_SUBTITLE)))
        } else {
          Optional.empty()
        }
      val description =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DESCRIPTION))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DESCRIPTION)))
        } else {
          Optional.empty()
        }
      val audioLanguages =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AUDIO_LANGUAGES))) {
          Optional.of(
            buildList<String> {
              tlvReader.enterArray(ContextSpecificTag(TAG_AUDIO_LANGUAGES))
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getString(AnonymousTag))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val ratings =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RATINGS))) {
          Optional.of(
            buildList<String> {
              tlvReader.enterArray(ContextSpecificTag(TAG_RATINGS))
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getString(AnonymousTag))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val thumbnailUrl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_THUMBNAIL_URL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_THUMBNAIL_URL)))
        } else {
          Optional.empty()
        }
      val posterArtUrl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_POSTER_ART_URL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_POSTER_ART_URL)))
        } else {
          Optional.empty()
        }
      val dvbiUrl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DVBI_URL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DVBI_URL)))
        } else {
          Optional.empty()
        }
      val releaseDate =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RELEASE_DATE))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_RELEASE_DATE)))
        } else {
          Optional.empty()
        }
      val parentalGuidanceText =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PARENTAL_GUIDANCE_TEXT))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_PARENTAL_GUIDANCE_TEXT)))
        } else {
          Optional.empty()
        }
      val recordingFlag =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RECORDING_FLAG))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_RECORDING_FLAG)))
        } else {
          Optional.empty()
        }
      val seriesInfo =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_SERIES_INFO))) {
            Optional.of(
              ChannelClusterSeriesInfoStruct.fromTlv(ContextSpecificTag(TAG_SERIES_INFO), tlvReader)
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SERIES_INFO))
          null
        }
      val categoryList =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CATEGORY_LIST))) {
          Optional.of(
            buildList<ChannelClusterProgramCategoryStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_CATEGORY_LIST))
              while (!tlvReader.isEndOfContainer()) {
                add(ChannelClusterProgramCategoryStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val castList =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CAST_LIST))) {
          Optional.of(
            buildList<ChannelClusterProgramCastStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_CAST_LIST))
              while (!tlvReader.isEndOfContainer()) {
                add(ChannelClusterProgramCastStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val externalIDList =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))) {
          Optional.of(
            buildList<ChannelClusterProgramCastStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_EXTERNAL_I_D_LIST))
              while (!tlvReader.isEndOfContainer()) {
                add(ChannelClusterProgramCastStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ChannelClusterProgramStruct(
        identifier,
        channel,
        startTime,
        endTime,
        title,
        subtitle,
        description,
        audioLanguages,
        ratings,
        thumbnailUrl,
        posterArtUrl,
        dvbiUrl,
        releaseDate,
        parentalGuidanceText,
        recordingFlag,
        seriesInfo,
        categoryList,
        castList,
        externalIDList
      )
    }
  }
}
