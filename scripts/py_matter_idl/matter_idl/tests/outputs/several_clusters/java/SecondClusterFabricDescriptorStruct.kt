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

class SecondClusterFabricDescriptorStruct (
    val rootPublicKey: ByteArray,
    val vendorID: UInt,
    val fabricID: ULong,
    val nodeID: ULong,
    val label: String,
    val fabricIndex: UInt) {
  override fun toString(): String  = buildString {
    append("SecondClusterFabricDescriptorStruct {\n")
    append("\trootPublicKey : $rootPublicKey\n")
    append("\tvendorID : $vendorID\n")
    append("\tfabricID : $fabricID\n")
    append("\tnodeID : $nodeID\n")
    append("\tlabel : $label\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ROOT_PUBLIC_KEY), rootPublicKey)
      put(ContextSpecificTag(TAG_VENDOR_I_D), vendorID)
      put(ContextSpecificTag(TAG_FABRIC_I_D), fabricID)
      put(ContextSpecificTag(TAG_NODE_I_D), nodeID)
      put(ContextSpecificTag(TAG_LABEL), label)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROOT_PUBLIC_KEY = 1
    private const val TAG_VENDOR_I_D = 2
    private const val TAG_FABRIC_I_D = 3
    private const val TAG_NODE_I_D = 4
    private const val TAG_LABEL = 5
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : SecondClusterFabricDescriptorStruct {
      tlvReader.enterStructure(tlvTag)
      val rootPublicKey = tlvReader.getByteArray(ContextSpecificTag(TAG_ROOT_PUBLIC_KEY))
      val vendorID = tlvReader.getUInt(ContextSpecificTag(TAG_VENDOR_I_D))
      val fabricID = tlvReader.getULong(ContextSpecificTag(TAG_FABRIC_I_D))
      val nodeID = tlvReader.getULong(ContextSpecificTag(TAG_NODE_I_D))
      val label = tlvReader.getString(ContextSpecificTag(TAG_LABEL))
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return SecondClusterFabricDescriptorStruct(rootPublicKey, vendorID, fabricID, nodeID, label, fabricIndex)
    }
  }
}
