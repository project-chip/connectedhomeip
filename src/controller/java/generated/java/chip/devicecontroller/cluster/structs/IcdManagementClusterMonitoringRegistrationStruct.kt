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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class IcdManagementClusterMonitoringRegistrationStruct (
    val checkInNodeID: ULong,
    val monitoredSubject: ULong,
    val fabricIndex: UInt) {
  override fun toString(): String  = buildString {
    append("IcdManagementClusterMonitoringRegistrationStruct {\n")
    append("\tcheckInNodeID : $checkInNodeID\n")
    append("\tmonitoredSubject : $monitoredSubject\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CHECK_IN_NODE_I_D), checkInNodeID)
      put(ContextSpecificTag(TAG_MONITORED_SUBJECT), monitoredSubject)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CHECK_IN_NODE_I_D = 1
    private const val TAG_MONITORED_SUBJECT = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : IcdManagementClusterMonitoringRegistrationStruct {
      tlvReader.enterStructure(tlvTag)
      val checkInNodeID = tlvReader.getULong(ContextSpecificTag(TAG_CHECK_IN_NODE_I_D))
      val monitoredSubject = tlvReader.getULong(ContextSpecificTag(TAG_MONITORED_SUBJECT))
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return IcdManagementClusterMonitoringRegistrationStruct(checkInNodeID, monitoredSubject, fabricIndex)
    }
  }
}
