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

class AccessControlClusterAccessControlExtensionChangedEvent (
    val adminNodeID: Long?,
    val adminPasscodeID: Int?,
    val changeType: Int,
    val latestValue: chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlExtensionStruct?,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("AccessControlClusterAccessControlExtensionChangedEvent {\n")
    builder.append("\tadminNodeID : $adminNodeID\n")
    builder.append("\tadminPasscodeID : $adminPasscodeID\n")
    builder.append("\tchangeType : $changeType\n")
    builder.append("\tlatestValue : $latestValue\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    if (adminNodeID == null) { tlvWriter.putNull(ContextSpecificTag(TAG_ADMIN_NODE_I_D)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_ADMIN_NODE_I_D), adminNodeID)
    }
    if (adminPasscodeID == null) { tlvWriter.putNull(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D), adminPasscodeID)
    }
    tlvWriter.put(ContextSpecificTag(TAG_CHANGE_TYPE), changeType)
    if (latestValue == null) { tlvWriter.putNull(ContextSpecificTag(TAG_LATEST_VALUE)) }
    else {
      latestValue.toTlv(ContextSpecificTag(TAG_LATEST_VALUE), tlvWriter)
    }
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_ADMIN_NODE_I_D = 1
    private const val TAG_ADMIN_PASSCODE_I_D = 2
    private const val TAG_CHANGE_TYPE = 3
    private const val TAG_LATEST_VALUE = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlExtensionChangedEvent {
      tlvReader.enterStructure(tag)
      val adminNodeID: Long? = try {
      tlvReader.getLong(ContextSpecificTag(TAG_ADMIN_NODE_I_D))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_ADMIN_NODE_I_D))
      null
    }
      val adminPasscodeID: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D))
      null
    }
      val changeType: Int = tlvReader.getInt(ContextSpecificTag(TAG_CHANGE_TYPE))
      val latestValue: chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlExtensionStruct? = try {
      chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlExtensionStruct.fromTlv(ContextSpecificTag(TAG_LATEST_VALUE), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_LATEST_VALUE))
      null
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return AccessControlClusterAccessControlExtensionChangedEvent(adminNodeID, adminPasscodeID, changeType, latestValue, fabricIndex)
    }
  }
}
