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

class MediaPlaybackClusterTrackAttributesStruct(
  val languageCode: String,
  val characteristics: Optional<List<UByte>>?,
  val displayName: Optional<String>?
) {
  override fun toString(): String = buildString {
    append("MediaPlaybackClusterTrackAttributesStruct {\n")
    append("\tlanguageCode : $languageCode\n")
    append("\tcharacteristics : $characteristics\n")
    append("\tdisplayName : $displayName\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LANGUAGE_CODE), languageCode)
      if (characteristics != null) {
        if (characteristics.isPresent) {
          val optcharacteristics = characteristics.get()
          startArray(ContextSpecificTag(TAG_CHARACTERISTICS))
          for (item in optcharacteristics.iterator()) {
            put(AnonymousTag, item)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_CHARACTERISTICS))
      }
      if (displayName != null) {
        if (displayName.isPresent) {
          val optdisplayName = displayName.get()
          put(ContextSpecificTag(TAG_DISPLAY_NAME), optdisplayName)
        }
      } else {
        putNull(ContextSpecificTag(TAG_DISPLAY_NAME))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LANGUAGE_CODE = 0
    private const val TAG_CHARACTERISTICS = 1
    private const val TAG_DISPLAY_NAME = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MediaPlaybackClusterTrackAttributesStruct {
      tlvReader.enterStructure(tlvTag)
      val languageCode = tlvReader.getString(ContextSpecificTag(TAG_LANGUAGE_CODE))
      val characteristics =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CHARACTERISTICS))) {
            Optional.of(
              buildList<UByte> {
                tlvReader.enterArray(ContextSpecificTag(TAG_CHARACTERISTICS))
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CHARACTERISTICS))
          null
        }
      val displayName =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_DISPLAY_NAME))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DISPLAY_NAME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DISPLAY_NAME))
          null
        }

      tlvReader.exitContainer()

      return MediaPlaybackClusterTrackAttributesStruct(languageCode, characteristics, displayName)
    }
  }
}
