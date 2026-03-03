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

class ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct(
  val rangeMin: Long,
  val rangeMax: Long,
  val percentMax: Optional<UInt>,
  val percentMin: Optional<UInt>,
  val percentTypical: Optional<UInt>,
  val fixedMax: Optional<ULong>,
  val fixedMin: Optional<ULong>,
  val fixedTypical: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct {\n")
    append("\trangeMin : $rangeMin\n")
    append("\trangeMax : $rangeMax\n")
    append("\tpercentMax : $percentMax\n")
    append("\tpercentMin : $percentMin\n")
    append("\tpercentTypical : $percentTypical\n")
    append("\tfixedMax : $fixedMax\n")
    append("\tfixedMin : $fixedMin\n")
    append("\tfixedTypical : $fixedTypical\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_RANGE_MIN), rangeMin)
      put(ContextSpecificTag(TAG_RANGE_MAX), rangeMax)
      if (percentMax.isPresent) {
        val optpercentMax = percentMax.get()
        put(ContextSpecificTag(TAG_PERCENT_MAX), optpercentMax)
      }
      if (percentMin.isPresent) {
        val optpercentMin = percentMin.get()
        put(ContextSpecificTag(TAG_PERCENT_MIN), optpercentMin)
      }
      if (percentTypical.isPresent) {
        val optpercentTypical = percentTypical.get()
        put(ContextSpecificTag(TAG_PERCENT_TYPICAL), optpercentTypical)
      }
      if (fixedMax.isPresent) {
        val optfixedMax = fixedMax.get()
        put(ContextSpecificTag(TAG_FIXED_MAX), optfixedMax)
      }
      if (fixedMin.isPresent) {
        val optfixedMin = fixedMin.get()
        put(ContextSpecificTag(TAG_FIXED_MIN), optfixedMin)
      }
      if (fixedTypical.isPresent) {
        val optfixedTypical = fixedTypical.get()
        put(ContextSpecificTag(TAG_FIXED_TYPICAL), optfixedTypical)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_RANGE_MIN = 0
    private const val TAG_RANGE_MAX = 1
    private const val TAG_PERCENT_MAX = 2
    private const val TAG_PERCENT_MIN = 3
    private const val TAG_PERCENT_TYPICAL = 4
    private const val TAG_FIXED_MAX = 5
    private const val TAG_FIXED_MIN = 6
    private const val TAG_FIXED_TYPICAL = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct {
      tlvReader.enterStructure(tlvTag)
      val rangeMin = tlvReader.getLong(ContextSpecificTag(TAG_RANGE_MIN))
      val rangeMax = tlvReader.getLong(ContextSpecificTag(TAG_RANGE_MAX))
      val percentMax =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PERCENT_MAX))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PERCENT_MAX)))
        } else {
          Optional.empty()
        }
      val percentMin =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PERCENT_MIN))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PERCENT_MIN)))
        } else {
          Optional.empty()
        }
      val percentTypical =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_PERCENT_TYPICAL))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_PERCENT_TYPICAL)))
        } else {
          Optional.empty()
        }
      val fixedMax =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_FIXED_MAX))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_FIXED_MAX)))
        } else {
          Optional.empty()
        }
      val fixedMin =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_FIXED_MIN))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_FIXED_MIN)))
        } else {
          Optional.empty()
        }
      val fixedTypical =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_FIXED_TYPICAL))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_FIXED_TYPICAL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct(
        rangeMin,
        rangeMax,
        percentMax,
        percentMin,
        percentTypical,
        fixedMax,
        fixedMin,
        fixedTypical,
      )
    }
  }
}
