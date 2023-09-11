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

class ContentLauncherClusterDimensionStruct(
  val width: Double,
  val height: Double,
  val metric: Int
) {
  override fun toString(): String = buildString {
    append("ContentLauncherClusterDimensionStruct {\n")
    append("\twidth : $width\n")
    append("\theight : $height\n")
    append("\tmetric : $metric\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_WIDTH), width)
      put(ContextSpecificTag(TAG_HEIGHT), height)
      put(ContextSpecificTag(TAG_METRIC), metric)
      endStructure()
    }
  }

  companion object {
    private const val TAG_WIDTH = 0
    private const val TAG_HEIGHT = 1
    private const val TAG_METRIC = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ContentLauncherClusterDimensionStruct {
      tlvReader.enterStructure(tag)
      val width = tlvReader.getDouble(ContextSpecificTag(TAG_WIDTH))
      val height = tlvReader.getDouble(ContextSpecificTag(TAG_HEIGHT))
      val metric = tlvReader.getInt(ContextSpecificTag(TAG_METRIC))

      tlvReader.exitContainer()

      return ContentLauncherClusterDimensionStruct(width, height, metric)
    }
  }
}
