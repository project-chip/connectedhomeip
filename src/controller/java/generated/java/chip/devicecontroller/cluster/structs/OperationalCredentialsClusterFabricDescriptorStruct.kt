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
    tlvWriter.put(ContextSpecificTag(TAG_ROOT_PUBLIC_KEY), rootPublicKey)
    tlvWriter.put(ContextSpecificTag(TAG_VENDOR_I_D), vendorID)
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_I_D), fabricID)
    tlvWriter.put(ContextSpecificTag(TAG_NODE_I_D), nodeID)
    tlvWriter.put(ContextSpecificTag(TAG_LABEL), label)
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_ROOT_PUBLIC_KEY = 1
    private const val TAG_VENDOR_I_D = 2
    private const val TAG_FABRIC_I_D = 3
    private const val TAG_NODE_I_D = 4
    private const val TAG_LABEL = 5
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalCredentialsClusterFabricDescriptorStruct {
      tlvReader.enterStructure(tag)
      val rootPublicKey: ByteArray = tlvReader.getByteArray(ContextSpecificTag(TAG_ROOT_PUBLIC_KEY))
      val vendorID: Int = tlvReader.getInt(ContextSpecificTag(TAG_VENDOR_I_D))
      val fabricID: Long = tlvReader.getLong(ContextSpecificTag(TAG_FABRIC_I_D))
      val nodeID: Long = tlvReader.getLong(ContextSpecificTag(TAG_NODE_I_D))
      val label: String = tlvReader.getString(ContextSpecificTag(TAG_LABEL))
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return OperationalCredentialsClusterFabricDescriptorStruct(rootPublicKey, vendorID, fabricID, nodeID, label, fabricIndex)
    }
  }
}
