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

package com.google.chip.chiptool.commissioner.thread;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.util.concurrent.CompletableFuture;

public interface ThreadCommissionerService {

  // This method returns Credential of a given Thread Network.
  // If the Commissioner Service doesn't possess the Network
  // Credentials already, it will fetch the Credentials from
  // a Border Agent (assisting device) using the Thread
  // Commissioning protocol.
  //
  // The pskc is used to securely connect to the Border Agent device.
  // If no pskc is given, the user will be asked to input it.
  // The Network Credentials will be saved in the Commissioner
  // Service before returning.
  CompletableFuture<ThreadNetworkCredential> FetchThreadNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo, @Nullable byte[] pskc);

  // This method returns Credential of a given Thread Network stored in the database on the phone.
  CompletableFuture<ThreadNetworkCredential> getThreadNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo);

  // This method deletes Credential of a given Thread Network stored in the database on the phone.
  CompletableFuture<Void> deleteThreadNetworkCredential(@NonNull BorderAgentInfo borderAgentInfo);
}
