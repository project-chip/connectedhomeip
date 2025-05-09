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

class ContentLauncherClusterTrackPreferenceStruct(
  val languageCode: String,
  val characteristics: Optional<List<UByte>>,
  val audioOutputIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterTrackPreferenceStruct {\n")
    append("\tlanguageCode : $languageCode\n")
    append("\tcharacteristics : $characteristics\n")
    append("\taudioOutputIndex : $audioOutputIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LANGUAGE_CODE), languageCode)
      if (characteristics.isPresent) {
        val optcharacteristics = characteristics.get()
        startArray(ContextSpecificTag(TAG_CHARACTERISTICS))
        for (item in optcharacteristics.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      }
      put(ContextSpecificTag(TAG_AUDIO_OUTPUT_INDEX), audioOutputIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_LANGUAGE_CODE = 0
    private const val TAG_CHARACTERISTICS = 1
    private const val TAG_AUDIO_OUTPUT_INDEX = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterTrackPreferenceStruct {
      tlvReader.enterStructure(tlvTag)
      val languageCode = tlvReader.getString(ContextSpecificTag(TAG_LANGUAGE_CODE))
      val characteristics =
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
      val audioOutputIndex = tlvReader.getUByte(ContextSpecificTag(TAG_AUDIO_OUTPUT_INDEX))

      tlvReader.exitContainer()

      return ContentLauncherClusterTrackPreferenceStruct(
        languageCode,
        characteristics,
        audioOutputIndex,
      )
    }
  }
}
