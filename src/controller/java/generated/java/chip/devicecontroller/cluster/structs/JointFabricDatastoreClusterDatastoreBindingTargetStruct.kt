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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class JointFabricDatastoreClusterDatastoreBindingTargetStruct(
  val node: Optional<ULong>,
  val group: Optional<UInt>,
  val endpoint: Optional<UInt>,
  val cluster: Optional<ULong>,
) {
  override fun toString(): String = buildString {
    append("JointFabricDatastoreClusterDatastoreBindingTargetStruct {\n")
    append("\tnode : $node\n")
    append("\tgroup : $group\n")
    append("\tendpoint : $endpoint\n")
    append("\tcluster : $cluster\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (node.isPresent) {
        val optnode = node.get()
        put(ContextSpecificTag(TAG_NODE), optnode)
      }
      if (group.isPresent) {
        val optgroup = group.get()
        put(ContextSpecificTag(TAG_GROUP), optgroup)
      }
      if (endpoint.isPresent) {
        val optendpoint = endpoint.get()
        put(ContextSpecificTag(TAG_ENDPOINT), optendpoint)
      }
      if (cluster.isPresent) {
        val optcluster = cluster.get()
        put(ContextSpecificTag(TAG_CLUSTER), optcluster)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE = 1
    private const val TAG_GROUP = 2
    private const val TAG_ENDPOINT = 3
    private const val TAG_CLUSTER = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): JointFabricDatastoreClusterDatastoreBindingTargetStruct {
      tlvReader.enterStructure(tlvTag)
      val node =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NODE))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_NODE)))
        } else {
          Optional.empty()
        }
      val group =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_GROUP))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_GROUP)))
        } else {
          Optional.empty()
        }
      val endpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT)))
        } else {
          Optional.empty()
        }
      val cluster =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CLUSTER))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_CLUSTER)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return JointFabricDatastoreClusterDatastoreBindingTargetStruct(node, group, endpoint, cluster)
    }
  }
}
