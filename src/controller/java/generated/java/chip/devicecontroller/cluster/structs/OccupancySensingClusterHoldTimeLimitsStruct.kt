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

class OccupancySensingClusterHoldTimeLimitsStruct(
  val holdTimeMin: UInt,
  val holdTimeMax: UInt,
  val holdTimeDefault: UInt,
) {
  override fun toString(): String = buildString {
    append("OccupancySensingClusterHoldTimeLimitsStruct {\n")
    append("\tholdTimeMin : $holdTimeMin\n")
    append("\tholdTimeMax : $holdTimeMax\n")
    append("\tholdTimeDefault : $holdTimeDefault\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_HOLD_TIME_MIN), holdTimeMin)
      put(ContextSpecificTag(TAG_HOLD_TIME_MAX), holdTimeMax)
      put(ContextSpecificTag(TAG_HOLD_TIME_DEFAULT), holdTimeDefault)
      endStructure()
    }
  }

  companion object {
    private const val TAG_HOLD_TIME_MIN = 0
    private const val TAG_HOLD_TIME_MAX = 1
    private const val TAG_HOLD_TIME_DEFAULT = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): OccupancySensingClusterHoldTimeLimitsStruct {
      tlvReader.enterStructure(tlvTag)
      val holdTimeMin = tlvReader.getUInt(ContextSpecificTag(TAG_HOLD_TIME_MIN))
      val holdTimeMax = tlvReader.getUInt(ContextSpecificTag(TAG_HOLD_TIME_MAX))
      val holdTimeDefault = tlvReader.getUInt(ContextSpecificTag(TAG_HOLD_TIME_DEFAULT))

      tlvReader.exitContainer()

      return OccupancySensingClusterHoldTimeLimitsStruct(holdTimeMin, holdTimeMax, holdTimeDefault)
    }
  }
}
