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

class ActionsClusterActionStruct (
    val actionID: Int,
    val name: String,
    val type: Int,
    val endpointListID: Int,
    val supportedCommands: Int,
    val state: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ActionsClusterActionStruct {\n")
    builder.append("\tactionID : $actionID\n")
    builder.append("\tname : $name\n")
    builder.append("\ttype : $type\n")
    builder.append("\tendpointListID : $endpointListID\n")
    builder.append("\tsupportedCommands : $supportedCommands\n")
    builder.append("\tstate : $state\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), actionID)
    tlvWriter.put(ContextSpecificTag(1), name)
    tlvWriter.put(ContextSpecificTag(2), type)
    tlvWriter.put(ContextSpecificTag(3), endpointListID)
    tlvWriter.put(ContextSpecificTag(4), supportedCommands)
    tlvWriter.put(ContextSpecificTag(5), state)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterActionStruct {
      tlvReader.enterStructure(tag)
      val actionID: Int = tlvReader.getInt(ContextSpecificTag(0))
      val name: String = tlvReader.getString(ContextSpecificTag(1))
      val type: Int = tlvReader.getInt(ContextSpecificTag(2))
      val endpointListID: Int = tlvReader.getInt(ContextSpecificTag(3))
      val supportedCommands: Int = tlvReader.getInt(ContextSpecificTag(4))
      val state: Int = tlvReader.getInt(ContextSpecificTag(5))
      
      tlvReader.exitContainer()

      return ActionsClusterActionStruct(actionID, name, type, endpointListID, supportedCommands, state)
    }
  }
}
