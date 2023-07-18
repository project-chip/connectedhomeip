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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class AccessControlClusterAccessControlEntryChangedEvent (
    val adminNodeID: Long?,
    val adminPasscodeID: Int?,
    val changeType: Int,
    val latestValue: chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlEntryStruct?,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("AccessControlClusterAccessControlEntryChangedEvent {\n")
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
    if (adminNodeID == null) { tlvWriter.putNull(ContextSpecificTag(1)) }
    else {
      tlvWriter.put(ContextSpecificTag(1), adminNodeID)
    }
    if (adminPasscodeID == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), adminPasscodeID)
    }
    tlvWriter.put(ContextSpecificTag(3), changeType)
    if (latestValue == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      latestValue.toTlv(ContextSpecificTag(4), tlvWriter)
    }
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlEntryChangedEvent {
      tlvReader.enterStructure(tag)
      val adminNodeID: Long? = try {
      tlvReader.getLong(ContextSpecificTag(1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(1))
      null
    }
      val adminPasscodeID: Int? = try {
      tlvReader.getInt(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      val changeType: Int = tlvReader.getInt(ContextSpecificTag(3))
      val latestValue: chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlEntryStruct? = try {
      chip.devicecontroller.cluster.structs.AccessControlClusterAccessControlEntryStruct.fromTlv(ContextSpecificTag(4), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return AccessControlClusterAccessControlEntryChangedEvent(adminNodeID, adminPasscodeID, changeType, latestValue, fabricIndex)
    }
  }
}
