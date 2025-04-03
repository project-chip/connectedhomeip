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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ElectricalEnergyMeasurementClusterEnergyMeasurementStruct(
  val energy: Long,
  val startTimestamp: Optional<UInt>,
  val endTimestamp: Optional<UInt>,
  val startSystime: Optional<ULong>,
  val endSystime: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("ElectricalEnergyMeasurementClusterEnergyMeasurementStruct {\n")
    append("\tenergy : $energy\n")
    append("\tstartTimestamp : $startTimestamp\n")
    append("\tendTimestamp : $endTimestamp\n")
    append("\tstartSystime : $startSystime\n")
    append("\tendSystime : $endSystime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ENERGY), energy)
      if (startTimestamp.isPresent) {
        val optstartTimestamp = startTimestamp.get()
        put(ContextSpecificTag(TAG_START_TIMESTAMP), optstartTimestamp)
      }
      if (endTimestamp.isPresent) {
        val optendTimestamp = endTimestamp.get()
        put(ContextSpecificTag(TAG_END_TIMESTAMP), optendTimestamp)
      }
      if (startSystime.isPresent) {
        val optstartSystime = startSystime.get()
        put(ContextSpecificTag(TAG_START_SYSTIME), optstartSystime)
      }
      if (endSystime.isPresent) {
        val optendSystime = endSystime.get()
        put(ContextSpecificTag(TAG_END_SYSTIME), optendSystime)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ENERGY = 0
    private const val TAG_START_TIMESTAMP = 1
    private const val TAG_END_TIMESTAMP = 2
    private const val TAG_START_SYSTIME = 3
    private const val TAG_END_SYSTIME = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalEnergyMeasurementClusterEnergyMeasurementStruct {
      tlvReader.enterStructure(tlvTag)
      val energy = tlvReader.getLong(ContextSpecificTag(TAG_ENERGY))
      val startTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_TIMESTAMP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_START_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val endTimestamp =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_END_TIMESTAMP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_END_TIMESTAMP)))
        } else {
          Optional.empty()
        }
      val startSystime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_START_SYSTIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_START_SYSTIME)))
        } else {
          Optional.empty()
        }
      val endSystime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_END_SYSTIME))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_END_SYSTIME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterEnergyMeasurementStruct(
        energy,
        startTimestamp,
        endTimestamp,
        startSystime,
        endSystime,
      )
    }
  }
}
