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

package com.google.chip.chiptool.commissioner.thread.internal;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.ThreadCommissionerException;
import com.google.chip.chiptool.commissioner.thread.ThreadCommissionerService;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkCredential;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

public class ThreadCommissionerServiceImpl implements ThreadCommissionerService {

  private static final String TAG = ThreadCommissionerException.class.getSimpleName();

  private BorderAgentDatabase borderAgentDatabase;

  public ThreadCommissionerServiceImpl(@NonNull Context context) {
    this.borderAgentDatabase = BorderAgentDatabase.getDatabase(context);
  }

  @Override
  public CompletableFuture<ThreadNetworkCredential> FetchThreadNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo, @Nullable byte[] pskc) {
    return getThreadNetworkCredential(borderAgentInfo)
        .thenApply(
            credential -> {
              if (credential != null) {
                return credential;
              }

              NetworkCredentialFetcher credentialFetcher = new NetworkCredentialFetcher();
              try {
                credential = credentialFetcher.fetchNetworkCredential(borderAgentInfo, pskc);
                addThreadNetworkCredential(borderAgentInfo, pskc, credential).get();
                return credential;
              } catch (Exception ex) {
                throw new CompletionException(ex);
              }
            });
  }

  @Override
  public CompletableFuture<ThreadNetworkCredential> getThreadNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo) {
    return getBorderAgentRecord(borderAgentInfo)
        .thenApply(
            borderAgentRecord -> {
              if (borderAgentRecord == null) {
                return null;
              }
              return new ThreadNetworkCredential(borderAgentRecord.getActiveOperationalDataset());
            });
  }

  CompletableFuture<BorderAgentRecord> getBorderAgentRecord(
      @NonNull BorderAgentInfo borderAgentInfo) {
    return borderAgentDatabase.getBorderAgent(borderAgentInfo.discriminator);
  }

  @Override
  public CompletableFuture<Void> deleteThreadNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo) {
    return borderAgentDatabase.deleteBorderAgent(borderAgentInfo.discriminator);
  }

  // This method adds given Thread Network Credential into database on the phone.
  CompletableFuture<Void> addThreadNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo,
      @NonNull byte[] pskc,
      @NonNull ThreadNetworkCredential networkCredential) {
    BorderAgentRecord borderAgentRecord =
        new BorderAgentRecord(
            borderAgentInfo.discriminator,
            borderAgentInfo.networkName,
            borderAgentInfo.extendedPanId,
            pskc,
            networkCredential.getEncoded());
    return borderAgentDatabase.insertBorderAgent(borderAgentRecord);
  }
}
