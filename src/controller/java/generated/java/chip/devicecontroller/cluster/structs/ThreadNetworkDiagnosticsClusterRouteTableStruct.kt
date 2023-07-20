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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ThreadNetworkDiagnosticsClusterRouteTableStruct (
    val extAddress: Long,
    val rloc16: Int,
    val routerId: Int,
    val nextHop: Int,
    val pathCost: Int,
    val LQIIn: Int,
    val LQIOut: Int,
    val age: Int,
    val allocated: Boolean,
    val linkEstablished: Boolean) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ThreadNetworkDiagnosticsClusterRouteTableStruct {\n")
    builder.append("\textAddress : $extAddress\n")
    builder.append("\trloc16 : $rloc16\n")
    builder.append("\trouterId : $routerId\n")
    builder.append("\tnextHop : $nextHop\n")
    builder.append("\tpathCost : $pathCost\n")
    builder.append("\tLQIIn : $LQIIn\n")
    builder.append("\tLQIOut : $LQIOut\n")
    builder.append("\tage : $age\n")
    builder.append("\tallocated : $allocated\n")
    builder.append("\tlinkEstablished : $linkEstablished\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_EXT_ADDRESS), extAddress)
    tlvWriter.put(ContextSpecificTag(TAG_RLOC16), rloc16)
    tlvWriter.put(ContextSpecificTag(TAG_ROUTER_ID), routerId)
    tlvWriter.put(ContextSpecificTag(TAG_NEXT_HOP), nextHop)
    tlvWriter.put(ContextSpecificTag(TAG_PATH_COST), pathCost)
    tlvWriter.put(ContextSpecificTag(TAG_L_Q_I_IN), LQIIn)
    tlvWriter.put(ContextSpecificTag(TAG_L_Q_I_OUT), LQIOut)
    tlvWriter.put(ContextSpecificTag(TAG_AGE), age)
    tlvWriter.put(ContextSpecificTag(TAG_ALLOCATED), allocated)
    tlvWriter.put(ContextSpecificTag(TAG_LINK_ESTABLISHED), linkEstablished)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_EXT_ADDRESS = 0
    private const val TAG_RLOC16 = 1
    private const val TAG_ROUTER_ID = 2
    private const val TAG_NEXT_HOP = 3
    private const val TAG_PATH_COST = 4
    private const val TAG_L_Q_I_IN = 5
    private const val TAG_L_Q_I_OUT = 6
    private const val TAG_AGE = 7
    private const val TAG_ALLOCATED = 8
    private const val TAG_LINK_ESTABLISHED = 9

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterRouteTableStruct {
      tlvReader.enterStructure(tag)
      val extAddress: Long = tlvReader.getLong(ContextSpecificTag(TAG_EXT_ADDRESS))
      val rloc16: Int = tlvReader.getInt(ContextSpecificTag(TAG_RLOC16))
      val routerId: Int = tlvReader.getInt(ContextSpecificTag(TAG_ROUTER_ID))
      val nextHop: Int = tlvReader.getInt(ContextSpecificTag(TAG_NEXT_HOP))
      val pathCost: Int = tlvReader.getInt(ContextSpecificTag(TAG_PATH_COST))
      val LQIIn: Int = tlvReader.getInt(ContextSpecificTag(TAG_L_Q_I_IN))
      val LQIOut: Int = tlvReader.getInt(ContextSpecificTag(TAG_L_Q_I_OUT))
      val age: Int = tlvReader.getInt(ContextSpecificTag(TAG_AGE))
      val allocated: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_ALLOCATED))
      val linkEstablished: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_LINK_ESTABLISHED))
      
      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterRouteTableStruct(extAddress, rloc16, routerId, nextHop, pathCost, LQIIn, LQIOut, age, allocated, linkEstablished)
    }
  }
}
