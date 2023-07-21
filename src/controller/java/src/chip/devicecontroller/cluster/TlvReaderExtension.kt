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

package chip.devicecontroller.cluster

import chip.tlv.ArrayValue
import chip.tlv.BooleanValue
import chip.tlv.ByteStringValue
import chip.tlv.DoubleValue
import chip.tlv.FloatValue
import chip.tlv.IntValue
import chip.tlv.ListValue
import chip.tlv.NullValue
import chip.tlv.StructureValue
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.UnsignedIntValue
import chip.tlv.Utf8StringValue

fun TlvReader.getBoolean(tag: Tag): Boolean {
  return getBool(tag)
}

fun TlvReader.getString(tag: Tag): String {
  return getUtf8String(tag)
}

fun TlvReader.getByteArray(tag: Tag): ByteArray {
  return getByteString(tag)
}

fun TlvReader.isNull(): Boolean {
  val value = peekElement().value
  return (value is NullValue)
}

fun TlvReader.isNextTag(tag: Tag): Boolean {
  val nextTag = peekElement().tag
  return (nextTag == tag)
}
