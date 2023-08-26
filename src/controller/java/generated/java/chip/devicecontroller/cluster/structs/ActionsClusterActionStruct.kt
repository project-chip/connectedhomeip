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
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ActionsClusterActionStruct(
  val actionID: Int,
  val name: String,
  val type: Int,
  val endpointListID: Int,
  val supportedCommands: Int,
  val state: Int
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

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ACTION_I_D), actionID)
      put(ContextSpecificTag(TAG_NAME), name)
      put(ContextSpecificTag(TAG_TYPE), type)
      put(ContextSpecificTag(TAG_ENDPOINT_LIST_I_D), endpointListID)
      put(ContextSpecificTag(TAG_SUPPORTED_COMMANDS), supportedCommands)
      put(ContextSpecificTag(TAG_STATE), state)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ACTION_I_D = 0
    private const val TAG_NAME = 1
    private const val TAG_TYPE = 2
    private const val TAG_ENDPOINT_LIST_I_D = 3
    private const val TAG_SUPPORTED_COMMANDS = 4
    private const val TAG_STATE = 5

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ActionsClusterActionStruct {
      tlvReader.enterStructure(tag)
      val actionID = tlvReader.getInt(ContextSpecificTag(TAG_ACTION_I_D))
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val type = tlvReader.getInt(ContextSpecificTag(TAG_TYPE))
      val endpointListID = tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT_LIST_I_D))
      val supportedCommands = tlvReader.getInt(ContextSpecificTag(TAG_SUPPORTED_COMMANDS))
      val state = tlvReader.getInt(ContextSpecificTag(TAG_STATE))

      tlvReader.exitContainer()

      return ActionsClusterActionStruct(
        actionID,
        name,
        type,
        endpointListID,
        supportedCommands,
        state
      )
    }
  }
}
