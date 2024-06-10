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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class AccessControlClusterAccessControlExtensionChangedEvent (
    val adminNodeID: ULong?,
    val adminPasscodeID: UInt?,
    val changeType: UInt,
    val latestValue: chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlExtensionStruct?,
    val fabricIndex: UInt) {
  override fun toString(): String  = buildString {
    append("AccessControlClusterAccessControlExtensionChangedEvent {\n")
    append("\tadminNodeID : $adminNodeID\n")
    append("\tadminPasscodeID : $adminPasscodeID\n")
    append("\tchangeType : $changeType\n")
    append("\tlatestValue : $latestValue\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (adminNodeID != null) {
      put(ContextSpecificTag(TAG_ADMIN_NODE_I_D), adminNodeID)
    } else {
      putNull(ContextSpecificTag(TAG_ADMIN_NODE_I_D))
    }
      if (adminPasscodeID != null) {
      put(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D), adminPasscodeID)
    } else {
      putNull(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D))
    }
      put(ContextSpecificTag(TAG_CHANGE_TYPE), changeType)
      if (latestValue != null) {
      latestValue.toTlv(ContextSpecificTag(TAG_LATEST_VALUE), this)
    } else {
      putNull(ContextSpecificTag(TAG_LATEST_VALUE))
    }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ADMIN_NODE_I_D = 1
    private const val TAG_ADMIN_PASSCODE_I_D = 2
    private const val TAG_CHANGE_TYPE = 3
    private const val TAG_LATEST_VALUE = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlExtensionChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val adminNodeID = if (!tlvReader.isNull()) {
      tlvReader.getULong(ContextSpecificTag(TAG_ADMIN_NODE_I_D))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_ADMIN_NODE_I_D))
      null
    }
      val adminPasscodeID = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_ADMIN_PASSCODE_I_D))
      null
    }
      val changeType = tlvReader.getUInt(ContextSpecificTag(TAG_CHANGE_TYPE))
      val latestValue = if (!tlvReader.isNull()) {
      chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlExtensionStruct.fromTlv(ContextSpecificTag(TAG_LATEST_VALUE), tlvReader)
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_LATEST_VALUE))
      null
    }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return AccessControlClusterAccessControlExtensionChangedEvent(adminNodeID, adminPasscodeID, changeType, latestValue, fabricIndex)
    }
  }
}
