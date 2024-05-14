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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThreadBorderRouterManagementClusterNeiborTableStruct(
  val extAddress: ULong,
  val age: UInt,
  val rloc16: UShort,
  val averageRssi: Byte?,
  val lastRssi: Byte?,
  val routingRole: UByte
) {
  override fun toString(): String = buildString {
    append("ThreadBorderRouterManagementClusterNeiborTableStruct {\n")
    append("\textAddress : $extAddress\n")
    append("\tage : $age\n")
    append("\trloc16 : $rloc16\n")
    append("\taverageRssi : $averageRssi\n")
    append("\tlastRssi : $lastRssi\n")
    append("\troutingRole : $routingRole\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EXT_ADDRESS), extAddress)
      put(ContextSpecificTag(TAG_AGE), age)
      put(ContextSpecificTag(TAG_RLOC16), rloc16)
      if (averageRssi != null) {
        put(ContextSpecificTag(TAG_AVERAGE_RSSI), averageRssi)
      } else {
        putNull(ContextSpecificTag(TAG_AVERAGE_RSSI))
      }
      if (lastRssi != null) {
        put(ContextSpecificTag(TAG_LAST_RSSI), lastRssi)
      } else {
        putNull(ContextSpecificTag(TAG_LAST_RSSI))
      }
      put(ContextSpecificTag(TAG_ROUTING_ROLE), routingRole)
      endStructure()
    }
  }

  companion object {
    private const val TAG_EXT_ADDRESS = 0
    private const val TAG_AGE = 1
    private const val TAG_RLOC16 = 2
    private const val TAG_AVERAGE_RSSI = 3
    private const val TAG_LAST_RSSI = 4
    private const val TAG_ROUTING_ROLE = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ThreadBorderRouterManagementClusterNeiborTableStruct {
      tlvReader.enterStructure(tlvTag)
      val extAddress = tlvReader.getULong(ContextSpecificTag(TAG_EXT_ADDRESS))
      val age = tlvReader.getUInt(ContextSpecificTag(TAG_AGE))
      val rloc16 = tlvReader.getUShort(ContextSpecificTag(TAG_RLOC16))
      val averageRssi =
        if (!tlvReader.isNull()) {
          tlvReader.getByte(ContextSpecificTag(TAG_AVERAGE_RSSI))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_AVERAGE_RSSI))
          null
        }
      val lastRssi =
        if (!tlvReader.isNull()) {
          tlvReader.getByte(ContextSpecificTag(TAG_LAST_RSSI))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LAST_RSSI))
          null
        }
      val routingRole = tlvReader.getUByte(ContextSpecificTag(TAG_ROUTING_ROLE))

      tlvReader.exitContainer()

      return ThreadBorderRouterManagementClusterNeiborTableStruct(
        extAddress,
        age,
        rloc16,
        averageRssi,
        lastRssi,
        routingRole
      )
    }
  }
}
