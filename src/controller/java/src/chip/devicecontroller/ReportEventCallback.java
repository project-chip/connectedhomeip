/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller;

import chip.devicecontroller.model.ChipEventPath;
import chip.devicecontroller.model.NodeState;

/** An interface for receiving read/subscribe CHIP reports. */
public interface ReportEventCallback {
  void onError(ChipEventPath eventPath, Exception e);

  void onReport(NodeState nodeState);

  default void onDone() {}
}
