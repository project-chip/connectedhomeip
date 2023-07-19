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
    tlvWriter.put(ContextSpecificTag(0), extAddress)
    tlvWriter.put(ContextSpecificTag(1), rloc16)
    tlvWriter.put(ContextSpecificTag(2), routerId)
    tlvWriter.put(ContextSpecificTag(3), nextHop)
    tlvWriter.put(ContextSpecificTag(4), pathCost)
    tlvWriter.put(ContextSpecificTag(5), LQIIn)
    tlvWriter.put(ContextSpecificTag(6), LQIOut)
    tlvWriter.put(ContextSpecificTag(7), age)
    tlvWriter.put(ContextSpecificTag(8), allocated)
    tlvWriter.put(ContextSpecificTag(9), linkEstablished)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterRouteTableStruct {
      tlvReader.enterStructure(tag)
      val extAddress: Long = tlvReader.getLong(ContextSpecificTag(0))
      val rloc16: Int = tlvReader.getInt(ContextSpecificTag(1))
      val routerId: Int = tlvReader.getInt(ContextSpecificTag(2))
      val nextHop: Int = tlvReader.getInt(ContextSpecificTag(3))
      val pathCost: Int = tlvReader.getInt(ContextSpecificTag(4))
      val LQIIn: Int = tlvReader.getInt(ContextSpecificTag(5))
      val LQIOut: Int = tlvReader.getInt(ContextSpecificTag(6))
      val age: Int = tlvReader.getInt(ContextSpecificTag(7))
      val allocated: Boolean = tlvReader.getBoolean(ContextSpecificTag(8))
      val linkEstablished: Boolean = tlvReader.getBoolean(ContextSpecificTag(9))
      
      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterRouteTableStruct(extAddress, rloc16, routerId, nextHop, pathCost, LQIIn, LQIOut, age, allocated, linkEstablished)
    }
  }
}
