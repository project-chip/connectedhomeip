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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class PowerTopologyClusterCircuitNodeStruct(
  val node: ULong,
  val endpoint: Optional<UShort>,
  val label: Optional<String>,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("PowerTopologyClusterCircuitNodeStruct {\n")
    append("\tnode : $node\n")
    append("\tendpoint : $endpoint\n")
    append("\tlabel : $label\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NODE), node)
      if (endpoint.isPresent) {
        val optendpoint = endpoint.get()
        put(ContextSpecificTag(TAG_ENDPOINT), optendpoint)
      }
      if (label.isPresent) {
        val optlabel = label.get()
        put(ContextSpecificTag(TAG_LABEL), optlabel)
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE = 1
    private const val TAG_ENDPOINT = 2
    private const val TAG_LABEL = 3
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): PowerTopologyClusterCircuitNodeStruct {
      tlvReader.enterStructure(tlvTag)
      val node = tlvReader.getULong(ContextSpecificTag(TAG_NODE))
      val endpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_ENDPOINT)))
        } else {
          Optional.empty()
        }
      val label =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LABEL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_LABEL)))
        } else {
          Optional.empty()
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return PowerTopologyClusterCircuitNodeStruct(node, endpoint, label, fabricIndex)
    }
  }
}
