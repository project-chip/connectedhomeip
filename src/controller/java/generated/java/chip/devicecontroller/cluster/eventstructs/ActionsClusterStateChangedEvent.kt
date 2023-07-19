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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ActionsClusterStateChangedEvent (
    val actionID: Int,
    val invokeID: Long,
    val newState: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ActionsClusterStateChangedEvent {\n")
    builder.append("\tactionID : $actionID\n")
    builder.append("\tinvokeID : $invokeID\n")
    builder.append("\tnewState : $newState\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), actionID)
    tlvWriter.put(ContextSpecificTag(1), invokeID)
    tlvWriter.put(ContextSpecificTag(2), newState)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterStateChangedEvent {
      tlvReader.enterStructure(tag)
      val actionID: Int = tlvReader.getInt(ContextSpecificTag(0))
      val invokeID: Long = tlvReader.getLong(ContextSpecificTag(1))
      val newState: Int = tlvReader.getInt(ContextSpecificTag(2))
      
      tlvReader.exitContainer()

      return ActionsClusterStateChangedEvent(actionID, invokeID, newState)
    }
  }
}
