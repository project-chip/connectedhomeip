/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019-2023 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.tlv

/**
 * Represents a single TLV element.
 *
 * @property tag the tag of the element and its associated data
 * @property value the value of the element
 */
data class Element(val tag: Tag, val value: Value)
