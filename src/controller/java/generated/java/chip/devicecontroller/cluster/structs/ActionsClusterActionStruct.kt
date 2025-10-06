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

class ActionsClusterActionStruct(
  val actionID: UInt,
  val name: String,
  val type: UInt,
  val endpointListID: UInt,
  val supportedCommands: UInt,
  val state: UInt,
) {
  override fun toString(): String = buildString {
    append("ActionsClusterActionStruct {\n")
    append("\tactionID : $actionID\n")
    append("\tname : $name\n")
    append("\ttype : $type\n")
    append("\tendpointListID : $endpointListID\n")
    append("\tsupportedCommands : $supportedCommands\n")
    append("\tstate : $state\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ACTION_ID), actionID)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_TYPE), type)
      put(ContextSpecificTag(TAG_ENDPOINT_LIST_ID), endpointListID)
      put(ContextSpecificTag(TAG_SUPPORTED_COMMANDS), supportedCommands)
      put(ContextSpecificTag(TAG_STATE), state)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ACTION_ID = 0
    private const val TAG_NAME = 1
    private const val TAG_TYPE = 2
    private const val TAG_ENDPOINT_LIST_ID = 3
    private const val TAG_SUPPORTED_COMMANDS = 4
    private const val TAG_STATE = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ActionsClusterActionStruct {
      tlvReader.enterStructure(tlvTag)
      val actionID = tlvReader.getUInt(ContextSpecificTag(TAG_ACTION_ID))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val type = tlvReader.getUInt(ContextSpecificTag(TAG_TYPE))
      val endpointListID = tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT_LIST_ID))
      val supportedCommands = tlvReader.getUInt(ContextSpecificTag(TAG_SUPPORTED_COMMANDS))
      val state = tlvReader.getUInt(ContextSpecificTag(TAG_STATE))

      tlvReader.exitContainer()

      return ActionsClusterActionStruct(
        actionID,
        name,
        type,
        endpointListID,
        supportedCommands,
        state,
      )
    }
  }
}
