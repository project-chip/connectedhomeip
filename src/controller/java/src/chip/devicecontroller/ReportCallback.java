/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.ChipEventPath;
import chip.devicecontroller.model.NodeState;
import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/** An interface for receiving read/subscribe CHIP reports. */
public interface ReportCallback {
  void onError(
      @Nullable ChipAttributePath attributePath,
      @Nullable ChipEventPath eventPath,
      @Nonnull Exception e);

  void onReport(NodeState nodeState);

  default void onDone() {}
}
