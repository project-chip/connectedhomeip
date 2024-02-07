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

class ElectricalPowerMeasurementClusterHarmonicMeasurementStruct(
  val order: UInt,
  val measurement: Long?
) {
  override fun toString(): String = buildString {
    append("ElectricalPowerMeasurementClusterHarmonicMeasurementStruct {\n")
    append("\torder : $order\n")
    append("\tmeasurement : $measurement\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ORDER), order)
      if (measurement != null) {
        put(ContextSpecificTag(TAG_MEASUREMENT), measurement)
      } else {
        putNull(ContextSpecificTag(TAG_MEASUREMENT))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ORDER = 0
    private const val TAG_MEASUREMENT = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ElectricalPowerMeasurementClusterHarmonicMeasurementStruct {
      tlvReader.enterStructure(tlvTag)
      val order = tlvReader.getUInt(ContextSpecificTag(TAG_ORDER))
      val measurement =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_MEASUREMENT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MEASUREMENT))
          null
        }

      tlvReader.exitContainer()

      return ElectricalPowerMeasurementClusterHarmonicMeasurementStruct(order, measurement)
    }
  }
}
