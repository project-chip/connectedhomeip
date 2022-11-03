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

package com.matter.controller;

import com.matter.controller.commands.common.CredentialsIssuer;
import com.matter.controller.commands.common.MatterCommand;
import java.util.concurrent.atomic.AtomicLong;

public final class Off extends MatterCommand {
  private final AtomicLong mNodeId = new AtomicLong();
  private final AtomicLong mFabricId = new AtomicLong();

  public Off(CredentialsIssuer credIssuerCmds) {
    super("off", credIssuerCmds);
    addArgument("nodeid", 0L, Long.MAX_VALUE, mNodeId, null);
    addArgument("fabricid", 0L, Long.MAX_VALUE, mFabricId, null);
  }

  @Override
  protected final void runCommand() {}
}
