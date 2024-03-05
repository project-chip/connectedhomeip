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
package matter.controller.cluster.eventstructs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AccountLoginClusterLoggedOutEvent(val node: Optional<ULong>) {
  override fun toString(): String = buildString {
    append("AccountLoginClusterLoggedOutEvent {\n")
    append("\tnode : $node\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (node.isPresent) {
        val optnode = node.get()
        put(ContextSpecificTag(TAG_NODE), optnode)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE = 0

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AccountLoginClusterLoggedOutEvent {
      tlvReader.enterStructure(tlvTag)
      val node =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NODE))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_NODE)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return AccountLoginClusterLoggedOutEvent(node)
    }
  }
}
