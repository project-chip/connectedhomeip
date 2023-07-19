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

class OperationalCredentialsClusterFabricDescriptorStruct (
    val rootPublicKey: ByteArray,
    val vendorID: Int,
    val fabricID: Long,
    val nodeID: Long,
    val label: String,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("OperationalCredentialsClusterFabricDescriptorStruct {\n")
    builder.append("\trootPublicKey : $rootPublicKey\n")
    builder.append("\tvendorID : $vendorID\n")
    builder.append("\tfabricID : $fabricID\n")
    builder.append("\tnodeID : $nodeID\n")
    builder.append("\tlabel : $label\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(1), rootPublicKey)
    tlvWriter.put(ContextSpecificTag(2), vendorID)
    tlvWriter.put(ContextSpecificTag(3), fabricID)
    tlvWriter.put(ContextSpecificTag(4), nodeID)
    tlvWriter.put(ContextSpecificTag(5), label)
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalCredentialsClusterFabricDescriptorStruct {
      tlvReader.enterStructure(tag)
      val rootPublicKey: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
      val vendorID: Int = tlvReader.getInt(ContextSpecificTag(2))
      val fabricID: Long = tlvReader.getLong(ContextSpecificTag(3))
      val nodeID: Long = tlvReader.getLong(ContextSpecificTag(4))
      val label: String = tlvReader.getString(ContextSpecificTag(5))
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return OperationalCredentialsClusterFabricDescriptorStruct(rootPublicKey, vendorID, fabricID, nodeID, label, fabricIndex)
    }
  }
}
