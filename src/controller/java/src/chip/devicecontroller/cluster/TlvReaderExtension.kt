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

fun TlvReader.isLong(min: Long = Long.MIN_VALUE, max: Long = Long.MAX_VALUE): Boolean {
  val value = peekElement().value
  if (value !is IntValue) { return false }
  return isRange(value.value, min..max)
}

fun TlvReader.isULong(min: ULong = ULong.MIN_VALUE, max: ULong = ULong.MAX_VALUE): Boolean {
  val value = peekElement().value
  if (value !is UnsignedIntValue) { return false }
  return isRange(value.value.toULong(), min..max)
}

fun TlvReader.isInt(): Boolean {
  return isLong(Int.MIN_VALUE.toLong(), Int.MAX_VALUE.toLong())
}

fun TlvReader.isUInt(): Boolean {
  return isULong(UInt.MIN_VALUE.toULong(), UInt.MAX_VALUE.toULong())
}

fun TlvReader.isShort(): Boolean {
  return isLong(Short.MIN_VALUE.toLong(),Short.MAX_VALUE.toLong())
}

fun TlvReader.isUShort(): Boolean {
  return isULong(UShort.MIN_VALUE.toULong(), UShort.MAX_VALUE.toULong())
}

fun TlvReader.isByte(): Boolean {
  return isLong(Byte.MIN_VALUE.toLong(), Byte.MAX_VALUE.toLong())
}

fun TlvReader.isUByte(): Boolean {
  return isULong(UByte.MIN_VALUE.toULong(), UByte.MAX_VALUE.toULong())
}

fun TlvReader.isBoolean(): Boolean {
  val value = peekElement().value
  return (value is BooleanValue)
}

fun TlvReader.isFloat(): Boolean {
  val value = peekElement().value
  return (value is FloatValue)
}

fun TlvReader.isDouble(): Boolean {
  val value = peekElement().value
  return (value is DoubleValue)
}

fun TlvReader.isString(): Boolean {
  val value = peekElement().value
  return (value is Utf8StringValue)
}

fun TlvReader.isByteArray(): Boolean {
  val value = peekElement().value
  return (value is ByteStringValue)
}

fun TlvReader.isNull(): Boolean {
  val value = peekElement().value
  return (value is NullValue)
}

fun TlvReader.isStructure(): Boolean {
  val value = peekElement().value
  return (value is StructureValue)
}

fun TlvReader.isArray(): Boolean {
  val value = peekElement().value
  return (value is ArrayValue)
}

fun TlvReader.isList(): Boolean {
  val value = peekElement().value
  return (value is ListValue)
}

fun TlvReader.isNextTag(tag: Tag): Boolean {
  val nextTag = peekElement().tag
  return (nextTag == tag)
}

private fun <T : Comparable<T>> isRange(
  value: T,
  range: ClosedRange<T>
): Boolean {
  return (value in range)
}
