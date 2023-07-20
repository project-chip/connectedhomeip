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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ContentLauncherClusterDimensionStruct (
    val width: Double,
    val height: Double,
    val metric: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ContentLauncherClusterDimensionStruct {\n")
    builder.append("\twidth : $width\n")
    builder.append("\theight : $height\n")
    builder.append("\tmetric : $metric\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_WIDTH), width)
    tlvWriter.put(ContextSpecificTag(TAG_HEIGHT), height)
    tlvWriter.put(ContextSpecificTag(TAG_METRIC), metric)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_WIDTH = 0
    private const val TAG_HEIGHT = 1
    private const val TAG_METRIC = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ContentLauncherClusterDimensionStruct {
      tlvReader.enterStructure(tag)
      val width: Double = tlvReader.getDouble(ContextSpecificTag(TAG_WIDTH))
      val height: Double = tlvReader.getDouble(ContextSpecificTag(TAG_HEIGHT))
      val metric: Int = tlvReader.getInt(ContextSpecificTag(TAG_METRIC))
      
      tlvReader.exitContainer()

      return ContentLauncherClusterDimensionStruct(width, height, metric)
    }
  }
}
