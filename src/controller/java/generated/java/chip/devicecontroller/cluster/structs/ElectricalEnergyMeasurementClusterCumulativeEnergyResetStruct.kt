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

class ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct(
  val importedResetTimestamp: Optional<ULong>?,
  val exportedResetTimestamp: Optional<ULong>?,
  val importedResetSystime: Optional<ULong>?,
  val exportedResetSystime: Optional<ULong>?,
) {
  override fun toString(): String = buildString {
    append("ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct {\n")
    append("\timportedResetTimestamp : $importedResetTimestamp\n")
    append("\texportedResetTimestamp : $exportedResetTimestamp\n")
    append("\timportedResetSystime : $importedResetSystime\n")
    append("\texportedResetSystime : $exportedResetSystime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (importedResetTimestamp != null) {
        if (importedResetTimestamp.isPresent) {
          val optimportedResetTimestamp = importedResetTimestamp.get()
          put(ContextSpecificTag(TAG_IMPORTED_RESET_TIMESTAMP), optimportedResetTimestamp)
        }
      } else {
        putNull(ContextSpecificTag(TAG_IMPORTED_RESET_TIMESTAMP))
      }
      if (exportedResetTimestamp != null) {
        if (exportedResetTimestamp.isPresent) {
          val optexportedResetTimestamp = exportedResetTimestamp.get()
          put(ContextSpecificTag(TAG_EXPORTED_RESET_TIMESTAMP), optexportedResetTimestamp)
        }
      } else {
        putNull(ContextSpecificTag(TAG_EXPORTED_RESET_TIMESTAMP))
      }
      if (importedResetSystime != null) {
        if (importedResetSystime.isPresent) {
          val optimportedResetSystime = importedResetSystime.get()
          put(ContextSpecificTag(TAG_IMPORTED_RESET_SYSTIME), optimportedResetSystime)
        }
      } else {
        putNull(ContextSpecificTag(TAG_IMPORTED_RESET_SYSTIME))
      }
      if (exportedResetSystime != null) {
        if (exportedResetSystime.isPresent) {
          val optexportedResetSystime = exportedResetSystime.get()
          put(ContextSpecificTag(TAG_EXPORTED_RESET_SYSTIME), optexportedResetSystime)
        }
      } else {
        putNull(ContextSpecificTag(TAG_EXPORTED_RESET_SYSTIME))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_IMPORTED_RESET_TIMESTAMP = 0
    private const val TAG_EXPORTED_RESET_TIMESTAMP = 1
    private const val TAG_IMPORTED_RESET_SYSTIME = 2
    private const val TAG_EXPORTED_RESET_SYSTIME = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct {
      tlvReader.enterStructure(tlvTag)
      val importedResetTimestamp =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_IMPORTED_RESET_TIMESTAMP))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_IMPORTED_RESET_TIMESTAMP)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_IMPORTED_RESET_TIMESTAMP))
          null
        }
      val exportedResetTimestamp =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXPORTED_RESET_TIMESTAMP))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_EXPORTED_RESET_TIMESTAMP)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_EXPORTED_RESET_TIMESTAMP))
          null
        }
      val importedResetSystime =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_IMPORTED_RESET_SYSTIME))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_IMPORTED_RESET_SYSTIME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_IMPORTED_RESET_SYSTIME))
          null
        }
      val exportedResetSystime =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_EXPORTED_RESET_SYSTIME))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_EXPORTED_RESET_SYSTIME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_EXPORTED_RESET_SYSTIME))
          null
        }

      tlvReader.exitContainer()

      return ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct(
        importedResetTimestamp,
        exportedResetTimestamp,
        importedResetSystime,
        exportedResetSystime,
      )
    }
  }
}
