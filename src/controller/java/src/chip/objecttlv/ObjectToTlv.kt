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

import chip.jsontlv.fromJson
import chip.tlv.AnonymousTag
import chip.tlv.Tag
import chip.tlv.TlvEncodingException
import chip.tlv.TlvWriter

fun TlvWriter.fromObject(value: Any?, tag: Tag = AnonymousTag): TlvWriter {
  when (value) {
    null -> {
      putNull(tag)
    }
    is Int, is Long, is Short, is Byte  -> {
      fromSignedObject(value, tag)
    }
    is UInt, is ULong, is UShort, is UByte -> {
      fromUnsignedObject(value, tag)
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
        fromJson(value)
      } catch (e: IllegalArgumentException) {
        // If json parsing exception, judged by string value.
        put(tag, value)
      }
  }
  return this
}

private fun TlvWriter.fromUnsignedObject(value: Any?, tag: Tag): TlvWriter {
  val ret = when(value) {
    is UInt -> {
      value.toULong()
    }
    is UShort -> {
      value.toULong()
    }
    is UByte -> {
      value.toULong()
    }
    is ULong -> {
      value
    }
    else -> {
      return this
    }
  }
  put(tag, ret)
  return this
}

private fun TlvWriter.fromSignedObject(value: Any?, tag: Tag): TlvWriter {
  val ret = when(value) {
    is Int -> {
      value.toLong()
    }
    is Short -> {
      value.toLong()
    }
    is Byte -> {
      value.toLong()
    }
    is Long -> {
      value
    }
    else -> {
      return this
    }
  }
  put(tag, ret)
  return this
}
