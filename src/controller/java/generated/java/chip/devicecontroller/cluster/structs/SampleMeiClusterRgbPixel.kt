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

class SampleMeiClusterRgbPixel(val r: Int, val g: Int, val b: Int) {
  override fun toString(): String = buildString {
    append("SampleMeiClusterRgbPixel {\n")
    append("\tr : $r\n")
    append("\tg : $g\n")
    append("\tb : $b\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_R), r)
      put(ContextSpecificTag(TAG_G), g)
      put(ContextSpecificTag(TAG_B), b)
      endStructure()
    }
  }

  companion object {
    private const val TAG_R = 0
    private const val TAG_G = 1
    private const val TAG_B = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SampleMeiClusterRgbPixel {
      tlvReader.enterStructure(tag)
      val r = tlvReader.getInt(ContextSpecificTag(TAG_R))
      val g = tlvReader.getInt(ContextSpecificTag(TAG_G))
      val b = tlvReader.getInt(ContextSpecificTag(TAG_B))

      tlvReader.exitContainer()

      return SampleMeiClusterRgbPixel(r, g, b)
    }
  }
}
