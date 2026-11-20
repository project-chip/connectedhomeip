/*
 *   Copyright (c) 2026 Project CHIP Authors
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
package com.google.chip.chiptool.provisioning

import android.content.Intent
import androidx.fragment.app.Fragment

/** No-op stub used when `matterEnableGms=false` so `play-services-threadnetwork` is not linked. */
object GmsThreadDatasetHelper {
  const val IS_AVAILABLE = false

  @Suppress("UNUSED_PARAMETER")
  fun fetchFromGms(fragment: Fragment, onFallback: () -> Unit) {
    onFallback()
  }

  @Suppress("UNUSED_PARAMETER")
  fun parseConsentResult(
    requestCode: Int,
    resultCode: Int,
    data: Intent?,
  ): List<ThreadDatasetCandidate>? = null
}
