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

class IcdManagementClusterMonitoringRegistrationStruct (
    val checkInNodeID: Long,
    val monitoredSubject: Long,
    val key: ByteArray,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("IcdManagementClusterMonitoringRegistrationStruct {\n")
    builder.append("\tcheckInNodeID : $checkInNodeID\n")
    builder.append("\tmonitoredSubject : $monitoredSubject\n")
    builder.append("\tkey : $key\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(1), checkInNodeID)
    tlvWriter.put(ContextSpecificTag(2), monitoredSubject)
    tlvWriter.put(ContextSpecificTag(3), key)
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : IcdManagementClusterMonitoringRegistrationStruct {
      tlvReader.enterStructure(tag)
      val checkInNodeID: Long = tlvReader.getLong(ContextSpecificTag(1))
      val monitoredSubject: Long = tlvReader.getLong(ContextSpecificTag(2))
      val key: ByteArray = tlvReader.getByteArray(ContextSpecificTag(3))
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return IcdManagementClusterMonitoringRegistrationStruct(checkInNodeID, monitoredSubject, key, fabricIndex)
    }
  }
}
