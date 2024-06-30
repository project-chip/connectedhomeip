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

class ContentControlClusterRatingNameStruct(
  val ratingName: String,
  val ratingNameDesc: Optional<String>
) {
  override fun toString(): String = buildString {
    append("ContentControlClusterRatingNameStruct {\n")
    append("\tratingName : $ratingName\n")
    append("\tratingNameDesc : $ratingNameDesc\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_RATING_NAME), ratingName)
      if (ratingNameDesc.isPresent) {
        val optratingNameDesc = ratingNameDesc.get()
        put(ContextSpecificTag(TAG_RATING_NAME_DESC), optratingNameDesc)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_RATING_NAME = 0
    private const val TAG_RATING_NAME_DESC = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentControlClusterRatingNameStruct {
      tlvReader.enterStructure(tlvTag)
      val ratingName = tlvReader.getString(ContextSpecificTag(TAG_RATING_NAME))
      val ratingNameDesc =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_RATING_NAME_DESC))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_RATING_NAME_DESC)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ContentControlClusterRatingNameStruct(ratingName, ratingNameDesc)
    }
  }
}
