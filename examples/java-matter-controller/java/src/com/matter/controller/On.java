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

import com.matter.controller.commands.common.CHIPCommand;
import com.matter.controller.commands.common.CredentialIssuerCommands;
import com.matter.controller.commands.common.MutableInteger;

public class On extends CHIPCommand {
  private MutableInteger mNodeId;
  private MutableInteger mFabricId;

  public On(CredentialIssuerCommands credIssuerCmds) {
    super("on", credIssuerCmds);
    addArgument("nodeid", 0, Long.MAX_VALUE, mNodeId);
    addArgument("fabricid", 0, Long.MAX_VALUE, mFabricId);
  }

  @Override
  protected final int runCommand() {
    return 0;
  }
}
