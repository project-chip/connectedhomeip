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
