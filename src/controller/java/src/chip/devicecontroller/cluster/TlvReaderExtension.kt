/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.devicecontroller.cluster

import chip.tlv.NullValue
import chip.tlv.Tag
import chip.tlv.TlvReader

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
