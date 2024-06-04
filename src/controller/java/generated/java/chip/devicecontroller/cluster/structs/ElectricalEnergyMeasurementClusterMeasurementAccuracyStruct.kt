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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct(
  val measurementType: UInt,
  val measured: Boolean,
  val minMeasuredValue: Long,
  val maxMeasuredValue: Long,
  val accuracyRanges: List<ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct>
) {
  override fun toString(): String = buildString {
    append("ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct {\n")
    append("\tmeasurementType : $measurementType\n")
    append("\tmeasured : $measured\n")
    append("\tminMeasuredValue : $minMeasuredValue\n")
    append("\tmaxMeasuredValue : $maxMeasuredValue\n")
    append("\taccuracyRanges : $accuracyRanges\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MEASUREMENT_TYPE), measurementType)
      put(ContextSpecificTag(TAG_MEASURED), measured)
      put(ContextSpecificTag(TAG_MIN_MEASURED_VALUE), minMeasuredValue)
      put(ContextSpecificTag(TAG_MAX_MEASURED_VALUE), maxMeasuredValue)
      startArray(ContextSpecificTag(TAG_ACCURACY_RANGES))
      for (item in accuracyRanges.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_MEASUREMENT_TYPE = 0
    private const val TAG_MEASURED = 1
    private const val TAG_MIN_MEASURED_VALUE = 2
    private const val TAG_MAX_MEASURED_VALUE = 3
    private const val TAG_ACCURACY_RANGES = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct {
      tlvReader.enterStructure(tlvTag)
      val measurementType = tlvReader.getUInt(ContextSpecificTag(TAG_MEASUREMENT_TYPE))
      val measured = tlvReader.getBoolean(ContextSpecificTag(TAG_MEASURED))
      val minMeasuredValue = tlvReader.getLong(ContextSpecificTag(TAG_MIN_MEASURED_VALUE))
      val maxMeasuredValue = tlvReader.getLong(ContextSpecificTag(TAG_MAX_MEASURED_VALUE))
      val accuracyRanges =
        buildList<ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ACCURACY_RANGES))
          while (!tlvReader.isEndOfContainer()) {
            add(
              ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct.fromTlv(
                AnonymousTag,
                tlvReader
              )
            )
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct(
        measurementType,
        measured,
        minMeasuredValue,
        maxMeasuredValue,
        accuracyRanges
      )
    }
  }
}
