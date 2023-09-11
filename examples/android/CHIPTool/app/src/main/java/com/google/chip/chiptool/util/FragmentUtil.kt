/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

package com.google.chip.chiptool.util

import androidx.fragment.app.Fragment
import java.util.Locale

object FragmentUtil {
  /**
   * Resolves a host of this Fragment as an instance of the specified class. The first of either the
   * parent Fragment or the host Activity which implements this interface will be returned.
   *
   * @param source the Fragment whose host should be retrieved
   * @param hostClass the interface which the host must implement
   * @throws IllegalStateException if neither host implements <var>hostClass</var>
   */
  @JvmStatic
  fun <T> getHost(source: Fragment, hostClass: Class<T>): T? {
    val activity = source.activity
    val parentFragment = source.parentFragment
    return when {
      hostClass.isInstance(parentFragment) -> hostClass.cast(parentFragment)
      hostClass.isInstance(activity) -> hostClass.cast(activity)
      else -> {
        val activityName = activity?.let { it::class.java.simpleName }
        val parentName = parentFragment?.let { it::class.java.simpleName }
        val exceptionString =
          String.format(
            Locale.ROOT,
            "Neither the parent Fragment $parentName nor the host Activity" +
              " $activityName of ${source::class.java.simpleName} implement" +
              " ${hostClass.simpleName}."
          )
        throw IllegalStateException(exceptionString)
      }
    }
  }
}
