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

class BasicInformationClusterCapabilityMinimaStruct (
    val caseSessionsPerFabric: Int,
    val subscriptionsPerFabric: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("BasicInformationClusterCapabilityMinimaStruct {\n")
    builder.append("\tcaseSessionsPerFabric : $caseSessionsPerFabric\n")
    builder.append("\tsubscriptionsPerFabric : $subscriptionsPerFabric\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), caseSessionsPerFabric)
    tlvWriter.put(ContextSpecificTag(1), subscriptionsPerFabric)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : BasicInformationClusterCapabilityMinimaStruct {
      tlvReader.enterStructure(tag)
      val caseSessionsPerFabric: Int = tlvReader.getInt(ContextSpecificTag(0))
      val subscriptionsPerFabric: Int = tlvReader.getInt(ContextSpecificTag(1))
      
      tlvReader.exitContainer()

      return BasicInformationClusterCapabilityMinimaStruct(caseSessionsPerFabric, subscriptionsPerFabric)
    }
  }
}
