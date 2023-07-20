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

class ActionsClusterEndpointListStruct (
    val endpointListID: Int,
    val name: String,
    val type: Int,
    val endpoints: List<Int>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ActionsClusterEndpointListStruct {\n")
    builder.append("\tendpointListID : $endpointListID\n")
    builder.append("\tname : $name\n")
    builder.append("\ttype : $type\n")
    builder.append("\tendpoints : $endpoints\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_ENDPOINT_LIST_I_D), endpointListID)
    tlvWriter.put(ContextSpecificTag(TAG_NAME), name)
    tlvWriter.put(ContextSpecificTag(TAG_TYPE), type)
    tlvWriter.startList(ContextSpecificTag(TAG_ENDPOINTS))
      val iterendpoints = endpoints.iterator()
      while(iterendpoints.hasNext()) {
        val next = iterendpoints.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_ENDPOINT_LIST_I_D = 0
    private const val TAG_NAME = 1
    private const val TAG_TYPE = 2
    private const val TAG_ENDPOINTS = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ActionsClusterEndpointListStruct {
      tlvReader.enterStructure(tag)
      val endpointListID: Int = tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT_LIST_I_D))
      val name: String = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val type: Int = tlvReader.getInt(ContextSpecificTag(TAG_TYPE))
      val endpoints: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_ENDPOINTS))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return ActionsClusterEndpointListStruct(endpointListID, name, type, endpoints)
    }
  }
}
