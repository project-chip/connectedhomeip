/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

  public native boolean startAppWithDelegate(ChipAppServerDelegate appDelegate);

  public native boolean stopApp();

  public native boolean resetApp();
}
