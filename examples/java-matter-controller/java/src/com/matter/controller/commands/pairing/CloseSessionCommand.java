/*
 *   Copyright (c) 2022 Project CHIP Authors
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

package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;
import com.matter.controller.commands.common.MatterCommand;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

public final class CloseSessionCommand extends MatterCommand {
  private final AtomicLong mDestinationId = new AtomicLong();
  private final AtomicInteger mTimeoutSecs = new AtomicInteger();

  public CloseSessionCommand(ChipDeviceController controller, CredentialsIssuer credsIssuer) {
    super(controller, "close-session", credsIssuer);
    addArgument("destination-id", 0, Long.MAX_VALUE, mDestinationId, null);
    addArgument(
        "timeout",
        (short) 0,
        Short.MAX_VALUE,
        mTimeoutSecs,
        "Time, in seconds, before this command is considered to have timed out.");
  }

  @Override
  protected final void runCommand() {}
}
