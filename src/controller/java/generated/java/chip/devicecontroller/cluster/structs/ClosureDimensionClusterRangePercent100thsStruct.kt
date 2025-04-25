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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ClosureDimensionClusterRangePercent100thsStruct(val min: UInt, val max: UInt) {
  override fun toString(): String = buildString {
    append("ClosureDimensionClusterRangePercent100thsStruct {\n")
    append("\tmin : $min\n")
    append("\tmax : $max\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MIN), min)
      put(ContextSpecificTag(TAG_MAX), max)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MIN = 0
    private const val TAG_MAX = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ClosureDimensionClusterRangePercent100thsStruct {
      tlvReader.enterStructure(tlvTag)
      val min = tlvReader.getUInt(ContextSpecificTag(TAG_MIN))
      val max = tlvReader.getUInt(ContextSpecificTag(TAG_MAX))

      tlvReader.exitContainer()

      return ClosureDimensionClusterRangePercent100thsStruct(min, max)
    }
  }
}
