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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class BindingClusterTargetStruct (
    val node: Optional<Long>,
    val group: Optional<Int>,
    val endpoint: Optional<Int>,
    val cluster: Optional<Long>,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("BindingClusterTargetStruct {\n")
    builder.append("\tnode : $node\n")
    builder.append("\tgroup : $group\n")
    builder.append("\tendpoint : $endpoint\n")
    builder.append("\tcluster : $cluster\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    if (node.isPresent) {
      val opt_node = node.get()
      tlvWriter.put(ContextSpecificTag(1), opt_node)
    }
    if (group.isPresent) {
      val opt_group = group.get()
      tlvWriter.put(ContextSpecificTag(2), opt_group)
    }
    if (endpoint.isPresent) {
      val opt_endpoint = endpoint.get()
      tlvWriter.put(ContextSpecificTag(3), opt_endpoint)
    }
    if (cluster.isPresent) {
      val opt_cluster = cluster.get()
      tlvWriter.put(ContextSpecificTag(4), opt_cluster)
    }
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : BindingClusterTargetStruct {
      tlvReader.enterStructure(tag)
      val node: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val group: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val endpoint: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val cluster: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return BindingClusterTargetStruct(node, group, endpoint, cluster, fabricIndex)
    }
  }
}
