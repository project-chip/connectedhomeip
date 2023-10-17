/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2019-2023 Google LLC.
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

package matter.tlv

import java.lang.Double.doubleToLongBits
import java.lang.Float.floatToIntBits

/** Represents the value of a TLV element. */
sealed class Value {
  internal abstract fun toType(): Type

  internal abstract fun encode(): ByteArray

  open fun toAny(): Any? = null
}

/** Represents a signed integer value of a TLV element. */
data class IntValue(val value: Long) : Value() {
  override fun toType() = SignedIntType(signedIntSize(value))

  override fun encode() = value.toByteArrayLittleEndian(toType().valueSize)

  override fun toAny() = value
}

/** Represents an unsigned integer value of a TLV element. */
data class UnsignedIntValue(val value: Long) : Value() {
  override fun toType() = UnsignedIntType(unsignedIntSize(value.toULong()))

  override fun encode() = value.toByteArrayLittleEndian(toType().valueSize)

  override fun toAny() = value
}

/** Represents a boolean value of a TLV element. */
data class BooleanValue(val value: Boolean) : Value() {
  override fun toType() = BooleanType(value)

  override fun encode() = ByteArray(0)

  override fun toAny() = value
}

/** Represents a floating-point Float value of a TLV element. */
data class FloatValue(val value: Float) : Value() {
  override fun toType() = FloatType()

  override fun encode() = floatToIntBits(value).toByteArrayLittleEndian(4)

  override fun toAny() = value
}

/** Represents a floating-point DoubleFloat value of a TLV element. */
data class DoubleValue(val value: Double) : Value() {
  override fun toType() = DoubleType()

  override fun encode() = doubleToLongBits(value).toByteArrayLittleEndian(8)

  override fun toAny() = value
}

/** Represents a UTF8 string value of a TLV element. */
data class Utf8StringValue(val value: String) : Value() {
  override fun toType() = Utf8StringType(unsignedIntSize(value.toByteArray().size.toULong()))

  override fun encode() =
    value.toByteArray().size.toByteArrayLittleEndian(toType().lengthSize) + value.toByteArray()

  override fun toAny() = value
}

/** Represents an octet string value of a TLV element. */
data class ByteStringValue(val value: ByteArray) : Value() {
  override fun toType() = ByteStringType(unsignedIntSize(value.size.toULong()))

  override fun encode() = value.size.toByteArrayLittleEndian(toType().lengthSize) + value

  override fun toAny() = value
}

/** Represents a null value in a TLV element. */
object NullValue : Value() {
  override fun toType() = NullType()

  override fun encode() = ByteArray(0)
}

/** Represents an empty value for a structure container element. */
object StructureValue : Value() {
  override fun toType() = StructureType()

  override fun encode() = ByteArray(0)
}

/** Represents an array value of a TLV element. */
object ArrayValue : Value() {
  override fun toType() = ArrayType()

  override fun encode() = ByteArray(0)
}

/** Represents a list value of a TLV element. */
object ListValue : Value() {
  override fun toType() = ListType()

  override fun encode() = ByteArray(0)
}

/** Represents an empty value for an end-of-container element. */
object EndOfContainerValue : Value() {
  override fun toType() = EndOfContainerType

  override fun encode() = ByteArray(0)
}
