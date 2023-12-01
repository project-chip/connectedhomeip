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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ElectricalEnergyMeasurementClusterCummulativeEnergyMeasuredEvent(
  val energyImported:
    chip.devicecontroller.cluster.structs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct,
  val energyExported:
    chip.devicecontroller.cluster.structs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct
) {
  override fun toString(): String = buildString {
    append("ElectricalEnergyMeasurementClusterCummulativeEnergyMeasuredEvent {\n")
    append("\tenergyImported : $energyImported\n")
    append("\tenergyExported : $energyExported\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      energyImported.toTlv(ContextSpecificTag(TAG_ENERGY_IMPORTED), this)
      energyExported.toTlv(ContextSpecificTag(TAG_ENERGY_EXPORTED), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ENERGY_IMPORTED = 0
    private const val TAG_ENERGY_EXPORTED = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ElectricalEnergyMeasurementClusterCummulativeEnergyMeasuredEvent {
      tlvReader.enterStructure(tlvTag)
      val energyImported =
        chip.devicecontroller.cluster.structs
          .ElectricalEnergyMeasurementClusterEnergyMeasurementStruct
          .fromTlv(ContextSpecificTag(TAG_ENERGY_IMPORTED), tlvReader)
      val energyExported =
        chip.devicecontroller.cluster.structs
          .ElectricalEnergyMeasurementClusterEnergyMeasurementStruct
          .fromTlv(ContextSpecificTag(TAG_ENERGY_EXPORTED), tlvReader)

      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterCummulativeEnergyMeasuredEvent(
        energyImported,
        energyExported
      )
    }
  }
}
