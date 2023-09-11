/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

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

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ContentLauncherClusterStyleInformationStruct {
      tlvReader.enterStructure(tag)
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
