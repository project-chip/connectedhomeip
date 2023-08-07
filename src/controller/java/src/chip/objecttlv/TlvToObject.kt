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

package chip.objecttlv

import chip.jsontlv.toJsonString
import chip.tlv.AnonymousTag
import chip.tlv.ArrayValue
import chip.tlv.BooleanValue
import chip.tlv.ByteStringValue
import chip.tlv.DoubleValue
import chip.tlv.FloatValue
import chip.tlv.IntValue
import chip.tlv.NullValue
import chip.tlv.StructureValue
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.UnsignedIntValue
import chip.tlv.Utf8StringValue

/** Converts TLV to kotlin value. */
fun TlvReader.toObject(tag: Tag = AnonymousTag): Any? {
  val element = peekElement()
  return when (element.value) {
    StructureValue -> {
      toJsonString()
    }
    ArrayValue -> {
      buildList {
        enterArray(tag)
        while (!isEndOfContainer()) {
          add(toObject())
        }
        exitContainer()
      }
    }
    NullValue -> {
      getNull(tag)
      null
    }
    is BooleanValue -> {
      getBool(tag)
    }
    is ByteStringValue -> {
      getByteString(tag)
    }
    is DoubleValue -> {
      getDouble(tag)
    }
    is FloatValue -> {
      getFloat(tag)
    }
    is IntValue -> {
      getLong(tag)
    }
    is UnsignedIntValue -> {
      getULong(tag)
    }
    is Utf8StringValue -> {
      getUtf8String(tag)
    }
    else -> {
      null
    }
  }
}
