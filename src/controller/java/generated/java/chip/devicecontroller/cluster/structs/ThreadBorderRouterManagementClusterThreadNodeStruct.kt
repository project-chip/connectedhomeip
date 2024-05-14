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

class ThreadBorderRouterManagementClusterThreadNodeStruct(
  val extAddress: ULong,
  val rloc16: UInt,
  val IPv6s: List<ByteArray>,
  val routingRole: UInt,
  val routeTable: List<ThreadBorderRouterManagementClusterRouteTableStruct>,
  val childTable: List<ThreadBorderRouterManagementClusterChildTableStruct>
) {
  override fun toString(): String = buildString {
    append("ThreadBorderRouterManagementClusterThreadNodeStruct {\n")
    append("\textAddress : $extAddress\n")
    append("\trloc16 : $rloc16\n")
    append("\tIPv6s : $IPv6s\n")
    append("\troutingRole : $routingRole\n")
    append("\trouteTable : $routeTable\n")
    append("\tchildTable : $childTable\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EXT_ADDRESS), extAddress)
      put(ContextSpecificTag(TAG_RLOC16), rloc16)
      startArray(ContextSpecificTag(TAG_I_PV6S))
      for (item in IPv6s.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      put(ContextSpecificTag(TAG_ROUTING_ROLE), routingRole)
      startArray(ContextSpecificTag(TAG_ROUTE_TABLE))
      for (item in routeTable.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_CHILD_TABLE))
      for (item in childTable.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_EXT_ADDRESS = 0
    private const val TAG_RLOC16 = 1
    private const val TAG_I_PV6S = 2
    private const val TAG_ROUTING_ROLE = 3
    private const val TAG_ROUTE_TABLE = 4
    private const val TAG_CHILD_TABLE = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ThreadBorderRouterManagementClusterThreadNodeStruct {
      tlvReader.enterStructure(tlvTag)
      val extAddress = tlvReader.getULong(ContextSpecificTag(TAG_EXT_ADDRESS))
      val rloc16 = tlvReader.getUInt(ContextSpecificTag(TAG_RLOC16))
      val IPv6s =
        buildList<ByteArray> {
          tlvReader.enterArray(ContextSpecificTag(TAG_I_PV6S))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val routingRole = tlvReader.getUInt(ContextSpecificTag(TAG_ROUTING_ROLE))
      val routeTable =
        buildList<ThreadBorderRouterManagementClusterRouteTableStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ROUTE_TABLE))
          while (!tlvReader.isEndOfContainer()) {
            add(
              ThreadBorderRouterManagementClusterRouteTableStruct.fromTlv(AnonymousTag, tlvReader)
            )
          }
          tlvReader.exitContainer()
        }
      val childTable =
        buildList<ThreadBorderRouterManagementClusterChildTableStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_CHILD_TABLE))
          while (!tlvReader.isEndOfContainer()) {
            add(
              ThreadBorderRouterManagementClusterChildTableStruct.fromTlv(AnonymousTag, tlvReader)
            )
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ThreadBorderRouterManagementClusterThreadNodeStruct(
        extAddress,
        rloc16,
        IPv6s,
        routingRole,
        routeTable,
        childTable
      )
    }
  }
}
