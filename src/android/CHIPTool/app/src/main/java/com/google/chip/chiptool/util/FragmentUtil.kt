/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package com.google.chip.chiptool.util

import android.widget.Toast
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
  @JvmStatic fun <T> getHost(source: Fragment, hostClass: Class<T>): T? {
    val activity = source.activity
    val parentFragment = source.parentFragment
    return when {
        hostClass.isInstance(parentFragment) -> hostClass.cast(parentFragment)
        hostClass.isInstance(activity) -> hostClass.cast(activity)
        else -> {
            val activityName = activity?.let { it::class.java.simpleName }
            val parentName = parentFragment?.let { it::class.java.simpleName }
            val exceptionString = String.format(
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
