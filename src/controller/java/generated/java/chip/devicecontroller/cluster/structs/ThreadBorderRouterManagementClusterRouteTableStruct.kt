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

class ThreadBorderRouterManagementClusterRouteTableStruct(
  val routerId: UInt,
  val pathCost: UInt,
  val LQIIn: UInt,
  val LQIOut: UInt
) {
  override fun toString(): String = buildString {
    append("ThreadBorderRouterManagementClusterRouteTableStruct {\n")
    append("\trouterId : $routerId\n")
    append("\tpathCost : $pathCost\n")
    append("\tLQIIn : $LQIIn\n")
    append("\tLQIOut : $LQIOut\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ROUTER_ID), routerId)
      put(ContextSpecificTag(TAG_PATH_COST), pathCost)
      put(ContextSpecificTag(TAG_L_Q_I_IN), LQIIn)
      put(ContextSpecificTag(TAG_L_Q_I_OUT), LQIOut)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROUTER_ID = 0
    private const val TAG_PATH_COST = 1
    private const val TAG_L_Q_I_IN = 2
    private const val TAG_L_Q_I_OUT = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): ThreadBorderRouterManagementClusterRouteTableStruct {
      tlvReader.enterStructure(tlvTag)
      val routerId = tlvReader.getUInt(ContextSpecificTag(TAG_ROUTER_ID))
      val pathCost = tlvReader.getUInt(ContextSpecificTag(TAG_PATH_COST))
      val LQIIn = tlvReader.getUInt(ContextSpecificTag(TAG_L_Q_I_IN))
      val LQIOut = tlvReader.getUInt(ContextSpecificTag(TAG_L_Q_I_OUT))

      tlvReader.exitContainer()

      return ThreadBorderRouterManagementClusterRouteTableStruct(routerId, pathCost, LQIIn, LQIOut)
    }
  }
}
