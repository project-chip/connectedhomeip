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

import chip.jsontlv.fromJsonString
import chip.tlv.AnonymousTag
import chip.tlv.Tag
import chip.tlv.TlvWriter

fun TlvWriter.fromObject(value: Any?, tag: Tag = AnonymousTag): TlvWriter {
  when (value) {
    null -> {
      putNull(tag)
    }
    is Int -> {
      put(tag, value)
    }
    is Long -> {
      put(tag, value)
    }
    is Short -> {
      put(tag, value)
    }
    is Byte -> {
      put(tag, value)
    }
    is UInt -> {
      put(tag, value)
    }
    is ULong -> {
      put(tag, value)
    }
    is UShort -> {
      put(tag, value)
    }
    is UByte -> {
      put(tag, value)
    }
    is Boolean -> {
      put(tag, value)
    }
    is Float -> {
      put(tag, value)
    }
    is Double -> {
      put(tag, value)
    }
    is ByteArray -> {
      put(tag, value)
    }
    is List<*> -> {
      startArray(tag)
      value.forEach { fromObject(it) }
      endArray()
    }
    is String -> {
      try {
        fromJsonString(value)
      } catch (e: IllegalArgumentException) {
        // If json parsing exception, judged by string value.
        put(tag, value)
      } catch (e: com.google.gson.JsonSyntaxException) {
        // If json parsing exception, judged by string value.
        put(tag, value)
      }
    }
  }
  return this
}
