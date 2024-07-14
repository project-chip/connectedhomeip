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

class ServiceAreaClusterProgressStruct(
  val locationID: UInt,
  val status: UByte,
  val totalOperationalTime: Optional<UInt>?,
  val estimatedTime: Optional<UInt>?,
) {
  override fun toString(): String = buildString {
    append("ServiceAreaClusterProgressStruct {\n")
    append("\tlocationID : $locationID\n")
    append("\tstatus : $status\n")
    append("\ttotalOperationalTime : $totalOperationalTime\n")
    append("\testimatedTime : $estimatedTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LOCATION_I_D), locationID)
      put(ContextSpecificTag(TAG_STATUS), status)
      if (totalOperationalTime != null) {
        if (totalOperationalTime.isPresent) {
          val opttotalOperationalTime = totalOperationalTime.get()
          put(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME), opttotalOperationalTime)
        }
      } else {
        putNull(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME))
      }
      if (estimatedTime != null) {
        if (estimatedTime.isPresent) {
          val optestimatedTime = estimatedTime.get()
          put(ContextSpecificTag(TAG_ESTIMATED_TIME), optestimatedTime)
        }
      } else {
        putNull(ContextSpecificTag(TAG_ESTIMATED_TIME))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LOCATION_I_D = 0
    private const val TAG_STATUS = 1
    private const val TAG_TOTAL_OPERATIONAL_TIME = 2
    private const val TAG_ESTIMATED_TIME = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ServiceAreaClusterProgressStruct {
      tlvReader.enterStructure(tlvTag)
      val locationID = tlvReader.getUInt(ContextSpecificTag(TAG_LOCATION_I_D))
      val status = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS))
      val totalOperationalTime =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME))
          null
        }
      val estimatedTime =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_ESTIMATED_TIME))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ESTIMATED_TIME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ESTIMATED_TIME))
          null
        }

      tlvReader.exitContainer()

      return ServiceAreaClusterProgressStruct(
        locationID,
        status,
        totalOperationalTime,
        estimatedTime,
      )
    }
  }
}
