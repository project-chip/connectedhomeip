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

class ContentLauncherClusterStyleInformationStruct(
  val imageURL: Optional<String>,
  val color: Optional<String>,
  val size: Optional<ContentLauncherClusterDimensionStruct>
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterStyleInformationStruct {\n")
    append("\timageURL : $imageURL\n")
    append("\tcolor : $color\n")
    append("\tsize : $size\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (imageURL.isPresent) {
        val optimageURL = imageURL.get()
        put(ContextSpecificTag(TAG_IMAGE_U_R_L), optimageURL)
      }
      if (color.isPresent) {
        val optcolor = color.get()
        put(ContextSpecificTag(TAG_COLOR), optcolor)
      }
      if (size.isPresent) {
        val optsize = size.get()
        optsize.toTlv(ContextSpecificTag(TAG_SIZE), this)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_IMAGE_U_R_L = 0
    private const val TAG_COLOR = 1
    private const val TAG_SIZE = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentLauncherClusterStyleInformationStruct {
      tlvReader.enterStructure(tlvTag)
      val imageURL =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_IMAGE_U_R_L))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_IMAGE_U_R_L)))
        } else {
          Optional.empty()
        }
      val color =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_COLOR))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_COLOR)))
        } else {
          Optional.empty()
        }
      val size =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SIZE))) {
          Optional.of(
            ContentLauncherClusterDimensionStruct.fromTlv(ContextSpecificTag(TAG_SIZE), tlvReader)
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ContentLauncherClusterStyleInformationStruct(imageURL, color, size)
    }
  }
}
