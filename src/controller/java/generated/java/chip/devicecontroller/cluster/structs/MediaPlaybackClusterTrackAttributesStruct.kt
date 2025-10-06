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

class MediaPlaybackClusterTrackAttributesStruct(
  val languageCode: String,
  val displayName: Optional<String>?,
) {
  override fun toString(): String = buildString {
    append("MediaPlaybackClusterTrackAttributesStruct {\n")
    append("\tlanguageCode : $languageCode\n")
    append("\tdisplayName : $displayName\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LANGUAGE_CODE), languageCode)
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
    private const val TAG_DISPLAY_NAME = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): MediaPlaybackClusterTrackAttributesStruct {
      tlvReader.enterStructure(tlvTag)
      val languageCode = tlvReader.getString(ContextSpecificTag(TAG_LANGUAGE_CODE))
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

      return MediaPlaybackClusterTrackAttributesStruct(languageCode, displayName)
    }
  }
}
