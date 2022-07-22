/*
 *   Copyright (c) 2021 Project CHIP Authors
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
package chip.appserver;

/** Controller to interact with the CHIP device. */
public class ChipAppServer {
  private static final String TAG = ChipAppServer.class.getSimpleName();

  private volatile ChipFabricProvider mChipFabricProvider;

  public ChipFabricProvider getFabricProvider() {

    if (mChipFabricProvider == null) {
      synchronized (this) {
        if (mChipFabricProvider == null) mChipFabricProvider = new ChipFabricProvider();
      }
    }

    return mChipFabricProvider;
  }

  public native boolean startApp();

  public native boolean stopApp();
}
