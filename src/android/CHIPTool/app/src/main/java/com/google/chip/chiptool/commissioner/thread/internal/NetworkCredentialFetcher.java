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

import android.util.Log;
import androidx.annotation.NonNull;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.CommissionerUtils;
import com.google.chip.chiptool.commissioner.thread.ThreadCommissionerException;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkCredential;
import io.openthread.commissioner.ByteArray;
import io.openthread.commissioner.ChannelMask;
import io.openthread.commissioner.Commissioner;
import io.openthread.commissioner.CommissionerHandler;
import io.openthread.commissioner.Config;
import io.openthread.commissioner.Error;
import io.openthread.commissioner.ErrorCode;
import io.openthread.commissioner.LogLevel;
import io.openthread.commissioner.Logger;
import java.net.InetAddress;

class NetworkCredentialFetcher {

  private static final String TAG = NetworkCredentialFetcher.class.getSimpleName();

  NativeCommissionerHandler nativeCommissionerHandler = new NativeCommissionerHandler();
  Commissioner nativeCommissioner;

  public ThreadNetworkCredential fetchNetworkCredential(
      @NonNull BorderAgentInfo borderAgentInfo, @NonNull byte[] pskc)
      throws ThreadCommissionerException {
    byte[] rawActiveDataset =
        fetchNetworkCredential(borderAgentInfo.host, borderAgentInfo.port, pskc);
    return new ThreadNetworkCredential(rawActiveDataset);
  }

  public void cancel() {
    if (nativeCommissioner != null) {
      Log.d(TAG, "cancel requesting credential");
      nativeCommissioner.cancelRequests();
    }
  }

  private byte[] fetchNetworkCredential(
      @NonNull InetAddress address, int port, @NonNull byte[] pskc)
      throws ThreadCommissionerException {
    nativeCommissioner = Commissioner.create(nativeCommissionerHandler);

    Config config = new Config();
    config.setId("TestComm");
    config.setDomainName("TestDomain");
    config.setEnableCcm(false);
    config.setEnableDtlsDebugLogging(true);
    config.setPSKc(CommissionerUtils.getByteArray(pskc));
    config.setLogger(new NativeCommissionerLogger());

    try {
      // Initialize the native commissioner
      throwIfFail(nativeCommissioner.init(config));

      // Petition to be the active commissioner in the Thread Network.
      String[] existingCommissionerId = new String[1];
      throwIfFail(
          nativeCommissioner.petition(existingCommissionerId, address.getHostAddress(), port));

      // Fetch Active Operational Dataset.
      ByteArray rawActiveDataset = new ByteArray();
      throwIfFail(nativeCommissioner.getRawActiveDataset(rawActiveDataset, 0xFFFF));
      nativeCommissioner.resign();
      nativeCommissioner = null;
      return CommissionerUtils.getByteArray(rawActiveDataset);
    } catch (ThreadCommissionerException e) {
      nativeCommissioner.resign();
      nativeCommissioner = null;
      throw e;
    }
  }

  private void throwIfFail(Error error) throws ThreadCommissionerException {
    if (error.getCode() != ErrorCode.kNone) {
      throw new ThreadCommissionerException(error.getCode().swigValue(), error.getMessage());
    }
  }
}

class NativeCommissionerLogger extends Logger {
  private static final String TAG = "NativeCommissioner";

  @Override
  public void log(LogLevel level, String region, String msg) {
    Log.d(TAG, String.format("[ %s ]: %s", region, msg));
  }
}

class NativeCommissionerHandler extends CommissionerHandler {
  private static final String TAG = NativeCommissionerHandler.class.getSimpleName();

  @Override
  public String onJoinerRequest(ByteArray joinerId) {
    Log.d(TAG, "A joiner is requesting commissioning");
    return "";
  }

  @Override
  public void onJoinerConnected(ByteArray joinerId, Error error) {
    Log.d(TAG, "A joiner is connected");
  }

  @Override
  public boolean onJoinerFinalize(
      ByteArray joinerId,
      String vendorName,
      String vendorModel,
      String vendorSwVersion,
      ByteArray vendorStackVersion,
      String provisioningUrl,
      ByteArray vendorData) {
    Log.d(TAG, "A joiner is finalizing");
    return true;
  }

  @Override
  public void onKeepAliveResponse(Error error) {
    Log.d(TAG, "received keep-alive response: " + error.toString());
  }

  @Override
  public void onPanIdConflict(String peerAddr, ChannelMask channelMask, int panId) {
    Log.d(TAG, "received PAN ID CONFLICT report");
  }

  @Override
  public void onEnergyReport(String aPeerAddr, ChannelMask aChannelMask, ByteArray aEnergyList) {
    Log.d(TAG, "received ENERGY SCAN report");
  }

  @Override
  public void onDatasetChanged() {
    Log.d(TAG, "Thread Network Dataset chanaged");
  }
}
